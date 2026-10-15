/** The Rust P5/P6 endpoint accepts exactly one multipart field: `file`. */
export function createRenderForm(file: File): FormData {
  const form = new FormData()
  form.append('file', file)
  return form
}
