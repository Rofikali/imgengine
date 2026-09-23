import type { RenderProblem } from '~/types/api'

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

function invalidBrowserRequest(): never {
  throw createError({
    statusCode: 400,
    statusMessage: 'Expected exactly one image file.',
    data: { code: 'invalid_request' },
  })
}

export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  if (!config.imgengineRenderApiKey) {
    throw createError({ statusCode: 503, statusMessage: 'Rendering is unavailable.' })
  }

  let incoming: FormData
  try {
    incoming = await readFormData(event)
  } catch {
    // H3 rejects an absent or malformed multipart body before entries can be
    // inspected. Keep that public proxy failure within the Rust contract's
    // safe invalid-request class rather than exposing an internal 500.
    invalidBrowserRequest()
  }
  const entries = [...incoming.entries()]
  if (entries.length !== 1 || entries[0][0] !== 'file' || typeof entries[0][1] === 'string') {
    invalidBrowserRequest()
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
    throw createError({ statusCode: 503, statusMessage: 'Rendering is unavailable.' })
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

  throw createError({ statusCode: 502, statusMessage: 'Rendering is unavailable.' })
})
