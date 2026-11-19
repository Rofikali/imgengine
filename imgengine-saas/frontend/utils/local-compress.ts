export const MIN_JPEG_QUALITY = 40
export const MAX_JPEG_QUALITY = 95
export const DEFAULT_JPEG_QUALITY = 80

export interface LocalCompressRequest { blob: Blob, quality: string }
export interface LocalCompressSuccess { blob: Blob, quality: number }
export interface LocalCompressBitmap { bitmap: ImageBitmap, quality: number }
export interface LocalCompressFailure { error: true }
export type LocalCompressResponse = LocalCompressSuccess | LocalCompressBitmap | LocalCompressFailure

export function jpegQuality(value: string): number {
  if (!/^\d{1,2}$/.test(value)) throw Error('Choose a whole-number JPEG quality from 40 to 95.')
  const quality = Number(value)
  if (quality < MIN_JPEG_QUALITY || quality > MAX_JPEG_QUALITY) {
    throw Error('Choose a JPEG quality from 40 to 95.')
  }
  return quality
}

export function isLocalCompressSuccess(value: unknown): value is LocalCompressSuccess {
  return !!value && typeof value === 'object' && 'blob' in value && 'quality' in value
    && (value as { blob: unknown }).blob instanceof Blob && Number.isSafeInteger((value as { quality: unknown }).quality)
}

export function isLocalCompressBitmap(value: unknown): value is LocalCompressBitmap {
  return !!value && typeof value === 'object' && 'bitmap' in value && 'quality' in value
    && (value as { bitmap: unknown }).bitmap instanceof ImageBitmap && Number.isSafeInteger((value as { quality: unknown }).quality)
}
