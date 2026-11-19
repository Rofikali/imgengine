import { onBeforeUnmount, ref, shallowRef } from 'vue'
import { inspectPreview, loadCropPreview, type CropPreview } from '~/utils/crop-preview'
import { DEFAULT_JPEG_QUALITY, isLocalCompressBitmap, isLocalCompressSuccess, jpegQuality, validCompressedJpeg, type LocalCompressRequest, type LocalCompressResponse } from '~/utils/local-compress'
import { MAX_LOCAL_BYTES, MAX_LOCAL_PIXELS } from '~/utils/local-resize'

export function useLocalCompress() {
  const source = shallowRef<CropPreview | null>(null)
  const result = ref(''), quality = ref(String(DEFAULT_JPEG_QUALITY))
  const busy = ref(false), error = ref(''), message = ref('Choose an image to begin.')
  let generation = 0, controller: AbortController | undefined, stopWorker: (() => void) | undefined
  const clearResult = () => { if (result.value) URL.revokeObjectURL(result.value); result.value = '' }

  function cancel() {
    generation++; controller?.abort(); stopWorker?.(); stopWorker = undefined; busy.value = false
    message.value = source.value ? 'Cancelled. Your image stays on this device.' : 'Choose an image to begin.'
  }
  function reset() {
    cancel(); source.value?.dispose(); source.value = null; clearResult(); quality.value = String(DEFAULT_JPEG_QUALITY)
    error.value = ''; message.value = 'Choose an image to begin.'
  }
  function changeQuality(value: string) {
    clearResult(); error.value = ''; quality.value = value
    message.value = 'Quality changed. Compress to create a new JPEG.'
  }
  async function select(file?: File) {
    reset()
    if (!file) return
    const current = generation, abort = new AbortController(); controller = abort
    busy.value = true; message.value = 'Opening locally…'
    try {
      if (!file.size || file.size > MAX_LOCAL_BYTES) throw Error()
      const size = inspectPreview(new Uint8Array(await file.arrayBuffer()))
      if (abort.signal.aborted || size.width * size.height > MAX_LOCAL_PIXELS) throw Error()
      const preview = await loadCropPreview(file, abort.signal)
      if (current !== generation) { preview.dispose(); return }
      source.value = preview; message.value = 'Image ready. Choose JPEG quality.'
    } catch {
      if (current === generation) {
        error.value = 'Cannot open this image safely. Use a valid, still JPEG or PNG up to 4 MiB, 4 megapixels and 8192 pixels per side.'
        message.value = 'Image unavailable.'
      }
    } finally { if (current === generation) busy.value = false }
  }

  async function compress() {
    const preview = source.value
    if (!preview || busy.value) return
    clearResult(); error.value = ''
    let outputQuality: number
    try { outputQuality = jpegQuality(quality.value) } catch (reason) { error.value = (reason as Error).message; return }
    const current = ++generation, abort = new AbortController(); controller = abort
    busy.value = true; message.value = 'Compressing on this device…'
    try {
      // The preview is already size-checked and has its EXIF orientation removed.
      // Fetching its local blob: URL is not a request to Nuxt or the Rust backend.
      const blob = await (await fetch(preview.url, { signal: abort.signal })).blob()
      if (current !== generation) return
      const output = await new Promise<Blob>((resolve, reject) => {
        let worker: Worker | undefined, image: HTMLImageElement | undefined, canvas: HTMLCanvasElement | undefined
        let timer: ReturnType<typeof setTimeout> | undefined
        let settled = false, fallbackStarted = false

        const arm = (milliseconds: number, onTimeout: () => void) => {
          if (timer) clearTimeout(timer)
          timer = setTimeout(onTimeout, milliseconds)
        }
        const finish = (encoded?: Blob) => {
          if (settled) return
          settled = true
          if (timer) clearTimeout(timer)
          worker?.terminate(); worker = undefined
          if (image) { image.onload = image.onerror = null; image.removeAttribute('src'); image = undefined }
          if (canvas) { canvas.width = canvas.height = 0; canvas = undefined }
          stopWorker = undefined
          if (encoded && validCompressedJpeg(encoded) && current === generation && !abort.signal.aborted) resolve(encoded)
          else reject(Error('Local compression did not produce a safe JPEG.'))
        }
        const encode = (drawable: CanvasImageSource, width: number, height: number, onError: () => void) => {
          try {
            if (width !== preview.width || height !== preview.height || width * height > MAX_LOCAL_PIXELS) throw Error()
            canvas = document.createElement('canvas')
            canvas.width = width; canvas.height = height
            const context = canvas.getContext('2d')
            if (!context) throw Error()
            context.fillStyle = '#ffffff'
            context.fillRect(0, 0, width, height)
            context.drawImage(drawable, 0, 0)
            canvas.toBlob(encoded => {
              if (settled) return
              if (!validCompressedJpeg(encoded)) { onError(); return }
              finish(encoded)
            }, 'image/jpeg', outputQuality / 100)
          } catch { onError() }
        }
        // Some browsers accept JPEG in HTMLImageElement but cannot decode or
        // transfer the same JPEG as an ImageBitmap in a Worker (notably Firefox).
        // Use a bounded local-only fallback, never a server request.
        const fallback = () => {
          if (settled || fallbackStarted || current !== generation || abort.signal.aborted) { if (!settled) finish(); return }
          fallbackStarted = true
          worker?.terminate(); worker = undefined
          arm(8500, () => finish())
          try {
            const img = new Image()
            image = img
            img.onload = () => {
              if (settled || current !== generation || abort.signal.aborted) { finish(); return }
              encode(img, img.naturalWidth, img.naturalHeight, () => finish())
            }
            img.onerror = () => finish()
            img.src = preview.url
          } catch { finish() }
        }
        stopWorker = () => finish()
        arm(6000, fallback)
        try {
          worker = new Worker(new URL('../workers/local-compress.ts', import.meta.url), { type: 'module' })
          worker.onerror = fallback
          worker.onmessage = (event: MessageEvent<LocalCompressResponse>) => {
            if (settled || fallbackStarted) {
              if (isLocalCompressBitmap(event.data)) event.data.bitmap.close()
              return
            }
            if (isLocalCompressSuccess(event.data)) { finish(event.data.blob); return }
            if (isLocalCompressBitmap(event.data)) {
              const bitmap = event.data.bitmap
              try { encode(bitmap, bitmap.width, bitmap.height, fallback) }
              finally { bitmap.close() }
              return
            }
            fallback()
          }
          worker.postMessage({ blob, quality: String(outputQuality) } satisfies LocalCompressRequest)
        } catch { fallback() }
      })
      if (current !== generation) return
      result.value = URL.createObjectURL(output)
      message.value = `Ready: JPEG quality ${outputQuality}. Download your compressed image.`
    } catch {
      if (current === generation) {
        error.value = 'Local compression could not finish. Try a smaller image or a browser with supported image encoding. Nothing was uploaded.'
        message.value = 'Compression stopped.'
      }
    } finally { if (current === generation) busy.value = false }
  }
  onBeforeUnmount(reset)
  return { source, result, quality, busy, error, message, select, changeQuality, compress, cancel, reset }
}
