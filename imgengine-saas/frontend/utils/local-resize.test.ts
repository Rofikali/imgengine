import { describe, expect, it } from 'vitest'
import { linkedEdge, resizeDimensions } from './local-resize'
describe('local dimensions', () => {
  it.each(['', '0', '-1', '1.5', '1e3', 'Infinity', 'NaN', '4097', '99999999999', ' 20'])('rejects %s', value => expect(() => resizeDimensions(value, '1')).toThrow())
  it('bounds allocation independently of edges', () => {
    expect(() => resizeDimensions('4096', '4096')).toThrow()
    expect(resizeDimensions('2000', '2000')).toEqual({ width: 2000, height: 2000 })
    expect(resizeDimensions('1', '4096')).toEqual({ width: 1, height: 4096 })
  })
  it('rounds portrait, landscape, odd and one-pixel edges', () => {
    expect(linkedEdge('100', 401, 203)).toBe('51')
    expect(linkedEdge('100', 203, 401)).toBe('198')
    expect(linkedEdge('1', 8192, 1)).toBe('1')
    expect(linkedEdge('1', 2, 3)).toBe('2')
    expect(linkedEdge('', 2, 3)).toBeNull()
  })
})
