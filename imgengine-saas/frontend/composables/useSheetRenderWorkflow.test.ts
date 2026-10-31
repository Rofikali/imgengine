import { describe, expect, it, vi } from 'vitest'
import { nextTick } from 'vue'
import { CLIENT_RENDER_FILE_LIMIT_BYTES } from './useRenderWorkflow'
import { createSheetRenderWorkflow } from './useSheetRenderWorkflow'
import { ImgengineRenderError } from './useImgengineApi'

function image(type = 'image/jpeg', size = 5) {
  return new File([new Uint8Array(size)], 'source-image', { type })
}

const input = {
  sheetWidthMm: '210',
  sheetHeightMm: '297',
  photoWidthMm: '35',
  photoHeightMm: '45',
  dpi: '300',
  copies: '8',
  marginMm: '10',
  gapXmm: '2',
  gapYmm: '2',
  outputFormat: 'jpeg' as const,
}

describe('P12 physical-sheet workflow', () => {
  it.each(['image/jpeg', 'image/png'])('accepts a valid %s selection', (type) => {
    const workflow = createSheetRenderWorkflow(vi.fn())
    workflow.select(image(type))
    expect(workflow.status.value).toBe('ready')
    expect(workflow.failure.value).toBeNull()
  })

  it('rejects missing, unsupported, and oversized files before a request', async () => {
    const render = vi.fn()
    const workflow = createSheetRenderWorkflow(render)
    workflow.select(null)
    expect(workflow.failure.value?.message).toMatch(/Choose a JPEG or PNG/)
    workflow.select(image('image/gif'))
    expect(workflow.failure.value?.message).toMatch(/not supported/)
    workflow.select(image('image/jpeg', CLIENT_RENDER_FILE_LIMIT_BYTES + 1))
    expect(workflow.failure.value?.message).toMatch(/under about 1 MB/)
    await workflow.submit()
    expect(render).not.toHaveBeenCalled()
  })

  it('tracks processing and releases the completed preview on reset', async () => {
    let complete!: (value: {
      jpeg: Blob
      requestId: string | null
      canvasWidth: number
      canvasHeight: number
      expectedCount: number
      plannedCount: number
      executedCount: number
    }) => void
    const render = vi.fn(() => new Promise<{
      jpeg: Blob
      requestId: string | null
      canvasWidth: number
      canvasHeight: number
      expectedCount: number
      plannedCount: number
      executedCount: number
    }>((resolve) => { complete = resolve }))
    const create = vi.spyOn(URL, 'createObjectURL').mockReturnValue('blob:sheet')
    const revoke = vi.spyOn(URL, 'revokeObjectURL')
    const workflow = createSheetRenderWorkflow(render)
    workflow.select(image())
    const submission = workflow.submit()
    expect(workflow.status.value).toBe('uploading')
    await nextTick()
    expect(workflow.status.value).toBe('processing')
    complete({
      jpeg: new Blob(['jpeg'], { type: 'image/jpeg' }),
      requestId: 'sheet-request',
      canvasWidth: 2480,
      canvasHeight: 3508,
      expectedCount: 8,
      plannedCount: 8,
      executedCount: 8,
    })
    await submission
    expect(workflow.status.value).toBe('success')
    expect(workflow.result.value?.executedCount).toBe(8)
    expect(workflow.previewUrl.value).toBe('blob:sheet')
    workflow.reset()
    expect(revoke).toHaveBeenCalledWith('blob:sheet')
    create.mockRestore()
    revoke.mockRestore()
  })

  it('keeps the safe server capacity error available for the user', async () => {
    const workflow = createSheetRenderWorkflow(async () => {
      throw new ImgengineRenderError({
        kind: 'does_not_fit',
        status: 422,
        requestId: 'safe-sheet-request',
        message: 'The requested number of copies does not fit on this sheet.',
      })
    })
    workflow.select(image())
    await workflow.submit()
    expect(workflow.status.value).toBe('error')
    expect(workflow.failure.value).toMatchObject({ detail: { kind: 'does_not_fit', status: 422 } })
  })

  it('passes the server-planned millimetre contract without rewriting values', async () => {
    const render = vi.fn().mockResolvedValue({
      jpeg: new Blob(['jpeg'], { type: 'image/jpeg' }),
      requestId: 'sheet-request',
      canvasWidth: 2480,
      canvasHeight: 3508,
      expectedCount: 8,
      plannedCount: 8,
      executedCount: 8,
    })
    const workflow = createSheetRenderWorkflow(render)
    workflow.select(image())
    await workflow.submit()
    expect(render).toHaveBeenCalledWith(expect.any(File), input)
  })
})
