import type { RenderProblem } from '~/types/api'
import type { H3Event } from 'h3'

// Keep the active Nuxt ingress bounded at the documented provisional P5/P6
// HTTP limit. The Rust service remains authoritative for all request policy.
const MAX_PROXY_REQUEST_BYTES = 1_048_576

class PayloadTooLargeError extends Error {}

const problemCodes = new Set([
  'invalid_request',
  'authentication_failed',
  'resource_limit',
  'unsupported_media_type',
  'invalid_image',
  'overloaded',
  'unavailable',
  'deadline_exceeded',
  'internal',
])

function safeProblem(value: unknown): RenderProblem | null {
  if (!value || typeof value !== 'object') return null
  const candidate = value as Partial<RenderProblem>
  if (
    typeof candidate.type !== 'string'
    || typeof candidate.title !== 'string'
    || typeof candidate.status !== 'number'
    || typeof candidate.code !== 'string'
    || !problemCodes.has(candidate.code)
    || typeof candidate.request_id !== 'string'
  ) return null
  return candidate as RenderProblem
}

function proxyProblem(
  event: H3Event,
  status: number,
  code: RenderProblem['code'],
  title: string,
): RenderProblem {
  setResponseStatus(event, status)
  setResponseHeader(event, 'content-type', 'application/problem+json')
  return {
    type: `https://imgengine.example/problems/${code}`,
    title,
    status,
    code,
    request_id: `proxy-${crypto.randomUUID()}`,
  }
}

/**
 * H3's `readFormData` otherwise consumes the Node request directly without a
 * byte ceiling. Install a bounded stream before parsing so a browser upload
 * cannot be buffered beyond the same provisional limit used by Rust.
 */
function installBoundedRequestBody(event: Parameters<typeof readFormData>[0]) {
  const contentLength = getHeader(event, 'content-length')
  if (contentLength && /^\d+$/.test(contentLength) && Number(contentLength) > MAX_PROXY_REQUEST_BYTES) {
    throw new PayloadTooLargeError()
  }

  const request = event.node.req
  const stream = new ReadableStream<Uint8Array>({
    async start(controller) {
      try {
        let received = 0
        for await (const chunk of request) {
          const bytes = chunk instanceof Uint8Array ? chunk : new Uint8Array(chunk)
          received += bytes.byteLength
          if (received > MAX_PROXY_REQUEST_BYTES) {
            throw new PayloadTooLargeError()
          }
          controller.enqueue(bytes)
        }
        controller.close()
      } catch (cause) {
        controller.error(cause)
      }
    },
  })

  // `_requestBody` is the H3 event seam consumed by `toWebRequest`, which is
  // what `readFormData` uses. Do not read the original request a second time.
  ;(event as typeof event & { _requestBody?: ReadableStream<Uint8Array> })._requestBody = stream
}

export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  if (!config.imgengineRenderApiKey) {
    return proxyProblem(event, 503, 'unavailable', 'Rendering is unavailable')
  }

  let incoming: FormData
  try {
    installBoundedRequestBody(event)
    incoming = await readFormData(event)
  } catch (cause) {
    if (cause instanceof PayloadTooLargeError) {
      return proxyProblem(event, 413, 'resource_limit', 'Request exceeds the configured limit')
    }
    // H3 rejects an absent or malformed multipart body before entries can be
    // inspected. Keep that public proxy failure within the Rust contract's
    // safe invalid-request class rather than exposing an internal error.
    return proxyProblem(event, 400, 'invalid_request', 'Invalid multipart request')
  }
  const entries = [...incoming.entries()]
  if (entries.length !== 1 || entries[0][0] !== 'file' || typeof entries[0][1] === 'string') {
    return proxyProblem(event, 400, 'invalid_request', 'Expected exactly one image file')
  }

  const [_, file] = entries[0]
  const body = new FormData()
  body.append('file', file)

  let response: Response
  try {
    response = await fetch(new URL('/api/v1/render', config.imgengineApiBase), {
      method: 'POST',
      headers: { 'X-API-Key': config.imgengineRenderApiKey },
      body,
    })
  } catch {
    return proxyProblem(event, 503, 'unavailable', 'Rendering is unavailable')
  }

  const requestId = response.headers.get('x-request-id')
  if (response.ok) {
    setResponseStatus(event, response.status)
    setResponseHeader(event, 'content-type', 'image/jpeg')
    if (requestId) setResponseHeader(event, 'x-request-id', requestId)
    return new Uint8Array(await response.arrayBuffer())
  }

  const problem = safeProblem(await response.json().catch(() => null))
  if (problem) {
    setResponseStatus(event, response.status)
    setResponseHeader(event, 'content-type', 'application/problem+json')
    return problem
  }

  return proxyProblem(event, 502, 'internal', 'Rendering is unavailable')
})
