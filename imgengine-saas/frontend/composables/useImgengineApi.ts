import type { LayoutRenderInput, LayoutRenderSuccess, RenderApiError, RenderOptions, RenderSuccess, SheetRenderInput, SheetRenderSuccess } from '~/types/api'
import { classifyRenderFailureData } from '~/utils/api-errors'
import { createLayoutRenderForm, createRenderForm, createSheetRenderForm } from '~/utils/render-form'

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

  async function renderLayout(
    file: File,
    input: LayoutRenderInput,
    options: RenderOptions = {},
  ): Promise<LayoutRenderSuccess> {
    const form = createLayoutRenderForm(file, input)
    try {
      const response = await $fetch.raw<Blob>('/api/layout-render', {
        method: 'POST', body: form, responseType: 'blob', signal: options.signal,
      })
      if (!(response._data instanceof Blob)) {
        throw new ImgengineRenderError({ kind: 'internal', status: response.status, requestId: null, message: 'The rendering service returned an invalid response.' })
      }
      const headerNumber = (name: string) => {
        const value = response.headers.get(name)
        const parsed = value === null ? NaN : Number(value)
        if (!Number.isSafeInteger(parsed) || parsed <= 0) throw new ImgengineRenderError({ kind: 'internal', status: response.status, requestId: null, message: 'The rendering service returned invalid layout accounting.' })
        return parsed
      }
      return {
        jpeg: response._data,
        requestId: response.headers.get('x-request-id'),
        canvasWidth: headerNumber('x-imgengine-canvas-width'),
        canvasHeight: headerNumber('x-imgengine-canvas-height'),
        expectedCount: headerNumber('x-imgengine-expected-count'),
        plannedCount: headerNumber('x-imgengine-planned-count'),
        executedCount: headerNumber('x-imgengine-executed-count'),
      }
    } catch (cause: unknown) {
      if (cause instanceof ImgengineRenderError) throw cause
      const fetchError = cause as { response?: { status?: number }, data?: unknown }
      const status = typeof fetchError.response?.status === 'number' ? fetchError.response.status : null
      throw new ImgengineRenderError(await classifyRenderFailureData(status, fetchError.data))
    }
  }

  async function renderSheet(
    file: File,
    input: SheetRenderInput,
    options: RenderOptions = {},
  ): Promise<SheetRenderSuccess> {
    const form = createSheetRenderForm(file, input)
    try {
      const response = await $fetch.raw<Blob>('/api/sheet-render', {
        method: 'POST', body: form, responseType: 'blob', signal: options.signal,
      })
      if (!(response._data instanceof Blob)) {
        throw new ImgengineRenderError({
          kind: 'internal',
          status: response.status,
          requestId: null,
          message: 'The rendering service returned an invalid response.',
        })
      }
      const headerNumber = (name: string) => {
        const value = response.headers.get(name)
        const parsed = value === null ? NaN : Number(value)
        if (!Number.isSafeInteger(parsed) || parsed <= 0) {
          throw new ImgengineRenderError({
            kind: 'internal',
            status: response.status,
            requestId: null,
            message: 'The rendering service returned invalid sheet accounting.',
          })
        }
        return parsed
      }
      return {
        jpeg: response._data,
        requestId: response.headers.get('x-request-id'),
        canvasWidth: headerNumber('x-imgengine-canvas-width'),
        canvasHeight: headerNumber('x-imgengine-canvas-height'),
        expectedCount: headerNumber('x-imgengine-expected-count'),
        plannedCount: headerNumber('x-imgengine-planned-count'),
        executedCount: headerNumber('x-imgengine-executed-count'),
      }
    } catch (cause: unknown) {
      if (cause instanceof ImgengineRenderError) throw cause
      const fetchError = cause as { response?: { status?: number }, data?: unknown }
      const status = typeof fetchError.response?.status === 'number' ? fetchError.response.status : null
      throw new ImgengineRenderError(await classifyRenderFailureData(status, fetchError.data))
    }
  }

  return { render, renderLayout, renderSheet }
}
