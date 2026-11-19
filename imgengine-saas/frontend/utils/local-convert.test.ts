import { describe, expect, it } from 'vitest'
import { convertFormat, convertedExtension, validConvertedImage } from './local-convert'

describe('local image conversion format contract', () => {
  it.each(['image/jpeg', 'image/png'] as const)('accepts supported output %s', format => {
    expect(convertFormat(format)).toBe(format)
    expect(convertedExtension(format)).toBe(format === 'image/jpeg' ? 'jpg' : 'png')
    expect(validConvertedImage(new Blob(['image'], { type: format }), format)).toBe(true)
  })
  it.each(['image/webp', 'image/gif', '', 'JPEG'])('rejects unsupported output %s', value => {
    expect(() => convertFormat(value)).toThrow('JPEG or PNG')
  })
  it('requires the requested MIME type and a bounded nonempty Blob', () => {
    expect(validConvertedImage(new Blob(['image'], { type: 'image/png' }), 'image/jpeg')).toBe(false)
    expect(validConvertedImage(new Blob([], { type: 'image/jpeg' }), 'image/jpeg')).toBe(false)
    expect(validConvertedImage('not a Blob', 'image/jpeg')).toBe(false)
  })
})
