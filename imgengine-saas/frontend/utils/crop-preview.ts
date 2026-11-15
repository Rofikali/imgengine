import { MAX_PREVIEW_EDGE, MAX_PREVIEW_PIXELS, type ImageSize } from './crop-editor'

export interface CropPreview extends ImageSize { url: string, dispose: () => void }
export class CropPreviewError extends Error {}
const invalid = () => new CropPreviewError('This image cannot be previewed safely. Choose another JPEG or PNG.')
const orientationError = () => new CropPreviewError('This photo has orientation metadata the crop service cannot match yet. Export an upright copy without orientation metadata to use the editor.')

function checkExif(bytes: Uint8Array) {
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
  if (bytes.length < 8) throw invalid()
  const little = view.getUint16(0) === 0x4949
  if ((!little && view.getUint16(0) !== 0x4d4d) || view.getUint16(2, little) !== 42) throw invalid()
  const offset = view.getUint32(4, little)
  if (offset < 8 || offset + 2 > bytes.length) throw invalid()
  const count = view.getUint16(offset, little)
  if (offset + 2 + count * 12 + 4 > bytes.length) throw invalid()
  for (let index = 0; index < count; index++) {
    const entry = offset + 2 + index * 12
    if (view.getUint16(entry, little) !== 0x112) continue
    if (view.getUint16(entry + 2, little) !== 3 || view.getUint32(entry + 4, little) !== 1
      || view.getUint16(entry + 8, little) !== 1) throw orientationError()
  }
}

/** Bounded header preflight before browser decode. This is a UX resource guard,
 * not a substitute for native validation or a browser decoder memory guarantee. */
export function inspectPreview(bytes: Uint8Array): ImageSize {
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
  let size: ImageSize | undefined
  if (bytes.length >= 33 && view.getUint32(0) === 0x89504e47 && view.getUint32(4) === 0x0d0a1a0a) {
    if (view.getUint32(8) !== 13 || view.getUint32(12) !== 0x49484452) throw invalid()
    size = { width: view.getUint32(16), height: view.getUint32(20) }
    for (let offset = 8; offset < bytes.length;) {
      if (offset + 12 > bytes.length) throw invalid()
      const length = view.getUint32(offset)
      if (length > bytes.length - offset - 12) throw invalid()
      const type = view.getUint32(offset + 4)
      if (type === 0x65584966) checkExif(bytes.subarray(offset + 8, offset + 8 + length))
      // Animated PNG has no single stable source frame contract.
      if (type === 0x6163544c) throw invalid()
      offset += length + 12
    }
  } else if (bytes.length >= 4 && view.getUint16(0) === 0xffd8) {
    for (let offset = 2; offset < bytes.length;) {
      if (bytes[offset++] !== 0xff) throw invalid()
      while (bytes[offset] === 0xff) offset++
      const marker = bytes[offset++]
      if (marker === 0xda || marker === 0xd9) break
      if (offset + 2 > bytes.length) throw invalid()
      const length = view.getUint16(offset)
      if (length < 2 || offset + length > bytes.length) throw invalid()
      if (marker === 0xe1 && length >= 8 && view.getUint32(offset + 2) === 0x45786966 && view.getUint16(offset + 6) === 0) {
        checkExif(bytes.subarray(offset + 8, offset + length))
      }
      if ([0xc0, 0xc1, 0xc2].includes(marker ?? 0)) {
        if (length < 8 || size) throw invalid()
        size = { width: view.getUint16(offset + 5), height: view.getUint16(offset + 3) }
      }
      offset += length
    }
  }
  if (!size || size.width < 1 || size.height < 1) throw invalid()
  if (size.width > MAX_PREVIEW_EDGE || size.height > MAX_PREVIEW_EDGE || size.width * size.height > MAX_PREVIEW_PIXELS) {
    throw new CropPreviewError('The crop preview supports images up to 16 megapixels and 8192 pixels per side. Choose a smaller image.')
  }
  return size
}

export async function loadCropPreview(file: File, signal: AbortSignal): Promise<CropPreview> {
  const size = inspectPreview(new Uint8Array(await file.arrayBuffer()))
  if (signal.aborted) throw invalid()
  const url = URL.createObjectURL(file)
  const image = new Image()
  let disposed = false
  const dispose = () => {
    image.removeAttribute('src')
    if (!disposed) URL.revokeObjectURL(url)
    disposed = true
  }
  try {
    await new Promise<void>((resolve, reject) => {
      const finish = (error?: Error) => {
        image.onload = image.onerror = null
        signal.removeEventListener('abort', abort)
        clearTimeout(timer)
        error ? reject(error) : resolve()
      }
      const abort = () => { dispose(); finish(invalid()) }
      const timer = setTimeout(abort, 10_000)
      signal.addEventListener('abort', abort, { once: true })
      image.onload = () => finish()
      image.onerror = () => finish(invalid())
      image.src = url
    })
    if (signal.aborted || image.naturalWidth !== size.width || image.naturalHeight !== size.height) throw invalid()
    image.removeAttribute('src')
    return { ...size, url, dispose }
  } catch (error) {
    dispose()
    throw error
  }
}
