import { MAX_LOCAL_PIXELS, resizeDimensions } from '../utils/local-resize'
import { validatePixelCrop, type PixelCrop } from '../utils/local-crop'
import { rotatedSize, validateRightAngleRotation, type RightAngleRotation } from '../utils/local-rotate'

self.onmessage = async (event: MessageEvent<{ blob: Blob, width: string, height: string, crop?: PixelCrop, rotation?: RightAngleRotation }>) => {
  let bitmap: ImageBitmap | undefined
  let canvas: OffscreenCanvas | undefined
  try {
    const size = resizeDimensions(event.data.width, event.data.height)
    bitmap = await createImageBitmap(event.data.blob)
    if (!bitmap.width || !bitmap.height || bitmap.width * bitmap.height > MAX_LOCAL_PIXELS) throw Error()
    const crop = event.data.crop ? validatePixelCrop(event.data.crop, bitmap) : undefined
    const rotation = event.data.rotation === undefined ? undefined : validateRightAngleRotation(event.data.rotation)
    if (crop && (size.width !== crop.width || size.height !== crop.height)) throw Error()
    if (rotation !== undefined && (crop || size.width !== rotatedSize(bitmap, rotation).width || size.height !== rotatedSize(bitmap, rotation).height)) throw Error()
    canvas = new OffscreenCanvas(size.width, size.height)
    const context = canvas.getContext('2d')
    if (!context) throw Error()
    context.imageSmoothingEnabled = true
    context.imageSmoothingQuality = 'high'
    if (crop) {
      context.drawImage(bitmap, crop.x, crop.y, crop.width, crop.height, 0, 0, crop.width, crop.height)
    } else if (rotation === 90) {
      context.translate(size.width, 0); context.rotate(Math.PI / 2); context.drawImage(bitmap, 0, 0)
    } else if (rotation === 180) {
      context.translate(size.width, size.height); context.rotate(Math.PI); context.drawImage(bitmap, 0, 0)
    } else if (rotation === 270) {
      context.translate(0, size.height); context.rotate(-Math.PI / 2); context.drawImage(bitmap, 0, 0)
    } else context.drawImage(bitmap, 0, 0, size.width, size.height)
    const blob = await canvas.convertToBlob({ type: 'image/png' })
    if (blob.type !== 'image/png' || !blob.size || blob.size > 20 * 1024 * 1024) throw Error()
    self.postMessage({ blob, ...size })
  } catch {
    self.postMessage({ error: true })
  } finally {
    bitmap?.close()
    if (canvas) { canvas.width = 0; canvas.height = 0 }
  }
}
