import type { LayoutRenderInput } from '~/types/api'

/** The Rust P5/P6 endpoint accepts exactly one multipart field: `file`. */
export function createRenderForm(file: File): FormData {
  const form = new FormData()
  form.append('file', file)
  return form
}

/** Builds only the documented layout multipart fields; browser state never carries a secret. */
export function createLayoutRenderForm(file: File, input: LayoutRenderInput): FormData {
  const form = new FormData()
  form.append('file', file)
  form.append('width', input.width)
  form.append('height', input.height)
  form.append('unit', input.unit)
  if (input.unit === 'mm' && input.dpi) form.append('dpi', input.dpi)
  form.append('copies', input.copies)
  if (input.columns) form.append('columns', input.columns)
  form.append('gap', input.gap)
  form.append('margin', input.margin)
  form.append('output_format', input.outputFormat)
  return form
}
