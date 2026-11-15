import { describe, expect, it } from 'vitest'
import { hasValidOptionalSheetCrop } from './sheet-crop'

function fields(entries: Array<[string, FormDataEntryValue]>): Map<string, FormDataEntryValue> {
  return new Map(entries)
}

describe('ABI 2.3 sheet crop proxy contract', () => {
  it('accepts omitted or complete in-bounds integer rectangles', () => {
    expect(hasValidOptionalSheetCrop(fields([]))).toBe(true)
    expect(hasValidOptionalSheetCrop(fields([
      ['crop_x', '0'], ['crop_y', '0'], ['crop_width', '1000000'], ['crop_height', '1000000'],
    ]))).toBe(true)
  })

  it('rejects partial, malformed, negative, overflowed, and out-of-range rectangles', () => {
    expect(hasValidOptionalSheetCrop(fields([['crop_x', '0']]))).toBe(false)
    expect(hasValidOptionalSheetCrop(fields([
      ['crop_x', '-1'], ['crop_y', '0'], ['crop_width', '1'], ['crop_height', '1'],
    ]))).toBe(false)
    expect(hasValidOptionalSheetCrop(fields([
      ['crop_x', '0'], ['crop_y', '0'], ['crop_width', '4294967296'], ['crop_height', '1'],
    ]))).toBe(false)
    expect(hasValidOptionalSheetCrop(fields([
      ['crop_x', '500000'], ['crop_y', '0'], ['crop_width', '500001'], ['crop_height', '1'],
    ]))).toBe(false)
  })
})
