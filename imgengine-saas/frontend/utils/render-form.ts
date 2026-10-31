import type { LayoutRenderInput, SheetRenderInput } from '~/types/api'

/** The Rust P5/P6 endpoint accepts exactly one multipart field: `file`. */
export function createRenderForm(file: File): FormData {
  const form = new FormData()
  form.append('file', file)
  return form
}

/** Builds the exact physical-sheet multipart contract; resource budgets stay server-owned. */
export function createSheetRenderForm(file: File, input: SheetRenderInput): FormData {
  const form = new FormData()
  form.append('file', file)
  form.append('sheet_width_mm', input.sheetWidthMm)
  form.append('sheet_height_mm', input.sheetHeightMm)
  form.append('photo_width_mm', input.photoWidthMm)
  form.append('photo_height_mm', input.photoHeightMm)
  if (input.dpi) form.append('dpi', input.dpi)
  form.append('copies', input.copies)
  form.append('margin_mm', input.marginMm)
  form.append('gap_x_mm', input.gapXmm)
  form.append('gap_y_mm', input.gapYmm)
  form.append('output_format', input.outputFormat)
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
