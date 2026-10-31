import { describe, expect, it } from 'vitest'
import { createLayoutRenderForm, createRenderForm, createSheetRenderForm } from './render-form'

describe('P5 render multipart request', () => {
  it.each([
    ['image/jpeg', 'source.jpg'],
    ['image/png', 'source.png'],
  ])('sends a %s image as the single file field', (type, name) => {
    const file = new File(['fixture'], name, { type })
    const form = createRenderForm(file)
    const entries = [...form.entries()]

    expect(entries).toHaveLength(1)
    expect(entries[0][0]).toBe('file')
    expect(entries[0][1]).toBe(file)
  })
})

describe('P8 layout multipart request', () => {
  it('sends the documented mm fields and no client-side output capability', () => {
    const form = createLayoutRenderForm(new File(['fixture'], 'source.png', { type: 'image/png' }), {
      width: '25.4', height: '12.7', unit: 'mm', dpi: '100', copies: '1', gap: '0', margin: '0', outputFormat: 'jpeg',
    })
    expect([...form.entries()].map(([field]) => field)).toEqual([
      'file', 'width', 'height', 'unit', 'dpi', 'copies', 'gap', 'margin', 'output_format',
    ])
  })
})

describe('P12 physical-sheet multipart request', () => {
  it('sends only the documented physical dimensions and server-owned settings', () => {
    const form = createSheetRenderForm(new File(['fixture'], 'source.jpg', { type: 'image/jpeg' }), {
      sheetWidthMm: '210', sheetHeightMm: '297', photoWidthMm: '35', photoHeightMm: '45',
      dpi: '300', copies: '8', marginMm: '10', gapXmm: '2', gapYmm: '2', outputFormat: 'jpeg',
    })
    expect([...form.entries()].map(([field]) => field)).toEqual([
      'file', 'sheet_width_mm', 'sheet_height_mm', 'photo_width_mm', 'photo_height_mm',
      'dpi', 'copies', 'margin_mm', 'gap_x_mm', 'gap_y_mm', 'output_format',
    ])
  })
})
