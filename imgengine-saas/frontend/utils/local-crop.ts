import { MAX_OUTPUT_EDGE, resizeDimensions } from './local-resize'
export interface PixelCrop { x: number, y: number, width: number, height: number }
export interface RasterSize { width: number, height: number }
export function validatePixelCrop(crop: PixelCrop, source: RasterSize): PixelCrop {
  if (!Object.values(crop).every(Number.isSafeInteger) || crop.x < 0 || crop.y < 0
    || crop.x + crop.width > source.width || crop.y + crop.height > source.height) throw Error('Keep the selection inside the image using whole pixels.')
  resizeDimensions(String(crop.width), String(crop.height))
  return { ...crop }
}
export function parsePixelCrop(fields: Record<keyof PixelCrop, string>, source: RasterSize) {
  if (!Object.values(fields).every(v => /^\d{1,4}$/.test(v))) throw Error('Enter whole-number crop coordinates and dimensions.')
  return validatePixelCrop({ x: Number(fields.x), y: Number(fields.y), width: Number(fields.width), height: Number(fields.height) }, source)
}
export function initialCrop(source: RasterSize, square = false): PixelCrop {
  let width = Math.min(source.width, MAX_OUTPUT_EDGE), height = Math.min(source.height, MAX_OUTPUT_EDGE)
  if (square) width = height = Math.min(width, height, 2000)
  return { x: Math.floor((source.width - width) / 2), y: Math.floor((source.height - height) / 2), width, height }
}
export function moveCrop(crop: PixelCrop, source: RasterSize, dx: number, dy: number): PixelCrop {
  return { ...crop, x: Math.max(0, Math.min(source.width - crop.width, crop.x + Math.round(dx))), y: Math.max(0, Math.min(source.height - crop.height, crop.y + Math.round(dy))) }
}
