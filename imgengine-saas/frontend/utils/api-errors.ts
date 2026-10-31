import type { RenderApiError, RenderErrorKind, RenderProblem, RenderProblemCode } from '~/types/api'

const problemKinds: Readonly<Record<RenderProblemCode, RenderErrorKind>> = {
  invalid_request: 'invalid_request',
  does_not_fit: 'does_not_fit',
  authentication_failed: 'authentication_failed',
  resource_limit: 'request_too_large',
  unsupported_media_type: 'unsupported_media_type',
  invalid_image: 'invalid_image',
  overloaded: 'overloaded',
  unavailable: 'unavailable',
  deadline_exceeded: 'deadline_exceeded',
  internal: 'internal',
}

const messages: Readonly<Record<RenderErrorKind, string>> = {
  invalid_request: 'The image request is invalid. Choose one JPEG or PNG file and try again.',
  does_not_fit: 'The requested number of copies does not fit on this sheet. Reduce the copies or adjust the sheet settings.',
  authentication_failed: 'The rendering service could not authenticate this request.',
  request_too_large: 'The selected image exceeds the current request size limit.',
  unsupported_media_type: 'Only JPEG and PNG images are supported.',
  invalid_image: 'The selected file is not a valid supported image.',
  overloaded: 'The rendering service is busy. Please try again shortly.',
  unavailable: 'The rendering service is temporarily unavailable.',
  deadline_exceeded: 'Rendering took too long. Please try again.',
  internal: 'The rendering service could not complete this request.',
  network: 'The connection to the rendering service was interrupted.',
}

function isProblemCode(value: unknown): value is RenderProblemCode {
  return typeof value === 'string' && value in problemKinds
}

/**
 * Accept only the stable, documented problem fields. Backend diagnostics and
 * arbitrary proxy responses must never become browser-facing error detail.
 */
export function parseRenderProblem(value: unknown): RenderProblem | null {
  if (!value || typeof value !== 'object') return null
  const candidate = value as Partial<RenderProblem>
  if (
    typeof candidate.type !== 'string'
    || typeof candidate.title !== 'string'
    || typeof candidate.status !== 'number'
    || !isProblemCode(candidate.code)
    || typeof candidate.request_id !== 'string'
  ) return null

  return candidate as RenderProblem
}

export function classifyRenderFailure(status: number | null, value: unknown): RenderApiError {
  const problem = parseRenderProblem(value)
  if (problem && problem.status === status) {
    return {
      kind: problemKinds[problem.code],
      status,
      requestId: problem.request_id,
      message: messages[problemKinds[problem.code]],
    }
  }

  return {
    kind: status === null ? 'network' : 'internal',
    status,
    requestId: null,
    message: messages[status === null ? 'network' : 'internal'],
  }
}

/** Decode a Blob error body before classifying the documented problem JSON. */
export async function classifyRenderFailureData(status: number | null, value: unknown): Promise<RenderApiError> {
  if (!(value instanceof Blob)) return classifyRenderFailure(status, value)

  try {
    return classifyRenderFailure(status, JSON.parse(await value.text()))
  } catch {
    return classifyRenderFailure(status, null)
  }
}
