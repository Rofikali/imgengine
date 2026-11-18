import { MAX_LOCAL_PIXELS } from '../utils/local-resize'
import { jpegQuality } from '../utils/local-compress'

self.onmessage = async (event: MessageEvent<{ blob: Blob, quality: string }>) => {
  let bitmap: ImageBitmap | undefined
  let canvas: OffscreenCanvas | undefined
  try {
    const quality = jpegQuality(event.data.quality)
    bitmap = await createImageBitmap(event.data.blob)
    if (!bitmap.width || !bitmap.height || bitmap.width * bitmap.height > MAX_LOCAL_PIXELS) throw Error()
    canvas = new OffscreenCanvas(bitmap.width, bitmap.height)
    const context = canvas.getContext('2d')
    if (!context) throw Error()
    // JPEG has no alpha channel. White compositing is deliberate and visible in
    // the UI rather than silently converting transparent pixels to black.
    context.fillStyle = '#ffffff'
    context.fillRect(0, 0, bitmap.width, bitmap.height)
    context.drawImage(bitmap, 0, 0)
    const blob = await canvas.convertToBlob({ type: 'image/jpeg', quality: quality / 100 })
    if (blob.type !== 'image/jpeg' || !blob.size || blob.size > 20 * 1024 * 1024) throw Error()
    self.postMessage({ blob, quality })
  } catch {
    self.postMessage({ error: true })
  } finally {
    bitmap?.close()
    if (canvas) { canvas.width = 0; canvas.height = 0 }
  }
}
