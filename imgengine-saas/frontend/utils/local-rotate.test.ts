import { expect, it } from 'vitest'
import { rotatedSize, validateRightAngleRotation } from './local-rotate'

it.each([0, 90, 180, 270] as const)('accepts %i-degree right-angle rotations', angle => {
  expect(validateRightAngleRotation(angle)).toBe(angle)
})

it.each([-90, 45, 360, 1.5, Number.NaN, Number.POSITIVE_INFINITY])('rejects non-right-angle rotation %s', angle => {
  expect(() => validateRightAngleRotation(angle)).toThrow('right-angle')
})

it('swaps only quarter-turn dimensions', () => {
  expect(rotatedSize({ width: 401, height: 203 }, 0)).toEqual({ width: 401, height: 203 })
  expect(rotatedSize({ width: 401, height: 203 }, 90)).toEqual({ width: 203, height: 401 })
  expect(rotatedSize({ width: 401, height: 203 }, 180)).toEqual({ width: 401, height: 203 })
  expect(rotatedSize({ width: 401, height: 203 }, 270)).toEqual({ width: 203, height: 401 })
})
