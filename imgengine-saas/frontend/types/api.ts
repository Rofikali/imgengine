export type RenderProblemCode =
  | 'invalid_request'
  | 'does_not_fit'
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

export type LayoutUnit = 'px' | 'mm'

/** Exact fields accepted by the additive FIT-only layout endpoint. */
export interface LayoutRenderInput {
  width: string
  height: string
  unit: LayoutUnit
  dpi?: string
  copies: string
  columns?: string
  gap: string
  margin: string
  outputFormat: 'jpeg'
}

export interface LayoutRenderSuccess extends RenderSuccess {
  canvasWidth: number
  canvasHeight: number
  expectedCount: number
  plannedCount: number
  executedCount: number
}

/** Complete source-relative crop at the fixed ABI 2.3 scale of 1,000,000. */
export interface SheetCrop { x: number, y: number, width: number, height: number }

/** Physical sheet dimensions are always expressed in millimetres at the HTTP boundary. */
export interface SheetRenderInput {
  /** Omission selects P12 centered fill; an explicit rectangle is all-or-none. */
  crop?: SheetCrop
  sheetWidthMm: string
  sheetHeightMm: string
  photoWidthMm: string
  photoHeightMm: string
  dpi?: string
  copies: string
  marginMm: string
  gapXmm: string
  gapYmm: string
  outputFormat: 'jpeg'
}

export type SheetRenderSuccess = LayoutRenderSuccess

export interface RenderOptions {
  signal?: AbortSignal
}

export type RenderErrorKind =
  | 'invalid_request'
  | 'does_not_fit'
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
