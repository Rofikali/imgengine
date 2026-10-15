import { describe, expect, it, vi } from 'vitest'
import { nextTick } from 'vue'
import { CLIENT_RENDER_FILE_LIMIT_BYTES, createRenderWorkflow } from './useRenderWorkflow'
import { ImgengineRenderError } from './useImgengineApi'

function image(type = 'image/jpeg', size = 5) {
  return new File([new Uint8Array(size)], 'source-image', { type })
}

describe('synchronous render workflow', () => {
  it.each(['image/jpeg', 'image/png'])('accepts a valid %s selection', (type) => {
    const workflow = createRenderWorkflow(vi.fn())
    workflow.select(image(type))
    expect(workflow.status.value).toBe('ready')
    expect(workflow.failure.value).toBeNull()
  })

  it('rejects missing, unsupported, and oversized files before a request', async () => {
    const render = vi.fn()
    const workflow = createRenderWorkflow(render)
    workflow.select(null)
    expect(workflow.failure.value?.message).toMatch(/Choose a JPEG or PNG/)
    workflow.select(image('image/gif'))
    expect(workflow.failure.value?.message).toMatch(/not supported/)
    workflow.select(image('image/jpeg', CLIENT_RENDER_FILE_LIMIT_BYTES + 1))
    expect(workflow.failure.value?.message).toMatch(/under about 1 MB/)
    await workflow.submit()
    expect(render).not.toHaveBeenCalled()
  })

  it('models upload, processing, success, and object-URL cleanup without fake progress', async () => {
    let complete!: (value: { jpeg: Blob, requestId: string | null }) => void
    const render = vi.fn(() => new Promise<{ jpeg: Blob, requestId: string | null }>((resolve) => { complete = resolve }))
    const create = vi.spyOn(URL, 'createObjectURL').mockReturnValue('blob:rendered')
    const revoke = vi.spyOn(URL, 'revokeObjectURL')
    const workflow = createRenderWorkflow(render)
    workflow.select(image())
    const submission = workflow.submit()
    expect(workflow.status.value).toBe('uploading')
    await nextTick()
    expect(workflow.status.value).toBe('processing')
    expect(workflow.isSubmitting.value).toBe(true)
    complete({ jpeg: new Blob(['jpeg'], { type: 'image/jpeg' }), requestId: 'safe-request-id' })
    await submission
    expect(workflow.status.value).toBe('success')
    expect(workflow.selectedFile.value).toBeNull()
    expect(workflow.previewUrl.value).toBe('blob:rendered')
    workflow.reset()
    expect(revoke).toHaveBeenCalledWith('blob:rendered')
    expect(workflow.status.value).toBe('idle')
    create.mockRestore()
    revoke.mockRestore()
  })

  it('preserves the typed, safe API error for retry presentation', async () => {
    const workflow = createRenderWorkflow(async () => {
      throw new ImgengineRenderError({ kind: 'overloaded', status: 429, requestId: 'safe-request-id', message: 'The rendering service is busy. Please try again shortly.' })
    })
    workflow.select(image())
    await workflow.submit()
    expect(workflow.status.value).toBe('error')
    expect(workflow.failure.value).toMatchObject({ detail: { kind: 'overloaded', status: 429 } })
  })
})
