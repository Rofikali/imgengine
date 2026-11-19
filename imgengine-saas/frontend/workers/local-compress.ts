import { MAX_LOCAL_PIXELS } from '../utils/local-resize'
import { jpegQuality, type LocalCompressRequest, type LocalCompressResponse } from '../utils/local-compress'

self.onmessage = async (event: MessageEvent<LocalCompressRequest>) => {
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
    const output = canvas.transferToImageBitmap()
    self.postMessage({ bitmap: output, quality } satisfies LocalCompressResponse, [output])
  } catch {
    self.postMessage({ error: true } satisfies LocalCompressResponse)
  } finally {
    bitmap?.close()
    if (canvas) { canvas.width = 0; canvas.height = 0 }
  }
}
