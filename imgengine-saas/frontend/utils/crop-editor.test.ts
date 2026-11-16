import { describe, expect, it } from 'vitest'
import { cropPixels, defaultCropPosition, editorCrop, photoRaster } from './crop-editor'

describe('crop preview geometry', () => {
  it('uses the raster aspect at the selected DPI, including half-up conversion', () => {
    expect(photoRaster('35', '45', '300')).toEqual({ width: 413, height: 531 })
    expect(photoRaster('12.7', '25.4', '1')).toEqual({ width: 1, height: 1 })
    for (const value of ['', 'NaN', 'Infinity', '-1', '1e2', '0.0001']) expect(photoRaster(value, '45')).toBeNull()
    expect(photoRaster('35', '45', '0')).toBeNull()
  })

  it('matches centered and edge reference vectors without losing a pixel on normalization', () => {
    const source = { width: 400, height: 200 }, target = { width: 1, height: 1 }
    expect(editorCrop(source, target, defaultCropPosition())).toEqual({ x: 250000, y: 0, width: 500000, height: 1000000 })
    expect(editorCrop(source, target, { x: 100, y: 0, zoom: 2 })).toEqual({ x: 750000, y: 0, width: 250000, height: 500000 })
  })

  it('keeps odd sizes, corners, extreme shapes and zoom within the native rounding envelope', () => {
    for (const [width, height] of [[401, 203], [203, 401], [1, 1], [8192, 10], [3999, 4001]]) {
      const source = { width: width!, height: height! }
      for (const target of [{ width: 413, height: 531 }, { width: 1, height: 1 }, { width: 300, height: 17 }]) {
        for (const zoom of [1, 1.05, 2.3, 4]) for (const x of [0, 50, 100]) for (const y of [0, 50, 100]) {
          const crop = editorCrop(source, target, { x, y, zoom })
          if (!crop) continue // No nonempty source rectangle exists for some one-pixel/extreme shapes.
          expect(Object.values(crop).every(Number.isSafeInteger)).toBe(true)
          expect(crop.width).toBeGreaterThan(0)
          expect(crop.height).toBeGreaterThan(0)
          expect(crop.x + crop.width).toBeLessThanOrEqual(1000000)
          expect(crop.y + crop.height).toBeLessThanOrEqual(1000000)
          const pixels = cropPixels(source, crop)
          expect(pixels.x + pixels.width).toBeLessThanOrEqual(source.width)
          expect(pixels.y + pixels.height).toBeLessThanOrEqual(source.height)
          expect(Math.abs(pixels.width * target.height - pixels.height * target.width)).toBeLessThanOrEqual(target.width + target.height)
        }
      }
    }
  })

  it('rejects nonfinite controls and oversized previews and bounds finite controls', () => {
    const source = { width: 400, height: 200 }, target = { width: 1, height: 1 }
    expect(editorCrop(source, target, { x: NaN, y: 50, zoom: 1 })).toBeNull()
    expect(editorCrop({ width: 100000, height: 100000 }, target, defaultCropPosition())).toBeNull()
    expect(editorCrop(source, target, { x: -1, y: 101, zoom: 0 })).toEqual(editorCrop(source, target, { x: 0, y: 100, zoom: 1 }))
  })
})
