import { describe, expect, it } from 'vitest'
import { createRenderForm } from './render-form'

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
