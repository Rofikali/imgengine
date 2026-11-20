import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'
import { oriented } from './orientation'

async function source(page: Page, width = 401, height = 203, jpeg = false) {
  return Buffer.from(await page.evaluate(({ width, height, jpeg }) => {
    const c = document.createElement('canvas'); c.width = width; c.height = height
    const ctx = c.getContext('2d')!, data = ctx.createImageData(width, height)
    for (let y = 0; y < height; y++) for (let x = 0; x < width; x++) {
      const i = (y * width + x) * 4
      data.data[i] = x % 256; data.data[i + 1] = y % 256; data.data[i + 2] = 160; data.data[i + 3] = x === 400 && y === 202 ? 0 : 255
    }
    ctx.putImageData(data, 0, 0)
    return c.toDataURL(jpeg ? 'image/jpeg' : 'image/png').split(',')[1]!
  }, { width, height, jpeg }), 'base64')
}
async function choose(page: Page, buffer: Buffer) {
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'private-fixture', mimeType: 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}
async function fields(page: Page, x: number, y: number, width: number, height: number) {
  for (const [name, value] of Object.entries({ Left: x, Top: y, Width: width, Height: height })) await page.getByLabel(name, { exact: true }).fill(String(value))
}
async function crop(page: Page) { await page.getByRole('button', { name: 'Crop locally', exact: true }).click(); await expect(page.getByAltText('Cropped PNG preview')).toBeVisible() }

test('exact odd rectangle pixels, transparent final pixel, keyboard, privacy and axe', async ({ page }) => {
  await page.goto('/crop-image')
  const requests: string[] = []
  page.on('request', r => {
    if (!/^https?:/.test(r.url())) return
    const u = new URL(r.url())
    const asset = /^\/_nuxt\/[^/]+\.(js|css)$/.test(u.pathname) || /^\/_nuxt\/builds\/meta\/[a-f0-9-]+\.json$/.test(u.pathname)
    if (r.method() !== 'GET' || r.postData() || u.search || u.origin !== 'http://127.0.0.1:3100' || !asset) requests.push(`${r.method()} ${u.pathname}`)
  })
  await choose(page, await source(page)); await fields(page, 10, 20, 17, 31)
  const frame = page.getByRole('group', { name: 'Crop selection preview' }); await frame.focus(); await page.keyboard.press('ArrowRight'); await page.keyboard.press('Shift+ArrowDown')
  await expect(page.getByLabel('Left', { exact: true })).toHaveValue('11'); await expect(page.getByLabel('Top', { exact: true })).toHaveValue('30')
  await crop(page)
  const result = page.getByAltText('Cropped PNG preview')
  const data = await result.evaluate(async (img: HTMLImageElement) => {
    await img.decode(); const c = document.createElement('canvas'); c.width = img.naturalWidth; c.height = img.naturalHeight
    const ctx = c.getContext('2d')!; ctx.drawImage(img, 0, 0)
    return { width: c.width, height: c.height, pixels: Array.from(ctx.getImageData(0, 0, c.width, c.height).data) }
  })
  expect([data.width, data.height]).toEqual([17, 31])
  for (let y = 0; y < 31; y++) for (let x = 0; x < 17; x++) expect(data.pixels.slice((y * 17 + x) * 4, (y * 17 + x) * 4 + 4)).toEqual([11 + x, 30 + y, 160, 255])
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download PNG' }).click(); expect((await download).suggestedFilename()).toBe('imgengine-cropped.png')
  expect((await new AxeBuilder({ page }).include('.local-crop').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()).violations.map(v => v.id)).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  await fields(page, 400, 202, 1, 1); await expect(result).toHaveCount(0); await crop(page)
  expect(await result.evaluate((img: HTMLImageElement) => { const c = document.createElement('canvas'); c.width = c.height = 1; const ctx = c.getContext('2d')!; ctx.drawImage(img, 0, 0); return [img.naturalWidth, img.naturalHeight, ctx.getImageData(0, 0, 1, 1).data[3]] })).toEqual([1, 1, 0])
  await page.getByLabel('Width', { exact: true }).fill('2'); await expect(page.getByRole('button', { name: 'Crop locally', exact: true })).toBeDisabled()
  await page.getByRole('button', { name: 'Reset selection' }).click(); await expect(page.getByLabel('Width', { exact: true })).toHaveValue('401')
  await page.getByLabel('Aspect ratio').selectOption('square'); await expect(page.getByLabel('Width', { exact: true })).toHaveValue('203'); await expect(page.getByLabel('Left', { exact: true })).toHaveValue('99')
  await page.getByLabel('Width', { exact: true }).fill('50'); await expect(page.getByLabel('Height', { exact: true })).toHaveValue('50')
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'bad', mimeType: 'image/png', buffer: Buffer.from('bad') }); await expect(page.getByRole('alert')).toContainText('Cannot open')
  expect(requests).toEqual([])
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})

test('portrait pointer/touch moves stay bounded and numeric controls resize', async ({ page, isMobile }) => {
  await page.goto('/crop-image'); await choose(page, await source(page, 203, 401)); await fields(page, 10, 10, 50, 60)
  const frame = page.getByRole('group', { name: 'Crop selection preview' })
  await frame.scrollIntoViewIfNeeded()
  const box = (await frame.boundingBox())!
  if (isMobile) {
    const session = await page.context().newCDPSession(page)
    await session.send('Input.dispatchTouchEvent', { type: 'touchStart', touchPoints: [{ x: box.x + box.width / 2, y: box.y + box.height / 2 }] })
    await session.send('Input.dispatchTouchEvent', { type: 'touchMove', touchPoints: [{ x: box.x + box.width - 1, y: box.y + box.height - 1 }] })
    await session.send('Input.dispatchTouchEvent', { type: 'touchEnd', touchPoints: [] })
  } else { await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2); await page.mouse.down(); await page.mouse.move(box.x + box.width + 200, box.y + box.height + 200); await page.mouse.up() }
  const left = Number(await page.getByLabel('Left', { exact: true }).inputValue()), top = Number(await page.getByLabel('Top', { exact: true }).inputValue())
  expect(left).toBeGreaterThan(10); expect(left).toBeLessThanOrEqual(153); expect(top).toBeGreaterThan(10); expect(top).toBeLessThanOrEqual(341)
  await crop(page); expect(await page.getByAltText('Cropped PNG preview').evaluate((img: HTMLImageElement) => [img.naturalWidth, img.naturalHeight])).toEqual([50, 60])
})

test('stored-raster JPEG/PNG orientation remains explicit in cropped output', async ({ page }) => {
  await page.goto('/crop-image')
  for (const jpeg of [false, true]) for (const orientation of [2, 6, 8]) {
    await choose(page, oriented(await source(page, 401, 203, jpeg), orientation, !jpeg, true))
    await fields(page, 11, 30, 17, 31); await crop(page)
    const values = await page.getByAltText('Cropped PNG preview').evaluate((img: HTMLImageElement) => { const c = document.createElement('canvas'); c.width = 17; c.height = 31; const ctx = c.getContext('2d')!; ctx.drawImage(img, 0, 0); return Array.from(ctx.getImageData(8, 8, 1, 1).data) })
    for (const [i, expected] of [19, 38, 160].entries()) expect(Math.abs(values[i]! - expected)).toBeLessThanOrEqual(jpeg ? 5 : 0)
  }
})

test('selection changes, remove and navigation release resources; cancel suppresses output', async ({ page }) => {
  await page.addInitScript(() => {
    const urls = new Set<string>(), create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }; URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    Object.defineProperty(window, 'cropURLs', { get: () => urls.size })
  })
  await page.goto('/crop-image'); const buffer = await source(page)
  const count = () => page.evaluate(() => (window as unknown as { cropURLs: number }).cropURLs)
  for (let i = 0; i < 3; i++) { await choose(page, buffer); await expect.poll(count).toBe(1); await fields(page, 10, 10, 50, 50); await crop(page); await expect.poll(count).toBe(2) }
  await page.getByLabel('Left', { exact: true }).fill('11'); await expect.poll(count).toBe(1)
  await page.evaluate(() => { window.Worker = class { postMessage() {} terminate() {} } as unknown as typeof Worker })
  await page.getByRole('button', { name: 'Crop locally', exact: true }).click(); await page.getByRole('button', { name: 'Cancel', exact: true }).click()
  await expect(page.getByAltText('Cropped PNG preview')).toHaveCount(0)
  await page.getByRole('button', { name: 'Remove image' }).click(); await expect.poll(count).toBe(0)
  await expect(page.getByLabel('Choose a JPEG or PNG', { exact: true })).toBeFocused()
  await choose(page, buffer); await page.getByRole('link', { name: 'Resize locally', exact: true }).click(); await expect.poll(count).toBe(0)
})
