import type { RenderApiError, RenderOptions, RenderSuccess } from '~/types/api'
import { classifyRenderFailureData } from '~/utils/api-errors'
import { createRenderForm } from '~/utils/render-form'

export class ImgengineRenderError extends Error {
  readonly detail: RenderApiError

  constructor(detail: RenderApiError) {
    super(detail.message)
    this.name = 'ImgengineRenderError'
    this.detail = detail
  }
}

/**
 * Browser-facing boundary for the verified synchronous Rust render slice.
 * Authentication is deliberately delegated to the same-origin Nuxt server
 * route; no API key is accepted or retained by this composable.
 */
export function useImgengineApi() {
  async function render(file: File, options: RenderOptions = {}): Promise<RenderSuccess> {
    const form = createRenderForm(file)

    try {
      const response = await $fetch.raw<Blob>('/api/render', {
        method: 'POST',
        body: form,
        responseType: 'blob',
        signal: options.signal,
      })

      if (!(response._data instanceof Blob)) {
        throw new ImgengineRenderError({
          kind: 'internal',
          status: response.status,
          requestId: null,
          message: 'The rendering service returned an invalid response.',
        })
      }

      return {
        jpeg: response._data,
        requestId: response.headers.get('x-request-id'),
      }
    } catch (cause: unknown) {
      if (cause instanceof ImgengineRenderError) throw cause
      const fetchError = cause as { response?: { status?: number }, data?: unknown }
      const status = typeof fetchError.response?.status === 'number'
        ? fetchError.response.status
        : null
      throw new ImgengineRenderError(await classifyRenderFailureData(status, fetchError.data))
    }
  }

  return { render }
}
