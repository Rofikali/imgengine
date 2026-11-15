/** ABI 2.3 normalized-crop multipart field names accepted by the sheet proxy. */
export const SHEET_CROP_FIELDS = ['crop_x', 'crop_y', 'crop_width', 'crop_height'] as const

const NORMALIZED_CROP_SCALE = 1_000_000

function parseNormalizedCoordinate(value: FormDataEntryValue | undefined): number | undefined {
  if (typeof value !== 'string' || !/^\d+$/.test(value)) return undefined
  const parsed = Number(value)
  if (!Number.isSafeInteger(parsed) || parsed > NORMALIZED_CROP_SCALE) return undefined
  return parsed
}

/**
 * Validates the all-or-none, fixed-point crop transport contract before the
 * Nuxt proxy forwards it. Rust and native C repeat validation; this never
 * substitutes for the server-side trust boundary.
 */
export function hasValidOptionalSheetCrop(fields: ReadonlyMap<string, FormDataEntryValue>): boolean {
  const supplied = SHEET_CROP_FIELDS.map(name => fields.has(name))
  if (supplied.every(present => !present)) return true
  if (supplied.some(present => !present)) return false

  const [x, y, width, height] = SHEET_CROP_FIELDS.map(name => parseNormalizedCoordinate(fields.get(name)))
  return x !== undefined
    && y !== undefined
    && width !== undefined
    && height !== undefined
    && width > 0
    && height > 0
    && x + width <= NORMALIZED_CROP_SCALE
    && y + height <= NORMALIZED_CROP_SCALE
}
