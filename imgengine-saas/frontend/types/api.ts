export type RenderProblemCode =
  | 'invalid_request'
  | 'authentication_failed'
  | 'resource_limit'
  | 'unsupported_media_type'
  | 'invalid_image'
  | 'overloaded'
  | 'unavailable'
  | 'deadline_exceeded'
  | 'internal'

export interface RenderProblem {
  type: string
  title: string
  status: number
  code: RenderProblemCode
  request_id: string
}

export interface RenderSuccess {
  jpeg: Blob
  requestId: string | null
}

export interface RenderOptions {
  signal?: AbortSignal
}

export type RenderErrorKind =
  | 'invalid_request'
  | 'authentication_failed'
  | 'request_too_large'
  | 'unsupported_media_type'
  | 'invalid_image'
  | 'overloaded'
  | 'unavailable'
  | 'deadline_exceeded'
  | 'internal'
  | 'network'

export interface RenderApiError {
  kind: RenderErrorKind
  status: number | null
  requestId: string | null
  message: string
}
