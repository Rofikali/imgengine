import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'

async function fixture(page: Page, transparent = false) {
  return Buffer.from(await page.evaluate(transparent => {
    const canvas = document.createElement('canvas'); canvas.width = 3; canvas.height = 2
    const context = canvas.getContext('2d')!
    context.fillStyle = '#e02020'; context.fillRect(0, 0, 2, 2)
    context.clearRect(2, 0, 1, 2)
    if (!transparent) { context.fillStyle = '#2040e0'; context.fillRect(2, 0, 1, 2) }
    return canvas.toDataURL('image/png').split(',')[1]!
  }, transparent), 'base64')
}

async function choose(page: Page, buffer: Buffer) {
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'private-fixture.png', mimeType: 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}

test('compresses locally to JPEG, composites alpha onto white, downloads, and passes accessibility', async ({ page }) => {
  const requests: string[] = []
  await page.goto('/compress-image')
  page.on('request', request => { if (/^https?:/.test(request.url()) && (request.method() !== 'GET' || request.postData())) requests.push(`${request.method()} ${request.url()}`) })
  await choose(page, await fixture(page, true))
  await page.getByLabel('Quality', { exact: true }).fill('80')
  await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  const output = page.getByAltText('Compressed JPEG preview'); await expect(output).toBeVisible()
  const pixels = await output.evaluate(async (img: HTMLImageElement) => {
    await img.decode(); const canvas = document.createElement('canvas'); canvas.width = img.naturalWidth; canvas.height = img.naturalHeight
    const context = canvas.getContext('2d')!; context.drawImage(img, 0, 0)
    return { type: await (await fetch(img.src)).blob().then(blob => blob.type), size: [canvas.width, canvas.height], transparentArea: Array.from(context.getImageData(2, 0, 1, 1).data) }
  })
  expect(pixels.type).toBe('image/jpeg'); expect(pixels.size).toEqual([3, 2])
  expect(pixels.transparentArea[0]).toBeGreaterThan(220); expect(pixels.transparentArea[1]).toBeGreaterThan(220); expect(pixels.transparentArea[2]).toBeGreaterThan(220)
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download JPEG' }).click()
  expect((await download).suggestedFilename()).toBe('imgengine-compressed.jpg')
  expect((await new AxeBuilder({ page }).include('.local-compress').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()).violations.map(v => v.id)).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  expect(requests).toEqual([])
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})

test('rejects invalid quality and cancels or removes local resources', async ({ page }) => {
  await page.addInitScript(() => {
    const urls = new Set<string>(), create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }
    URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    Object.defineProperty(window, 'compressURLs', { get: () => urls.size })
  })
  await page.goto('/compress-image'); await choose(page, await fixture(page))
  await page.getByLabel('Quality', { exact: true }).fill('39'); await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  await expect(page.getByRole('alert')).toContainText('40 to 95')
  await page.evaluate(() => { window.Worker = class { postMessage() {} terminate() {} } as unknown as typeof Worker })
  await page.getByLabel('Quality', { exact: true }).fill('80'); await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  await page.getByRole('button', { name: 'Cancel', exact: true }).click()
  await expect(page.getByAltText('Compressed JPEG preview')).toHaveCount(0)
  await page.getByRole('button', { name: 'Remove image' }).click()
  await expect.poll(() => page.evaluate(() => (window as unknown as { compressURLs: number }).compressURLs)).toBe(0)
  await expect(page.getByLabel('Choose a JPEG or PNG', { exact: true })).toBeFocused()
})
