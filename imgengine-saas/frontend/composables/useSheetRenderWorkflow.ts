import { computed, nextTick, onBeforeUnmount, ref, shallowRef, watch } from 'vue'
import { ImgengineRenderError, useImgengineApi } from '~/composables/useImgengineApi'
import type { RenderApiError, RenderOptions, SheetRenderInput, SheetRenderSuccess } from '~/types/api'
import { CLIENT_RENDER_FILE_LIMIT_BYTES } from '~/composables/useRenderWorkflow'
import { defaultCropPosition, editorCrop, photoRaster, type CropPosition } from '~/utils/crop-editor'
import { CropPreviewError, loadCropPreview, type CropPreview } from '~/utils/crop-preview'

export type SheetWorkflowStatus = 'idle' | 'ready' | 'uploading' | 'processing' | 'success' | 'error'
export interface SheetWorkflowFailure { message: string, detail: RenderApiError | null }

function validateFile(file: File | null): SheetWorkflowFailure | null {
  if (!file) return { message: 'Choose a JPEG or PNG image to continue.', detail: null }
  if (file.type && file.type !== 'image/jpeg' && file.type !== 'image/png') {
    return { message: 'This file type is not supported. Please choose a JPEG or PNG image.', detail: null }
  }
  if (file.size > CLIENT_RENDER_FILE_LIMIT_BYTES) {
    return { message: 'The image is too large. Please choose a file under about 1 MB.', detail: null }
  }
  return null
}

/** Ephemeral UI state for the server-planned P12 physical-sheet workflow. */
export function createSheetRenderWorkflow(
  renderSheet: (file: File, input: SheetRenderInput, options?: RenderOptions) => Promise<SheetRenderSuccess>,
  loadPreview = loadCropPreview,
) {
  const selectedFile = ref<File | null>(null)
  const status = ref<SheetWorkflowStatus>('idle')
  const failure = ref<SheetWorkflowFailure | null>(null)
  const result = ref<SheetRenderSuccess | null>(null)
  const previewUrl = ref<string | null>(null)
  const sourcePreview = shallowRef<CropPreview | null>(null)
  const previewLoading = ref(false)
  const previewError = ref<string | null>(null)
  const position = ref(defaultCropPosition())
  const cropSelected = ref(false)
  let previewController: AbortController | null = null
  const sheet = ref<SheetRenderInput>({
    sheetWidthMm: '210',
    sheetHeightMm: '297',
    photoWidthMm: '35',
    photoHeightMm: '45',
    dpi: '300',
    copies: '8',
    marginMm: '10',
    gapXmm: '2',
    gapYmm: '2',
    outputFormat: 'jpeg',
  })
  const isSubmitting = computed(() => status.value === 'uploading' || status.value === 'processing')
  const target = computed(() => photoRaster(sheet.value.photoWidthMm, sheet.value.photoHeightMm, sheet.value.dpi))
  const crop = computed(() => sourcePreview.value && target.value ? editorCrop(sourcePreview.value, target.value, position.value) : null)
  let activeController: AbortController | null = null

  function resetCrop() {
    position.value = defaultCropPosition()
    cropSelected.value = false
  }
  watch(() => [sheet.value.photoWidthMm, sheet.value.photoHeightMm, sheet.value.dpi], resetCrop, { flush: 'sync' })

  function changeCrop(value: CropPosition) {
    if (isSubmitting.value) return
    position.value = value
    cropSelected.value = true
  }

  function releaseSource() {
    previewController?.abort()
    previewController = null
    sourcePreview.value?.dispose()
    sourcePreview.value = null
    previewLoading.value = false
    resetCrop()
  }

  function cancelActive() {
    const controller = activeController
    activeController = null
    controller?.abort()
  }

  function releasePreview() {
    if (previewUrl.value) URL.revokeObjectURL(previewUrl.value)
    previewUrl.value = null
  }

  async function select(file: File | null) {
    cancelActive()
    releasePreview()
    releaseSource()
    previewError.value = null
    result.value = null
    selectedFile.value = file
    failure.value = validateFile(file)
    status.value = failure.value ? 'error' : 'ready'
    if (failure.value || !file) return
    const controller = new AbortController()
    previewController = controller
    previewLoading.value = true
    try {
      const loaded = await loadPreview(file, controller.signal)
      if (previewController !== controller || controller.signal.aborted) { loaded.dispose(); return }
      sourcePreview.value = loaded
    } catch (cause) {
      if (previewController !== controller || controller.signal.aborted) return
      previewError.value = cause instanceof CropPreviewError ? cause.message : 'The crop preview could not be loaded. Choose another JPEG or PNG.'
    } finally {
      if (previewController === controller) { previewController = null; previewLoading.value = false }
    }
  }

  async function submit() {
    if (isSubmitting.value || previewLoading.value) return
    const invalid = validateFile(selectedFile.value)
    if (invalid) {
      failure.value = invalid
      status.value = 'error'
      return
    }
    failure.value = null
    if (cropSelected.value && !crop.value) {
      failure.value = { message: 'Choose valid photo dimensions or reset the crop before rendering.', detail: null }
      status.value = 'error'
      return
    }
    const input = { ...sheet.value, crop: cropSelected.value ? crop.value! : undefined }
    const file = selectedFile.value!
    const controller = new AbortController()
    activeController = controller
    status.value = 'uploading'
    await nextTick()
    if (activeController !== controller || controller.signal.aborted) return
    status.value = 'processing'
    try {
      const completed = await renderSheet(file, input, { signal: controller.signal })
      if (activeController !== controller || controller.signal.aborted) return
      releasePreview()
      releaseSource()
      selectedFile.value = null
      result.value = completed
      previewUrl.value = URL.createObjectURL(completed.jpeg)
      status.value = 'success'
    } catch (cause) {
      if (activeController !== controller || controller.signal.aborted) return
      result.value = null
      releaseSource()
      previewError.value = 'The crop preview was released after the failed render. Choose the photo again to adjust its crop, or retry with centered fill.'
      failure.value = cause instanceof ImgengineRenderError
        ? { message: cause.detail.message, detail: cause.detail }
        : { message: 'The connection to the rendering service was interrupted.', detail: null }
      if (input.crop && cause instanceof ImgengineRenderError && [400, 413].includes(cause.detail.status ?? 0)) {
        failure.value.message = 'The service could not accept this crop or image within its limits. Choose the photo again to adjust the crop, or retry with centered fill.'
      }
      status.value = 'error'
    } finally {
      if (activeController === controller) activeController = null
    }
  }

  function reset() {
    cancelActive()
    releasePreview()
    releaseSource()
    previewError.value = null
    selectedFile.value = null
    result.value = null
    failure.value = null
    status.value = 'idle'
  }

  return { selectedFile, status, failure, result, previewUrl, isSubmitting, sheet, select, submit, reset,
    sourcePreview, previewLoading, previewError, position, cropSelected, target, crop, changeCrop, resetCrop }
}

/** Component-scoped wrapper that revokes its preview URL on unmount. */
export function useSheetRenderWorkflow() {
  const { renderSheet } = useImgengineApi()
  const workflow = createSheetRenderWorkflow(renderSheet)
  onBeforeUnmount(workflow.reset)
  return workflow
}
