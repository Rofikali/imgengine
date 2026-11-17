import { expect, it } from 'vitest'
import { initialCrop, moveCrop, parsePixelCrop, validatePixelCrop } from './local-crop'
it.each(['', '-1', '1.5', '1e2', 'NaN', 'Infinity', '999999999999', ' 0'])('rejects coordinate %s', x => {
  expect(() => parsePixelCrop({ x, y: '0', width: '1', height: '1' }, { width: 401, height: 203 })).toThrow()
})
it('rejects empty, overflowing and oversized rectangles', () => {
  const source = { width: 8192, height: 8192 }
  for (const c of [{ x: -1, y: 0, width: 1, height: 1 }, { x: 0, y: 0, width: 0, height: 1 }, { x: 8192, y: 0, width: 1, height: 1 }, { x: 0, y: 0, width: 4097, height: 1 }, { x: 0, y: 0, width: 2001, height: 2000 }, { x: 0.5, y: 0, width: 1, height: 1 }]) expect(() => validatePixelCrop(c, source)).toThrow()
})
it('accepts final pixel and full odd raster without normalized rounding', () => {
  const source = { width: 401, height: 203 }
  expect(parsePixelCrop({ x: '400', y: '202', width: '1', height: '1' }, source)).toEqual({ x: 400, y: 202, width: 1, height: 1 })
  expect(initialCrop(source)).toEqual({ x: 0, y: 0, ...source })
  expect(initialCrop(source, true)).toEqual({ x: 99, y: 0, width: 203, height: 203 })
})
it('drag rounds once in source pixels and clamps both edges', () => {
  const source = { width: 203, height: 401 }, crop = { x: 10, y: 20, width: 50, height: 60 }
  expect(moveCrop(crop, source, 0.49, 0.5)).toEqual({ ...crop, y: 21 })
  expect(moveCrop(crop, source, -9999, -9999)).toEqual({ ...crop, x: 0, y: 0 })
  expect(moveCrop(crop, source, 9999, 9999)).toEqual({ ...crop, x: 153, y: 341 })
})
it('bounded reset supports thin 8192-pixel sources', () => {
  expect(initialCrop({ width: 8192, height: 1 })).toEqual({ x: 2048, y: 0, width: 4096, height: 1 })
  for (const source of [{ width: 1, height: 8192 }, { width: 2000, height: 2000 }, { width: 203, height: 401 }]) {
    expect(() => validatePixelCrop(initialCrop(source), source)).not.toThrow()
    expect(() => validatePixelCrop(initialCrop(source, true), source)).not.toThrow()
  }
})
