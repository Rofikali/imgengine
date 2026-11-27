import { expect, test } from '@playwright/test'
import { expectedLocalToolOrigin } from './static-payload'
import AxeBuilder from '@axe-core/playwright'

async function fixture(page: import('@playwright/test').Page) {
  return Buffer.from(await page.evaluate(() => {
    const canvas = document.createElement('canvas'); canvas.width = 96; canvas.height = 48
    const context = canvas.getContext('2d')!; context.fillStyle = '#df2020'; context.fillRect(0, 0, 48, 48); context.clearRect(48, 0, 48, 48)
    return canvas.toDataURL('image/png').split(',')[1]!
  }), 'base64')
}

async function choose(page: import('@playwright/test').Page, buffer: Buffer) {
  const picker = page.getByLabel('Choose a JPEG or PNG', { exact: true })
  await expect(picker).toBeEnabled()
  await picker.setInputFiles({ name: 'private-source.png', mimeType: 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}

test('converts locally to PNG and JPEG, preserves the documented alpha behavior, downloads, and has no image upload', async ({ page }) => {
  const requests: Array<{ method: string, url: string, body: string | null }> = []
  await page.goto('/convert-image', { waitUntil: 'domcontentloaded' })
  page.on('request', request => { if (/^https?:/.test(request.url())) requests.push({ method: request.method(), url: request.url(), body: request.postData() }) })
  await choose(page, await fixture(page))
  await expect(page.getByRole('combobox', { name: 'Format', exact: true })).toBeVisible()
  await page.getByRole('button', { name: 'Convert locally', exact: true }).click()
  const png = page.getByAltText('Converted PNG preview'); await expect(png).toBeVisible()
  expect(await png.evaluate(async (image: HTMLImageElement) => (await (await fetch(image.src)).blob()).type)).toBe('image/png')
  const transparent = await png.evaluate(async (image: HTMLImageElement) => { const c = document.createElement('canvas'); c.width = image.naturalWidth; c.height = image.naturalHeight; c.getContext('2d')!.drawImage(image, 0, 0); return Array.from(c.getContext('2d')!.getImageData(72, 24, 1, 1).data) })
  expect(transparent).toEqual([0, 0, 0, 0])
  await page.getByLabel('Format', { exact: true }).selectOption('image/jpeg')
  await page.getByRole('button', { name: 'Convert locally', exact: true }).click()
  const jpeg = page.getByAltText('Converted JPEG preview'); await expect(jpeg).toBeVisible()
  const detail = await jpeg.evaluate(async (image: HTMLImageElement) => { const blob = await (await fetch(image.src)).blob(); const c = document.createElement('canvas'); c.width = image.naturalWidth; c.height = image.naturalHeight; c.getContext('2d')!.drawImage(image, 0, 0); return { type: blob.type, pixels: Array.from(c.getContext('2d')!.getImageData(72, 24, 1, 1).data), exif: new TextDecoder().decode(await blob.arrayBuffer()).includes('Exif') } })
  expect(detail.type).toBe('image/jpeg'); expect(detail.pixels[0]).toBeGreaterThan(220); expect(detail.pixels[1]).toBeGreaterThan(220); expect(detail.pixels[2]).toBeGreaterThan(220); expect(detail.exif).toBe(false)
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download JPEG' }).click(); expect((await download).suggestedFilename()).toBe('imgengine-converted.jpg')
  expect((await new AxeBuilder({ page }).include('.local-convert').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()).violations.map(v => v.id)).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  expect(requests.every(request => request.method === 'GET' && !request.body && new URL(request.url).origin === expectedLocalToolOrigin()), JSON.stringify(requests)).toBe(true)
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})

test('rejects malformed or oversized input and releases local URLs after remove', async ({ page }) => {
  await page.addInitScript(() => {
    const urls = new Set<string>(), create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }
    URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    Object.defineProperty(window, 'convertURLs', { get: () => urls.size })
  })
  await page.goto('/convert-image', { waitUntil: 'domcontentloaded' })
  const picker = page.getByLabel('Choose a JPEG or PNG', { exact: true })
  await expect(picker).toBeEnabled()
  await picker.setInputFiles({ name: 'bad', mimeType: 'image/png', buffer: Buffer.from('not an image') })
  await expect(page.getByRole('alert')).toContainText('Cannot open')
  await choose(page, await fixture(page)); await page.getByRole('button', { name: 'Convert locally', exact: true }).click(); await expect(page.getByAltText('Converted PNG preview')).toBeVisible()
  await page.getByRole('button', { name: 'Remove image', exact: true }).click()
  await expect.poll(() => page.evaluate(() => (window as unknown as { convertURLs: number }).convertURLs)).toBe(0)
  await expect(page.getByLabel('Choose a JPEG or PNG', { exact: true })).toBeFocused()
})
