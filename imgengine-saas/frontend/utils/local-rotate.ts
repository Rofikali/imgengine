import type { RasterSize } from './local-crop'

export const RIGHT_ANGLE_ROTATIONS = [0, 90, 180, 270] as const
export type RightAngleRotation = typeof RIGHT_ANGLE_ROTATIONS[number]

export function validateRightAngleRotation(value: number): RightAngleRotation {
  if (!Number.isSafeInteger(value) || !RIGHT_ANGLE_ROTATIONS.includes(value as RightAngleRotation)) {
    throw Error('Choose a right-angle rotation.')
  }
  return value as RightAngleRotation
}

export function rotatedSize(source: RasterSize, rotation: number): RasterSize {
  const angle = validateRightAngleRotation(rotation)
  return angle === 90 || angle === 270
    ? { width: source.height, height: source.width }
    : { width: source.width, height: source.height }
}
