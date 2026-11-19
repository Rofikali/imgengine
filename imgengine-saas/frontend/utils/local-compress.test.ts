import { describe, expect, it } from 'vitest'
import { DEFAULT_JPEG_QUALITY, isLocalCompressBitmap, isLocalCompressSuccess, jpegQuality, validCompressedJpeg } from './local-compress'

describe('local JPEG compression quality', () => {
  it('uses a conservative default within the documented range', () => {
    expect(DEFAULT_JPEG_QUALITY).toBe(80)
    expect(jpegQuality(String(DEFAULT_JPEG_QUALITY))).toBe(80)
  })

  it.each(['', '0', '39', '96', '1.5', '1e2', ' 80', '080', 'NaN'])('rejects invalid quality %s', value => {
    expect(() => jpegQuality(value)).toThrow()
  })

  it.each(['40', '95'])('accepts inclusive quality boundary %s', value => {
    expect(jpegQuality(value)).toBe(Number(value))
  })

  it('accepts only a typed Worker success response', () => {
    const jpeg = new Blob([new Uint8Array([0xff, 0xd8, 0xff, 0xd9])], { type: 'image/jpeg' })
    expect(isLocalCompressSuccess({ blob: jpeg, quality: 80 })).toBe(true)
    expect(isLocalCompressSuccess({ blob: new Blob(), quality: 80 })).toBe(false)
    expect(isLocalCompressSuccess({ blob: new Blob(['plain'], { type: 'text/plain' }), quality: 80 })).toBe(false)
    expect(isLocalCompressSuccess({ blob: 'not-a-blob', quality: 80 })).toBe(false)
    expect(isLocalCompressSuccess({ blob: jpeg, quality: 39 })).toBe(false)
    expect(isLocalCompressSuccess({ blob: jpeg, quality: 96 })).toBe(false)
    expect(isLocalCompressSuccess({ error: true })).toBe(false)
    expect(validCompressedJpeg(jpeg)).toBe(true)
    expect(validCompressedJpeg(new Blob())).toBe(false)
  })

  it('does not treat a Worker failure as a transferable bitmap response', () => {
    expect(isLocalCompressBitmap({ error: true })).toBe(false)
  })
})
