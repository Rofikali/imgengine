import { computed, nextTick, onBeforeUnmount, ref } from 'vue'
import { ImgengineRenderError, useImgengineApi } from '~/composables/useImgengineApi'
import type { LayoutRenderInput, LayoutRenderSuccess, RenderApiError } from '~/types/api'
import { CLIENT_RENDER_FILE_LIMIT_BYTES } from '~/composables/useRenderWorkflow'

export type LayoutWorkflowStatus = 'idle' | 'ready' | 'uploading' | 'processing' | 'success' | 'error'
export interface LayoutWorkflowFailure { message: string, detail: RenderApiError | null }

function validateFile(file: File | null): LayoutWorkflowFailure | null {
  if (!file) return { message: 'Choose a JPEG or PNG image to continue.', detail: null }
  if (file.type && file.type !== 'image/jpeg' && file.type !== 'image/png') return { message: 'This file type is not supported. Please choose a JPEG or PNG image.', detail: null }
  if (file.size > CLIENT_RENDER_FILE_LIMIT_BYTES) return { message: 'The image is too large. Please choose a file under about 1 MB.', detail: null }
  return null
}

/** Ephemeral UI state for the server-calculated, synchronous FIT-only layout flow. */
export function useLayoutRenderWorkflow() {
  const { renderLayout } = useImgengineApi()
  const selectedFile = ref<File | null>(null)
  const status = ref<LayoutWorkflowStatus>('idle')
  const failure = ref<LayoutWorkflowFailure | null>(null)
  const result = ref<LayoutRenderSuccess | null>(null)
  const previewUrl = ref<string | null>(null)
  const layout = ref<LayoutRenderInput>({ width: '100', height: '100', unit: 'px', dpi: '300', copies: '1', columns: '1', gap: '0', margin: '0', outputFormat: 'jpeg' })
  const isSubmitting = computed(() => status.value === 'uploading' || status.value === 'processing')
  function releasePreview() { if (previewUrl.value) URL.revokeObjectURL(previewUrl.value); previewUrl.value = null }
  function select(file: File | null) {
    releasePreview(); result.value = null; selectedFile.value = file
    failure.value = validateFile(file); status.value = failure.value ? 'error' : 'ready'
  }
  async function submit() {
    if (isSubmitting.value) return
    const invalid = validateFile(selectedFile.value)
    if (invalid) { failure.value = invalid; status.value = 'error'; return }
    failure.value = null; status.value = 'uploading'; await nextTick(); status.value = 'processing'
    try {
      const completed = await renderLayout(selectedFile.value!, layout.value)
      releasePreview(); selectedFile.value = null; result.value = completed
      previewUrl.value = URL.createObjectURL(completed.jpeg); status.value = 'success'
    } catch (cause) {
      result.value = null
      failure.value = cause instanceof ImgengineRenderError ? { message: cause.detail.message, detail: cause.detail } : { message: 'The connection to the rendering service was interrupted.', detail: null }
      status.value = 'error'
    }
  }
  function reset() { releasePreview(); selectedFile.value = null; result.value = null; failure.value = null; status.value = 'idle' }
  onBeforeUnmount(reset)
  return { selectedFile, status, failure, result, previewUrl, isSubmitting, layout, select, submit, reset }
}
