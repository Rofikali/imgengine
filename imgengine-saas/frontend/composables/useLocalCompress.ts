import { onBeforeUnmount, ref, shallowRef } from 'vue'
import { inspectPreview, loadCropPreview, type CropPreview } from '~/utils/crop-preview'
import { DEFAULT_JPEG_QUALITY, isLocalCompressBitmap, isLocalCompressSuccess, jpegQuality, type LocalCompressRequest, type LocalCompressResponse } from '~/utils/local-compress'
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
    if (!source.value || busy.value) return
    clearResult(); error.value = ''
    try { jpegQuality(quality.value) } catch (reason) { error.value = (reason as Error).message; return }
    const current = ++generation, abort = new AbortController(); controller = abort
    busy.value = true; message.value = 'Compressing on this device…'
    try {
      const blob = await (await fetch(source.value.url, { signal: abort.signal })).blob()
      if (current !== generation) return
      const output = await new Promise<Blob>((resolve, reject) => {
        const worker = new Worker(new URL('../workers/local-compress.ts', import.meta.url), { type: 'module' })
        const finish = (value?: Blob) => { clearTimeout(timer); worker.terminate(); stopWorker = undefined; value ? resolve(value) : reject(Error()) }
        const encode = (bitmap: ImageBitmap, outputQuality: number) => {
          const canvas = document.createElement('canvas')
          canvas.width = bitmap.width; canvas.height = bitmap.height
          const context = canvas.getContext('2d')
          if (!context) { bitmap.close(); finish(); return }
          context.drawImage(bitmap, 0, 0); bitmap.close()
          canvas.toBlob(encoded => {
            canvas.width = canvas.height = 0
            if (!encoded || encoded.type !== 'image/jpeg' || !encoded.size || encoded.size > 20 * 1024 * 1024) { finish(); return }
            finish(encoded)
          }, 'image/jpeg', outputQuality / 100)
        }
        const timer = setTimeout(() => finish(), 10_000)
        stopWorker = () => finish()
        worker.onerror = () => finish()
        worker.onmessage = (event: MessageEvent<LocalCompressResponse>) => {
          if (isLocalCompressSuccess(event.data)) finish(event.data.blob)
          else if (isLocalCompressBitmap(event.data)) encode(event.data.bitmap, event.data.quality)
          else finish()
        }
        try { worker.postMessage({ blob, quality: quality.value } satisfies LocalCompressRequest) } catch { finish() }
      })
      if (current !== generation) return
      result.value = URL.createObjectURL(output)
      message.value = `Ready: JPEG quality ${quality.value}. Download your compressed image.`
    } catch {
      if (current === generation) {
        error.value = 'Local compression could not finish. Try a smaller image or a browser with Worker image support. Nothing was uploaded.'
        message.value = 'Compression stopped.'
      }
    } finally { if (current === generation) busy.value = false }
  }
  onBeforeUnmount(reset)
  return { source, result, quality, busy, error, message, select, changeQuality, compress, cancel, reset }
}
