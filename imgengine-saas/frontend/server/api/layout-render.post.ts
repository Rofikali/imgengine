import type { H3Event } from 'h3'
import type { RenderProblem } from '~/types/api'

const MAX_PROXY_REQUEST_BYTES = 1_048_576
const problemCodes = new Set<RenderProblem['code']>([
  'invalid_request', 'authentication_failed', 'resource_limit', 'unsupported_media_type',
  'invalid_image', 'overloaded', 'unavailable', 'deadline_exceeded', 'internal',
])
const expectedFields = new Set([
  'file', 'width', 'height', 'unit', 'dpi', 'copies', 'columns', 'gap', 'margin', 'output_format',
])

class PayloadTooLargeError extends Error {}

function proxyProblem(event: H3Event, status: number, code: RenderProblem['code'], title: string): RenderProblem {
  setResponseStatus(event, status)
  setResponseHeader(event, 'content-type', 'application/problem+json')
  return { type: `https://imgengine.example/problems/${code}`, title, status, code, request_id: `proxy-${crypto.randomUUID()}` }
}

function safeProblem(value: unknown): RenderProblem | null {
  if (!value || typeof value !== 'object') return null
  const candidate = value as Partial<RenderProblem>
  if (typeof candidate.type !== 'string' || typeof candidate.title !== 'string' || typeof candidate.status !== 'number' || typeof candidate.code !== 'string' || !problemCodes.has(candidate.code as RenderProblem['code']) || typeof candidate.request_id !== 'string') return null
  return candidate as RenderProblem
}

function installBoundedRequestBody(event: Parameters<typeof readFormData>[0]) {
  const contentLength = getHeader(event, 'content-length')
  if (contentLength && /^\d+$/.test(contentLength) && Number(contentLength) > MAX_PROXY_REQUEST_BYTES) throw new PayloadTooLargeError()
  const request = event.node.req
  const stream = new ReadableStream<Uint8Array>({
    async start(controller) {
      try {
        let received = 0
        for await (const chunk of request) {
          const bytes = chunk instanceof Uint8Array ? chunk : new Uint8Array(chunk)
          received += bytes.byteLength
          if (received > MAX_PROXY_REQUEST_BYTES) throw new PayloadTooLargeError()
          controller.enqueue(bytes)
        }
        controller.close()
      } catch (cause) { controller.error(cause) }
    },
  })
  ;(event as typeof event & { _requestBody?: ReadableStream<Uint8Array> })._requestBody = stream
}

/** Same-origin proxy for the narrow, server-authoritative FIT-only layout contract. */
export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  if (!config.imgengineRenderApiKey) return proxyProblem(event, 503, 'unavailable', 'Rendering is unavailable')
  let incoming: FormData
  try {
    installBoundedRequestBody(event)
    incoming = await readFormData(event)
  } catch (cause) {
    return cause instanceof PayloadTooLargeError
      ? proxyProblem(event, 413, 'resource_limit', 'Request exceeds the configured limit')
      : proxyProblem(event, 400, 'invalid_request', 'Invalid multipart request')
  }
  const entries = [...incoming.entries()]
  if (entries.length < 8 || entries.length > 10 || entries.some(([name]) => !expectedFields.has(name))) {
    return proxyProblem(event, 400, 'invalid_request', 'Invalid layout request')
  }
  const fields = new Map<string, FormDataEntryValue>()
  for (const [name, value] of entries) {
    if (fields.has(name)) return proxyProblem(event, 400, 'invalid_request', 'Invalid layout request')
    fields.set(name, value)
  }
  const file = fields.get('file')
  if (!(file instanceof File)) return proxyProblem(event, 400, 'invalid_request', 'Expected one image file')
  const required = ['width', 'height', 'unit', 'copies', 'gap', 'margin', 'output_format']
  if (required.some(name => typeof fields.get(name) !== 'string')) return proxyProblem(event, 400, 'invalid_request', 'Invalid layout request')
  if (fields.get('unit') === 'mm' && typeof fields.get('dpi') !== 'string') return proxyProblem(event, 400, 'invalid_request', 'Invalid layout request')
  if (fields.get('unit') !== 'mm' && fields.has('dpi')) return proxyProblem(event, 400, 'invalid_request', 'Invalid layout request')
  const body = new FormData()
  // Preserve only exact declared fields; no client filename or unknown metadata is forwarded.
  body.append('file', file)
  for (const name of ['width', 'height', 'unit', 'dpi', 'copies', 'columns', 'gap', 'margin', 'output_format']) {
    const value = fields.get(name)
    if (typeof value === 'string') body.append(name, value)
  }
  let response: Response
  try {
    response = await fetch(new URL('/api/v1/layout-render', config.imgengineApiBase), { method: 'POST', headers: { 'X-API-Key': config.imgengineRenderApiKey }, body })
  } catch {
    return proxyProblem(event, 503, 'unavailable', 'Rendering is unavailable')
  }
  const requestId = response.headers.get('x-request-id')
  if (response.ok) {
    setResponseStatus(event, response.status)
    setResponseHeader(event, 'content-type', 'image/jpeg')
    if (requestId) setResponseHeader(event, 'x-request-id', requestId)
    for (const name of ['x-imgengine-canvas-width', 'x-imgengine-canvas-height', 'x-imgengine-expected-count', 'x-imgengine-planned-count', 'x-imgengine-executed-count']) {
      const value = response.headers.get(name)
      if (value) setResponseHeader(event, name, value)
    }
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
