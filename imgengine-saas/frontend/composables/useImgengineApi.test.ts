import { afterEach, describe, expect, it, vi } from 'vitest'
import { useImgengineApi } from './useImgengineApi'

afterEach(() => vi.unstubAllGlobals())

describe('useImgengineApi', () => {
  it.each([
    ['image/jpeg', 'source.jpg'],
    ['image/png', 'source.png'],
  ])('returns the completed JPEG for a %s upload', async (type, name) => {
    const raw = vi.fn().mockResolvedValue({
      status: 200,
      _data: new Blob(['completed-jpeg'], { type: 'image/jpeg' }),
      headers: new Headers({ 'x-request-id': 'request-test-123' }),
    })
    vi.stubGlobal('$fetch', { raw })

    const result = await useImgengineApi().render(new File(['image'], name, { type }))

    expect(result.jpeg.type).toBe('image/jpeg')
    expect(result.requestId).toBe('request-test-123')
    const request = raw.mock.calls[0][1]
    expect(raw).toHaveBeenCalledWith('/api/render', expect.objectContaining({ method: 'POST' }))
    expect([...request.body.entries()].map(([field]: [string]) => field)).toEqual(['file'])
  })

  it('maps a blob problem response through the stable error contract', async () => {
    const problem = new Blob([JSON.stringify({
      type: 'https://imgengine.example/problems/overloaded',
      title: 'Service overloaded',
      status: 429,
      code: 'overloaded',
      request_id: 'request-test-123',
    })], { type: 'application/problem+json' })
    vi.stubGlobal('$fetch', {
      raw: vi.fn().mockRejectedValue({ response: { status: 429 }, data: problem }),
    })

    await expect(useImgengineApi().render(new File(['image'], 'source.jpg', { type: 'image/jpeg' })))
      .rejects.toMatchObject({ detail: { kind: 'overloaded', status: 429 } })
  })
})
