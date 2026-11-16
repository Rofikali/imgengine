import type { SheetCrop } from '~/types/api'

export const CROP_SCALE = 1_000_000
export const MAX_PREVIEW_PIXELS = 16_000_000
export const MAX_PREVIEW_EDGE = 8192
export interface ImageSize { width: number, height: number }
export interface CropPosition { x: number, y: number, zoom: number }
export const defaultCropPosition = (): CropPosition => ({ x: 50, y: 50, zoom: 1 })
export const clamp = (value: number, min: number, max: number) => Math.min(max, Math.max(min, value))

/** Mirrors physical-to-raster rounding only for preview; the server still owns planning. */
export function photoRaster(width: string, height: string, dpi = '300'): ImageSize | null {
  if (![width, height].every(value => /^\d+(?:\.\d{1,3})?$/.test(value)) || !/^\d+$/.test(dpi)) return null
  const resolution = Number(dpi)
  if (resolution < 1 || resolution > 1200) return null
  const pixel = (value: string) => Math.floor((Math.round(Number(value) * 1000) * resolution + 12700) / 25400)
  const size = { width: pixel(width), height: pixel(height) }
  return Object.values(size).every(value => Number.isSafeInteger(value) && value > 0 && value <= 4096) ? size : null
}

/** Integer pixel edges -> fixed point. Inward quantization reconstructs these same
 * edges under native floor(left)/ceil(right), avoiding an extra rounding pixel. */
export function editorCrop(source: ImageSize, target: ImageSize, position: CropPosition): SheetCrop | null {
  if (![source.width, source.height, target.width, target.height].every(value => Number.isSafeInteger(value) && value > 0)
    || source.width > MAX_PREVIEW_EDGE || source.height > MAX_PREVIEW_EDGE
    || target.width > 4096 || target.height > 4096
    || source.width * source.height > MAX_PREVIEW_PIXELS
    || !Object.values(position).every(Number.isFinite)) return null
  const zoom = clamp(position.zoom, 1, 4)
  const ratio = target.width / target.height
  let width: number, height: number
  if (source.width / source.height > ratio) {
    height = Math.floor(source.height / zoom)
    width = Math.floor(height * ratio)
  } else {
    width = Math.floor(source.width / zoom)
    height = Math.floor(width / ratio)
  }
  if (width < 1 || height < 1) return null
  const left = Math.floor((source.width - width) * clamp(position.x, 0, 100) / 100)
  const top = Math.floor((source.height - height) * clamp(position.y, 0, 100) / 100)
  const x = Math.ceil(left * CROP_SCALE / source.width)
  const y = Math.ceil(top * CROP_SCALE / source.height)
  return { x, y, width: Math.floor((left + width) * CROP_SCALE / source.width) - x,
    height: Math.floor((top + height) * CROP_SCALE / source.height) - y }
}

export function cropPixels(source: ImageSize, crop: SheetCrop) {
  const x = Math.floor(crop.x * source.width / CROP_SCALE)
  const y = Math.floor(crop.y * source.height / CROP_SCALE)
  return { x, y, width: Math.ceil((crop.x + crop.width) * source.width / CROP_SCALE) - x,
    height: Math.ceil((crop.y + crop.height) * source.height / CROP_SCALE) - y }
}
