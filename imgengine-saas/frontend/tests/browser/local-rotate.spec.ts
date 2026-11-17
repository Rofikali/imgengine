import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'
import { oriented } from './orientation'

const pixels = [
  [255, 0, 0, 255], [0, 255, 0, 255], [0, 0, 255, 255],
  [255, 255, 0, 255], [255, 0, 255, 255], [0, 255, 255, 0],
]

async function fixture(page: Page, jpeg = false) {
  return Buffer.from(await page.evaluate(({ pixels, jpeg }) => {
    const c = document.createElement('canvas'); c.width = 3; c.height = 2
    const image = c.getContext('2d')!.createImageData(3, 2)
    pixels.forEach((pixel, index) => image.data.set(pixel, index * 4))
    c.getContext('2d')!.putImageData(image, 0, 0)
    return c.toDataURL(jpeg ? 'image/jpeg' : 'image/png').split(',')[1]!
  }, { pixels, jpeg }), 'base64')
}

async function choose(page: Page, buffer: Buffer, jpeg = false) {
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'private-fixture', mimeType: jpeg ? 'image/jpeg' : 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}

async function rotate(page: Page, angle: 0 | 90 | 180 | 270) {
  await page.getByRole('button', { name: angle === 0 ? 'Keep orientation' : `Rotate ${angle}° clockwise`, exact: true }).click()
  await page.getByRole('button', { name: 'Rotate locally', exact: true }).click()
  const result = page.getByAltText('Rotated PNG preview'); await expect(result).toBeVisible()
  return result.evaluate(async (img: HTMLImageElement) => {
    await img.decode(); const c = document.createElement('canvas'); c.width = img.naturalWidth; c.height = img.naturalHeight
    const context = c.getContext('2d')!; context.drawImage(img, 0, 0)
    return { width: c.width, height: c.height, pixels: Array.from(context.getImageData(0, 0, c.width, c.height).data) }
  })
}

function expected(angle: number) {
  const result: number[] = [], width = angle === 90 || angle === 270 ? 2 : 3, height = angle === 90 || angle === 270 ? 3 : 2
  for (let y = 0; y < height; y++) for (let x = 0; x < width; x++) {
    const [sourceX, sourceY] = angle === 0 ? [x, y] : angle === 90 ? [y, 1 - x] : angle === 180 ? [2 - x, 1 - y] : [2 - y, x]
    const pixel = pixels[sourceY * 3 + sourceX]!
    // Canvas readback preserves alpha but normalizes invisible RGB to zero.
    result.push(...(pixel[3] === 0 ? [0, 0, 0, 0] : pixel))
  }
  return { width, height, pixels: result }
}

test('right angles preserve asymmetric pixels and alpha, stay private, download, and pass axe', async ({ page }) => {
  const requests: string[] = []
  await page.goto('/rotate-image')
  page.on('request', request => { if (/^https?:/.test(request.url()) && (request.method() !== 'GET' || request.postData())) requests.push(`${request.method()} ${request.url()}`) })
  await choose(page, await fixture(page))
  for (const angle of [0, 90, 180, 270] as const) {
    expect(await rotate(page, angle)).toEqual(expected(angle))
    await expect(page.getByRole('button', { name: angle === 0 ? 'Keep orientation' : `Rotate ${angle}° clockwise`, exact: true })).toHaveAttribute('aria-pressed', 'true')
  }
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download PNG' }).click()
  expect((await download).suggestedFilename()).toBe('imgengine-rotated.png')
  expect((await new AxeBuilder({ page }).include('.local-rotate').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()).violations.map(v => v.id)).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  expect(requests).toEqual([])
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})

test('stored raster orientation is not auto-rotated, and malformed metadata is rejected', async ({ page }) => {
  await page.goto('/rotate-image')
  for (const jpeg of [false, true]) for (const orientation of [2, 6, 8]) {
    await choose(page, oriented(await fixture(page, jpeg), orientation, !jpeg, true), jpeg)
    await expect(page.getByText('Source: 3 × 2 pixels.', { exact: false })).toBeVisible()
    const output = await rotate(page, 90)
    expect([output.width, output.height]).toEqual([2, 3])
  }
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'bad-orientation', mimeType: 'image/png', buffer: oriented(await fixture(page), 9, true, true) })
  await expect(page.getByRole('alert')).toContainText('Cannot open')
})

test('output bounds, cancellation and URL cleanup do not leave local image state', async ({ page }) => {
  await page.addInitScript(() => {
    const urls = new Set<string>(), create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }; URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    Object.defineProperty(window, 'rotateURLs', { get: () => urls.size })
  })
  await page.goto('/rotate-image'); const buffer = await fixture(page)
  const count = () => page.evaluate(() => (window as unknown as { rotateURLs: number }).rotateURLs)
  for (let i = 0; i < 3; i++) { await choose(page, buffer); await rotate(page, 90); await expect.poll(count).toBe(2) }
  await page.getByRole('button', { name: 'Rotate 180° clockwise', exact: true }).click(); await expect.poll(count).toBe(1)
  await page.evaluate(() => { window.Worker = class { postMessage() {} terminate() {} } as unknown as typeof Worker })
  await page.getByRole('button', { name: 'Rotate locally', exact: true }).click(); await page.getByRole('button', { name: 'Cancel', exact: true }).click()
  await expect(page.getByAltText('Rotated PNG preview')).toHaveCount(0)
  await page.getByRole('button', { name: 'Remove image' }).click(); await expect.poll(count).toBe(0)
  await expect(page.getByLabel('Choose a JPEG or PNG', { exact: true })).toBeFocused()
  const overEdge = Buffer.from(await page.evaluate(() => {
    const c = document.createElement('canvas'); c.width = 4097; c.height = 1
    return c.toDataURL().split(',')[1]!
  }), 'base64')
  await choose(page, overEdge); await page.getByRole('button', { name: 'Rotate 90° clockwise', exact: true }).click()
  await page.getByRole('button', { name: 'Rotate locally', exact: true }).click()
  await expect(page.getByRole('alert')).toContainText('4096 pixels')
})
