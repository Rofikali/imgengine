import { MAX_LOCAL_PIXELS, resizeDimensions } from '../utils/local-resize'
import { validatePixelCrop, type PixelCrop } from '../utils/local-crop'

self.onmessage = async (event: MessageEvent<{ blob: Blob, width: string, height: string, crop?: PixelCrop }>) => {
  let bitmap: ImageBitmap | undefined
  let canvas: OffscreenCanvas | undefined
  try {
    const size = resizeDimensions(event.data.width, event.data.height)
    bitmap = await createImageBitmap(event.data.blob)
    if (!bitmap.width || !bitmap.height || bitmap.width * bitmap.height > MAX_LOCAL_PIXELS) throw Error()
    const crop = event.data.crop ? validatePixelCrop(event.data.crop, bitmap) : undefined
    if (crop && (size.width !== crop.width || size.height !== crop.height)) throw Error()
    canvas = new OffscreenCanvas(size.width, size.height)
    const context = canvas.getContext('2d')
    if (!context) throw Error()
    context.imageSmoothingEnabled = true
    context.imageSmoothingQuality = 'high'
    if (crop) {
      context.drawImage(bitmap, crop.x, crop.y, crop.width, crop.height, 0, 0, crop.width, crop.height)
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
