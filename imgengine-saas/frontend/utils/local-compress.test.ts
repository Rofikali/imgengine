import { describe, expect, it } from 'vitest'
import { DEFAULT_JPEG_QUALITY, jpegQuality } from './local-compress'

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
})
