export const LOCAL_CONVERT_FORMATS = ['image/jpeg', 'image/png'] as const
export type LocalConvertFormat = typeof LOCAL_CONVERT_FORMATS[number]
export const MAX_CONVERTED_BYTES = 20 * 1024 * 1024

export function convertFormat(value: string): LocalConvertFormat {
  if ((LOCAL_CONVERT_FORMATS as readonly string[]).includes(value)) return value as LocalConvertFormat
  throw Error('Choose JPEG or PNG as the output format.')
}

export function convertedExtension(format: LocalConvertFormat): 'jpg' | 'png' {
  return format === 'image/jpeg' ? 'jpg' : 'png'
}

export function validConvertedImage(value: unknown, format: LocalConvertFormat): value is Blob {
  return value instanceof Blob && value.type === format && value.size > 0 && value.size <= MAX_CONVERTED_BYTES
}
