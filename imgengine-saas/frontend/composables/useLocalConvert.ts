import { onBeforeUnmount, ref, shallowRef } from 'vue'
import { inspectPreview, loadCropPreview, type CropPreview } from '~/utils/crop-preview'
import { convertFormat, convertedExtension, validConvertedImage, type LocalConvertFormat } from '~/utils/local-convert'
import { MAX_LOCAL_BYTES, MAX_LOCAL_PIXELS } from '~/utils/local-resize'

export function useLocalConvert() {
  const source = shallowRef<CropPreview | null>(null)
  const result = ref(''), format = ref<LocalConvertFormat>('image/png')
  const busy = ref(false), error = ref(''), message = ref('Choose an image to begin.')
  let generation = 0, controller: AbortController | undefined
  const clearResult = () => { if (result.value) URL.revokeObjectURL(result.value); result.value = '' }
  function cancel() { generation++; controller?.abort(); busy.value = false; message.value = source.value ? 'Cancelled. Your image stays on this device.' : 'Choose an image to begin.' }
  function reset() { cancel(); source.value?.dispose(); source.value = null; clearResult(); format.value = 'image/png'; error.value = ''; message.value = 'Choose an image to begin.' }
  function changeFormat(value: string) { try { format.value = convertFormat(value); clearResult(); error.value = ''; message.value = 'Output format changed. Convert to create a new image.' } catch (reason) { error.value = (reason as Error).message } }
  async function select(file?: File) {
    reset(); if (!file) return
    const current = generation, abort = new AbortController(); controller = abort; busy.value = true; message.value = 'Opening locally…'
    try {
      if (!file.size || file.size > MAX_LOCAL_BYTES) throw Error()
      const size = inspectPreview(new Uint8Array(await file.arrayBuffer()))
      if (abort.signal.aborted || size.width * size.height > MAX_LOCAL_PIXELS) throw Error()
      const preview = await loadCropPreview(file, abort.signal)
      if (current !== generation) { preview.dispose(); return }
      source.value = preview; message.value = 'Image ready. Choose an output format.'
    } catch { if (current === generation) { error.value = 'Cannot open this image safely. Use a valid, still JPEG or PNG up to 4 MiB, 4 megapixels and 8192 pixels per side.'; message.value = 'Image unavailable.' } } finally { if (current === generation) busy.value = false }
  }
  async function convert() {
    const preview = source.value; if (!preview || busy.value) return
    clearResult(); error.value = ''; const outputFormat = format.value
    const current = ++generation, abort = new AbortController(); controller = abort; busy.value = true; message.value = 'Converting on this device…'
    try {
      const output = await new Promise<Blob>((resolve, reject) => {
        let image: HTMLImageElement | undefined, canvas: HTMLCanvasElement | undefined
        let settled = false
        let abortConversion: () => void = () => {}
        const finish = (blob?: Blob) => {
          if (settled) return
          settled = true
          clearTimeout(timer); abort.signal.removeEventListener('abort', abortConversion); if (image) { image.onload = image.onerror = null; image.removeAttribute('src') }; if (canvas) { canvas.width = canvas.height = 0 }
          if (blob && current === generation && !abort.signal.aborted && validConvertedImage(blob, outputFormat)) resolve(blob); else reject(Error())
        }
        const timer = setTimeout(() => finish(), 10_000)
        abortConversion = () => finish()
        abort.signal.addEventListener('abort', abortConversion, { once: true })
        try {
          image = new Image(); image.onload = () => { try {
            if (image!.naturalWidth !== preview.width || image!.naturalHeight !== preview.height || preview.width * preview.height > MAX_LOCAL_PIXELS) throw Error()
            canvas = document.createElement('canvas'); canvas.width = preview.width; canvas.height = preview.height
            const context = canvas.getContext('2d'); if (!context) throw Error()
            if (outputFormat === 'image/jpeg') { context.fillStyle = '#ffffff'; context.fillRect(0, 0, preview.width, preview.height) }
            context.drawImage(image!, 0, 0); canvas.toBlob(blob => finish(blob ?? undefined), outputFormat)
          } catch { finish() } }; image.onerror = () => finish(); image.src = preview.url
        } catch { finish() }
      })
      if (current !== generation) return
      result.value = URL.createObjectURL(output); message.value = `Ready: ${outputFormat === 'image/jpeg' ? 'JPEG' : 'PNG'} ${preview.width} × ${preview.height} pixels. Download your image.`
    } catch { if (current === generation) { error.value = 'Local conversion could not finish. Try a smaller image or a browser with supported image encoding. Nothing was uploaded.'; message.value = 'Conversion stopped.' } } finally { if (current === generation) busy.value = false }
  }
  onBeforeUnmount(reset)
  return { source, result, format, busy, error, message, select, changeFormat, convert, cancel, reset, extension: () => convertedExtension(format.value) }
}
