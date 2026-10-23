import { describe, expect, it } from 'vitest'
import { createLayoutRenderForm, createRenderForm } from './render-form'

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
