export const MAX_LOCAL_BYTES = 4 * 1024 * 1024
export const MAX_LOCAL_PIXELS = 4_000_000
export const MAX_OUTPUT_EDGE = 4096

export function resizeDimensions(width: string, height: string) {
  if (![width, height].every(v => /^\d{1,4}$/.test(v))) throw Error('Enter whole-number pixel dimensions.')
  const w = Number(width), h = Number(height)
  if (w < 1 || h < 1 || w > MAX_OUTPUT_EDGE || h > MAX_OUTPUT_EDGE || w * h > MAX_LOCAL_PIXELS) {
    throw Error('Use 1–4096 pixels per side and at most 4 megapixels.')
  }
  return { width: w, height: h }
}

// Nearest integer, halves rounded up, never a zero-sized short edge.
export function linkedEdge(value: string, from: number, to: number): string | null {
  if (!/^\d{1,4}$/.test(value) || Number(value) < 1 || Number(value) > MAX_OUTPUT_EDGE) return null
  return String(Math.max(1, Math.floor(Number(value) * to / from + 0.5)))
}
