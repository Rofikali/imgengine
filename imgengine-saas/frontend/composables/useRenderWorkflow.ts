import { computed, nextTick, onBeforeUnmount, ref } from 'vue'
import { ImgengineRenderError, useImgengineApi } from '~/composables/useImgengineApi'
import type { RenderApiError, RenderSuccess } from '~/types/api'

/** The current documented P5/P6 request cap. The Rust service remains authoritative. */
export const CLIENT_RENDER_FILE_LIMIT_BYTES = 1_048_576

export type RenderWorkflowStatus = 'idle' | 'ready' | 'uploading' | 'processing' | 'success' | 'error'

export interface RenderWorkflowFailure {
  message: string
  detail: RenderApiError | null
}

type RenderExecutor = (file: File) => Promise<RenderSuccess>

function clientFailure(message: string): RenderWorkflowFailure {
  return { message, detail: null }
}

function validateFile(file: File | null): RenderWorkflowFailure | null {
  if (!file) return clientFailure('Choose a JPEG or PNG image to continue.')
  if (file.type && file.type !== 'image/jpeg' && file.type !== 'image/png') {
    return clientFailure('This file type is not supported. Please choose a JPEG or PNG image.')
  }
  if (file.size > CLIENT_RENDER_FILE_LIMIT_BYTES) {
    return clientFailure('The image is too large. Please choose a file under 1 MiB.')
  }
  return null
}

/** Local ephemeral presentation state for the synchronous P5/P6 render flow. */
export function createRenderWorkflow(render: RenderExecutor) {
  const selectedFile = ref<File | null>(null)
  const status = ref<RenderWorkflowStatus>('idle')
  const failure = ref<RenderWorkflowFailure | null>(null)
  const result = ref<RenderSuccess | null>(null)
  const previewUrl = ref<string | null>(null)
  const isSubmitting = computed(() => status.value === 'uploading' || status.value === 'processing')

  function releasePreview() {
    if (previewUrl.value) URL.revokeObjectURL(previewUrl.value)
    previewUrl.value = null
  }

  function select(file: File | null) {
    releasePreview()
    result.value = null
    selectedFile.value = file
    const invalid = validateFile(file)
    failure.value = invalid
    status.value = invalid ? 'error' : 'ready'
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
    status.value = 'uploading'
    // There is no API progress telemetry. Once the request is underway, the
    // only honest server-side state the UI can show is processing.
    await nextTick()
    status.value = 'processing'

    try {
      const completed = await render(selectedFile.value!)
      releasePreview()
      result.value = completed
      previewUrl.value = URL.createObjectURL(completed.jpeg)
      status.value = 'success'
    } catch (cause) {
      result.value = null
      failure.value = cause instanceof ImgengineRenderError
        ? { message: cause.detail.message, detail: cause.detail }
        : clientFailure('The connection to the rendering service was interrupted.')
      status.value = 'error'
    }
  }

  function reset() {
    releasePreview()
    selectedFile.value = null
    result.value = null
    failure.value = null
    status.value = 'idle'
  }

  return { selectedFile, status, failure, result, previewUrl, isSubmitting, select, submit, reset }
}

export function useRenderWorkflow() {
  const { render } = useImgengineApi()
  const workflow = createRenderWorkflow(render)
  onBeforeUnmount(workflow.reset)
  return workflow
}
