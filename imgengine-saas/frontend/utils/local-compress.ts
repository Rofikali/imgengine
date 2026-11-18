export const MIN_JPEG_QUALITY = 40
export const MAX_JPEG_QUALITY = 95
export const DEFAULT_JPEG_QUALITY = 80

export function jpegQuality(value: string): number {
  if (!/^\d{1,2}$/.test(value)) throw Error('Choose a whole-number JPEG quality from 40 to 95.')
  const quality = Number(value)
  if (quality < MIN_JPEG_QUALITY || quality > MAX_JPEG_QUALITY) {
    throw Error('Choose a JPEG quality from 40 to 95.')
  }
  return quality
}
