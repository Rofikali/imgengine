import { computed, nextTick, onBeforeUnmount, ref } from 'vue'
import { ImgengineRenderError, useImgengineApi } from '~/composables/useImgengineApi'
import type { RenderApiError, RenderOptions, SheetRenderInput, SheetRenderSuccess } from '~/types/api'
import { CLIENT_RENDER_FILE_LIMIT_BYTES } from '~/composables/useRenderWorkflow'

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
) {
  const selectedFile = ref<File | null>(null)
  const status = ref<SheetWorkflowStatus>('idle')
  const failure = ref<SheetWorkflowFailure | null>(null)
  const result = ref<SheetRenderSuccess | null>(null)
  const previewUrl = ref<string | null>(null)
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
  let activeController: AbortController | null = null

  function cancelActive() {
    const controller = activeController
    activeController = null
    controller?.abort()
  }

  function releasePreview() {
    if (previewUrl.value) URL.revokeObjectURL(previewUrl.value)
    previewUrl.value = null
  }

  function select(file: File | null) {
    cancelActive()
    releasePreview()
    result.value = null
    selectedFile.value = file
    failure.value = validateFile(file)
    status.value = failure.value ? 'error' : 'ready'
  }

  async function submit() {
    if (isSubmitting.value) return
    const invalid = validateFile(selectedFile.value)
    if (invalid) {
      failure.value = invalid
      status.value = 'error'
      return
    }
    failure.value = null
    const controller = new AbortController()
    activeController = controller
    status.value = 'uploading'
    await nextTick()
    if (activeController !== controller || controller.signal.aborted) return
    status.value = 'processing'
    try {
      const completed = await renderSheet(selectedFile.value!, sheet.value, { signal: controller.signal })
      if (activeController !== controller || controller.signal.aborted) return
      releasePreview()
      selectedFile.value = null
      result.value = completed
      previewUrl.value = URL.createObjectURL(completed.jpeg)
      status.value = 'success'
    } catch (cause) {
      if (activeController !== controller || controller.signal.aborted) return
      result.value = null
      failure.value = cause instanceof ImgengineRenderError
        ? { message: cause.detail.message, detail: cause.detail }
        : { message: 'The connection to the rendering service was interrupted.', detail: null }
      status.value = 'error'
    } finally {
      if (activeController === controller) activeController = null
    }
  }

  function reset() {
    cancelActive()
    releasePreview()
    selectedFile.value = null
    result.value = null
    failure.value = null
    status.value = 'idle'
  }

  return { selectedFile, status, failure, result, previewUrl, isSubmitting, sheet, select, submit, reset }
}

/** Component-scoped wrapper that revokes its preview URL on unmount. */
export function useSheetRenderWorkflow() {
  const { renderSheet } = useImgengineApi()
  const workflow = createSheetRenderWorkflow(renderSheet)
  onBeforeUnmount(workflow.reset)
  return workflow
}
