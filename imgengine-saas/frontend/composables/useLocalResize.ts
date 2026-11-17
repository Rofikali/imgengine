import { computed, onBeforeUnmount, ref, shallowRef } from 'vue'
import { inspectPreview, loadCropPreview, type CropPreview } from '~/utils/crop-preview'
import { linkedEdge, MAX_LOCAL_BYTES, MAX_LOCAL_PIXELS, MAX_OUTPUT_EDGE, resizeDimensions } from '~/utils/local-resize'
import { validatePixelCrop, type PixelCrop } from '~/utils/local-crop'

export function useLocalResize(cropSelection?: () => PixelCrop) {
  const source = shallowRef<CropPreview | null>(null)
  const result = ref(''), width = ref(''), height = ref(''), locked = ref(true)
  const busy = ref(false), error = ref(''), message = ref('Choose an image to begin.')
  let generation = 0, controller: AbortController | undefined, stopWorker: (() => void) | undefined
  const clearResult = () => { if (result.value) URL.revokeObjectURL(result.value); result.value = '' }
  function invalidate() { cancel(); clearResult(); error.value = ''; message.value = 'Selection changed. Create a new PNG.' }
  const dimensionsError = computed(() => {
    if (!source.value) return ''
    try { resizeDimensions(width.value, height.value); return '' } catch (e) { return (e as Error).message }
  })
  function cancel() {
    generation++; controller?.abort(); stopWorker?.(); stopWorker = undefined; busy.value = false
    message.value = source.value ? 'Cancelled. Your image stays on this device.' : 'Choose an image to begin.'
  }
  function reset() {
    cancel(); source.value?.dispose(); source.value = null; clearResult()
    width.value = ''; height.value = ''; error.value = ''; message.value = 'Choose an image to begin.'
  }
  async function select(file?: File) {
    reset()
    if (!file) return
    const current = generation, abort = new AbortController(); controller = abort
    busy.value = true; message.value = 'Opening locally…'
    try {
      if (!file.size || file.size > MAX_LOCAL_BYTES) throw Error('Choose a JPEG or PNG up to 4 MiB.')
      const size = inspectPreview(new Uint8Array(await file.arrayBuffer()))
      if (abort.signal.aborted) return
      if (size.width * size.height > MAX_LOCAL_PIXELS) throw Error('Choose an image up to 4 megapixels.')
      const preview = await loadCropPreview(file, abort.signal)
      if (current !== generation) { preview.dispose(); return }
      source.value = preview
      const scale = Math.min(1, MAX_OUTPUT_EDGE / preview.width, MAX_OUTPUT_EDGE / preview.height)
      width.value = String(Math.max(1, Math.floor(preview.width * scale)))
      height.value = String(Math.max(1, Math.floor(preview.height * scale)))
      locked.value = true; message.value = 'Image ready. Set the output size.'
    } catch {
      if (current === generation) {
        error.value = 'Cannot open this image safely. Use a valid, still JPEG or PNG up to 4 MiB, 4 megapixels and 8192 pixels per side.'
        message.value = 'Image unavailable.'
      }
    } finally { if (current === generation) busy.value = false }
  }
  function change(axis: 'width' | 'height', value: string) {
    clearResult(); error.value = ''; message.value = 'Output size changed. Resize to create a new PNG.'
    const target = axis === 'width' ? width : height; target.value = value
    if (locked.value && source.value) {
      const paired = linkedEdge(value, source.value[axis], source.value[axis === 'width' ? 'height' : 'width'])
      if (paired !== null) (axis === 'width' ? height : width).value = paired
    }
  }
  function toggleLock(value: boolean) { locked.value = value; change('width', width.value) }
  async function resize() {
    if (!source.value || busy.value) return
    clearResult(); error.value = ''
    let crop: PixelCrop | undefined
    let outputSize: { width: number, height: number }
    try {
      crop = cropSelection ? validatePixelCrop(cropSelection(), source.value) : undefined
      outputSize = crop ?? resizeDimensions(width.value, height.value)
    } catch (e) { error.value = (e as Error).message; return }
    const current = ++generation, abort = new AbortController(); controller = abort
    busy.value = true; message.value = 'Processing on this device…'
    try {
      // blob: is a local object URL, not an HTTP request or server fallback.
      const blob = await (await fetch(source.value.url, { signal: abort.signal })).blob()
      if (current !== generation) return
      const output = await new Promise<Blob>((resolve, reject) => {
        const worker = new Worker(new URL('../workers/local-resize.ts', import.meta.url), { type: 'module' })
        const finish = (blob?: Blob) => { clearTimeout(timer); worker.terminate(); stopWorker = undefined; blob ? resolve(blob) : reject(Error()) }
        const timer = setTimeout(() => finish(), 10_000)
        stopWorker = () => finish()
        worker.onerror = () => finish()
        worker.onmessage = event => finish(event.data.blob instanceof Blob ? event.data.blob : undefined)
        try { worker.postMessage({ blob, width: String(outputSize.width), height: String(outputSize.height), crop }) } catch { finish() }
      })
      if (current !== generation) return
      result.value = URL.createObjectURL(output); message.value = `Ready: ${outputSize.width} × ${outputSize.height} pixels. Download your PNG.`
    } catch {
      if (current === generation) {
        error.value = 'Local image processing could not finish. Try a smaller image or a browser with Worker image support. Nothing was uploaded.'
        message.value = 'Resize stopped.'
      }
    } finally { if (current === generation) busy.value = false }
  }
  onBeforeUnmount(reset)
  return { source, result, width, height, locked, busy, error, message, dimensionsError, select, change, toggleLock, resize, cancel, reset, invalidate }
}
