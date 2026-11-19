export const MIN_JPEG_QUALITY = 40
export const MAX_JPEG_QUALITY = 95
export const DEFAULT_JPEG_QUALITY = 80
export const MAX_COMPRESSED_JPEG_BYTES = 20 * 1024 * 1024

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

export function validCompressedJpeg(value: unknown): value is Blob {
  return value instanceof Blob && value.type === 'image/jpeg'
    && value.size > 0 && value.size <= MAX_COMPRESSED_JPEG_BYTES
}

function validQuality(value: unknown): value is number {
  return Number.isSafeInteger(value) && typeof value === 'number'
    && value >= MIN_JPEG_QUALITY && value <= MAX_JPEG_QUALITY
}

export function isLocalCompressSuccess(value: unknown): value is LocalCompressSuccess {
  return !!value && typeof value === 'object' && 'blob' in value && 'quality' in value
    && validCompressedJpeg((value as { blob: unknown }).blob)
    && validQuality((value as { quality: unknown }).quality)
}

export function isLocalCompressBitmap(value: unknown): value is LocalCompressBitmap {
  return !!value && typeof value === 'object' && 'bitmap' in value && 'quality' in value
    && typeof ImageBitmap !== 'undefined' && (value as { bitmap: unknown }).bitmap instanceof ImageBitmap
    && validQuality((value as { quality: unknown }).quality)
}
