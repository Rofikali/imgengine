import { afterEach, describe, expect, it, vi } from 'vitest'
import { inspectPreview, loadCropPreview } from './crop-preview'

function png(width = 400, height = 200, extra = new Uint8Array()) {
  const bytes = new Uint8Array(33 + extra.length)
  const view = new DataView(bytes.buffer)
  view.setUint32(0, 0x89504e47); view.setUint32(4, 0x0d0a1a0a)
  view.setUint32(8, 13); view.setUint32(12, 0x49484452)
  view.setUint32(16, width); view.setUint32(20, height)
  bytes.set(extra, 33)
  return bytes
}

function exif(orientation: number, little: boolean) {
  const bytes = new Uint8Array(26), view = new DataView(bytes.buffer)
  view.setUint16(0, little ? 0x4949 : 0x4d4d)
  view.setUint16(2, 42, little); view.setUint32(4, 8, little); view.setUint16(8, 1, little)
  view.setUint16(10, 0x112, little); view.setUint16(12, 3, little)
  view.setUint32(14, 1, little); view.setUint16(18, orientation, little)
  return bytes
}

afterEach(() => { vi.restoreAllMocks(); vi.unstubAllGlobals(); vi.useRealTimers() })
describe('bounded crop preview header guard', () => {
  it('reads dimensions but rejects oversized, truncated and unsupported images before decoding', () => {
    expect(inspectPreview(png())).toEqual({ width: 400, height: 200 })
    expect(() => inspectPreview(png(8000, 8000))).toThrow('16 megapixels')
    expect(() => inspectPreview(png(9000, 1))).toThrow('8192')
    expect(() => inspectPreview(png(0, 1))).toThrow()
    expect(() => inspectPreview(new Uint8Array([0xff, 0xd8, 0xff, 0xe1, 0xff, 0xff]))).toThrow()
    expect(() => inspectPreview(new Uint8Array([1, 2, 3]))).toThrow()
  })
  it.each([true, false])('guards JPEG and PNG orientation metadata (little endian %s)', (little) => {
    for (const orientation of [1, 2, 3, 4, 5, 6, 7, 8]) {
      const metadata = exif(orientation, little)
      const jpeg = new Uint8Array([0xff, 0xd8, 0xff, 0xe1, 0, 34, 69, 120, 105, 102, 0, 0,
        ...metadata, 0xff, 0xc0, 0, 8, 8, 0, 200, 1, 144, 0, 0xff, 0xda])
      const chunk = new Uint8Array(12 + metadata.length)
      const view = new DataView(chunk.buffer)
      view.setUint32(0, metadata.length); view.setUint32(4, 0x65584966); chunk.set(metadata, 8)
      for (const bytes of [jpeg, png(400, 200, chunk)]) {
        if (orientation === 1) expect(inspectPreview(bytes)).toEqual({ width: 400, height: 200 })
        else expect(() => inspectPreview(bytes)).toThrow('orientation metadata')
      }
    }
  })

  it('releases the URL once after decode, cancellation, failure or dimension mismatch', async () => {
    let current: FakeImage
    class FakeImage {
      naturalWidth = 400; naturalHeight = 200; src = ''
      onload: (() => void) | null = null; onerror: (() => void) | null = null
      constructor() { current = this }
      removeAttribute() { this.src = '' }
    }
    vi.stubGlobal('Image', FakeImage)
    vi.spyOn(URL, 'createObjectURL').mockReturnValue('blob:guard')
    const revoke = vi.spyOn(URL, 'revokeObjectURL').mockImplementation(() => {})
    for (const outcome of ['success', 'abort', 'error', 'mismatch', 'timeout']) {
      revoke.mockClear()
      vi.useFakeTimers()
      const controller = new AbortController()
      const pending = loadCropPreview(new File([png()], 'fixture.png'), controller.signal)
      const settled = pending.catch(error => error as Error)
      await vi.waitFor(() => expect(current!.src).toBe('blob:guard'))
      if (outcome === 'abort') controller.abort()
      else if (outcome === 'timeout') await vi.advanceTimersByTimeAsync(10_000)
      else if (outcome === 'error') current!.onerror?.()
      else { if (outcome === 'mismatch') current!.naturalWidth = 200; current!.onload?.() }
      if (outcome === 'success') { const preview = await pending; preview.dispose(); preview.dispose() }
      else expect(await settled).toBeInstanceOf(Error)
      expect(revoke).toHaveBeenCalledExactlyOnceWith('blob:guard')
    }
  })
})
