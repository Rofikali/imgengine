import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'
import { oriented } from './orientation'

async function fixture(page: Page, jpeg = false) {
  return Buffer.from(await page.evaluate(jpeg => {
    const c = document.createElement('canvas'); c.width = 401; c.height = 203
    const context = c.getContext('2d')!
    context.fillStyle = '#ee2020'; context.fillRect(0, 0, 200, 203)
    context.fillStyle = '#2040ee'; context.fillRect(200, 0, 201, 203)
    return c.toDataURL(jpeg ? 'image/jpeg' : 'image/png').split(',')[1]!
  }, jpeg), 'base64')
}
async function select(page: Page, buffer: Buffer, jpeg = false) {
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'private-fixture', mimeType: jpeg ? 'image/jpeg' : 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}
test('local resize, aspect lock, PNG download, no image network/storage and accessibility', async ({ page }) => {
  const requests: Array<{ url: string, method: string, body: string | null }> = []
  await page.goto('/resize-image')
  page.on('request', r => { if (/^https?:/.test(r.url())) requests.push({ url: r.url(), method: r.method(), body: r.postData() }) })
  await select(page, await fixture(page))
  await page.getByLabel('Width', { exact: true }).focus()
  await page.keyboard.press('ControlOrMeta+A'); await page.keyboard.type('100')
  await expect(page.getByLabel('Height', { exact: true })).toHaveValue('51')
  await page.getByRole('button', { name: 'Resize locally', exact: true }).focus(); await page.keyboard.press('Enter')
  const output = page.getByAltText('Resized PNG preview')
  await expect(output).toBeVisible()
  const pixels = await output.evaluate(async (img: HTMLImageElement) => {
    await img.decode()
    const c = document.createElement('canvas'); c.width = img.naturalWidth; c.height = img.naturalHeight
    const ctx = c.getContext('2d')!; ctx.drawImage(img, 0, 0)
    return { width: c.width, height: c.height, red: Array.from(ctx.getImageData(10, 10, 1, 1).data), blue: Array.from(ctx.getImageData(90, 10, 1, 1).data) }
  })
  expect([pixels.width, pixels.height]).toEqual([100, 51])
  expect(pixels.red).toEqual([238, 32, 32, 255]); expect(pixels.blue).toEqual([32, 64, 238, 255])
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download PNG' }).click()
  expect((await download).suggestedFilename()).toBe('imgengine-resized.png')
  const audit = await new AxeBuilder({ page }).include('.local-resize').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()
  expect(audit.violations.map(v => ({ id: v.id, nodes: v.nodes.map(n => n.target) }))).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  const assetOnly = (r: typeof requests[number]) => {
    const url = new URL(r.url)
    return r.method === 'GET' && !r.body && url.origin === 'http://127.0.0.1:3100' && !url.search
      && (/^\/_nuxt\/[^/]+\.(js|css)$/.test(url.pathname) || /^\/_nuxt\/builds\/meta\/[a-f0-9-]+\.json$/.test(url.pathname))
  }
  expect(requests.every(assetOnly), JSON.stringify(requests.map(r => ({ method: r.method, path: new URL(r.url).pathname, hasBody: !!r.body })))).toBe(true)
  expect(await page.evaluate(async () => ({ local: localStorage.length, session: sessionStorage.length, databases: (await indexedDB.databases()).length }))).toEqual({ local: 0, session: 0, databases: 0 })
  await page.getByLabel('Maintain aspect ratio').uncheck()
  await page.getByLabel('Width', { exact: true }).fill('0')
  await expect(page.getByRole('button', { name: 'Resize locally', exact: true })).toBeDisabled()
  await expect(output).toHaveCount(0)
  await page.getByLabel('Width', { exact: true }).fill('4096'); await page.getByLabel('Height', { exact: true }).fill('4096')
  await expect(page.getByRole('alert')).toContainText('4 megapixels')
  await page.getByLabel('Width', { exact: true }).fill('1'); await page.getByLabel('Height', { exact: true }).fill('1')
  await page.getByRole('button', { name: 'Resize locally', exact: true }).click(); await expect(output).toBeVisible()
  expect(await output.evaluate((img: HTMLImageElement) => [img.naturalWidth, img.naturalHeight])).toEqual([1, 1])
})

test('stored-raster EXIF JPEG/PNG 1–8, malformed metadata and bounded input', async ({ page }) => {
  await page.goto('/resize-image')
  for (const jpeg of [false, true]) {
    const buffer = await fixture(page, jpeg)
    for (let value = 1; value <= 8; value++) {
      await select(page, oriented(buffer, value, !jpeg, value % 2 === 0), jpeg)
      await expect(page.getByLabel('Width', { exact: true })).toHaveValue('401')
      await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
      const output = page.getByAltText('Resized PNG preview'); await expect(output).toBeVisible()
      const pixel = await output.evaluate((img: HTMLImageElement) => { const c = document.createElement('canvas'); c.width = 401; c.height = 203; const ctx = c.getContext('2d')!; ctx.drawImage(img, 0, 0); return Array.from(ctx.getImageData(10, 10, 1, 1).data) })
      expect(pixel[0]).toBeGreaterThan(220); expect(pixel[2]).toBeLessThan(50)
    }
    await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'invalid', mimeType: 'image/png', buffer: oriented(buffer, 9, !jpeg, true) })
    await expect(page.getByRole('alert')).toContainText('Cannot open')
    await expect(page.getByAltText('Local source image')).toHaveCount(0)
  }
  for (const buffer of [Buffer.from('not an image'), Buffer.alloc(4 * 1024 * 1024 + 1)]) {
    await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'invalid', mimeType: 'image/png', buffer })
    await expect(page.getByRole('alert')).toContainText('Cannot open')
  }
  const large = await fixture(page); large.writeUInt32BE(8193, 16)
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'large', mimeType: 'image/png', buffer: large })
  await expect(page.getByRole('alert')).toContainText('Cannot open')
})

test('repeated operations release URLs and Workers on replace, remove and route exit', async ({ page }) => {
  await page.addInitScript(() => {
    const urls = new Set<string>(), workers = new Set<Worker>()
    const create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL), Original = Worker
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }
    URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    window.Worker = class extends Original {
      constructor(url: string | URL, options?: WorkerOptions) { super(url, options); workers.add(this) }
      override terminate() { workers.delete(this); super.terminate() }
    }
    Object.defineProperty(window, 'localResources', { get: () => [urls.size, workers.size] })
  })
  await page.goto('/resize-image'); const buffer = await fixture(page)
  const resources = () => page.evaluate(() => (window as unknown as { localResources: number[] }).localResources)
  for (let i = 0; i < 5; i++) {
    await select(page, buffer); await expect.poll(resources).toEqual([1, 0])
    await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
    await expect(page.getByAltText('Resized PNG preview')).toBeVisible(); await expect.poll(resources).toEqual([2, 0])
  }
  await page.getByRole('button', { name: 'Remove image' }).click(); await expect.poll(resources).toEqual([0, 0])
  await expect(page.getByLabel('Choose a JPEG or PNG', { exact: true })).toBeFocused()
  await select(page, buffer)
  await page.getByRole('link', { name: 'Server print layouts (uploads on Render)' }).click()
  await expect(page.getByRole('heading', { name: 'Create printable photo layouts.' })).toBeVisible()
  await expect.poll(resources).toEqual([0, 0])
})

test('cancel and Worker failure discard pending work without upload', async ({ page }) => {
  const posts: string[] = []; page.on('request', r => { if (r.method() !== 'GET') posts.push(r.url()) })
  await page.addInitScript(() => {
    window.Worker = class {
      onmessage = null; onerror = null
      postMessage() { Object.defineProperty(window, 'workerPosted', { value: true, configurable: true }) } terminate() {}
    } as unknown as typeof Worker
  })
  await page.goto('/resize-image'); await select(page, await fixture(page))
  await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
  await page.getByRole('button', { name: 'Cancel', exact: true }).click()
  await expect(page.getByAltText('Resized PNG preview')).toHaveCount(0)
  await expect(page.getByRole('button', { name: 'Resize locally', exact: true })).toBeEnabled()
  await page.clock.install()
  await page.evaluate(() => Object.defineProperty(window, 'workerPosted', { value: false, configurable: true }))
  await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
  await page.waitForFunction(() => (window as unknown as { workerPosted: boolean }).workerPosted)
  await page.clock.fastForward(10_001)
  await expect(page.getByRole('alert')).toContainText('Nothing was uploaded')
  await page.evaluate(() => { window.Worker = class { constructor() { throw Error('unsupported') } } as unknown as typeof Worker })
  await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
  await expect(page.getByRole('alert')).toContainText('Nothing was uploaded')
  expect(posts).toEqual([])
})

test('accepted four-megapixel boundary produces exact PNG dimensions', async ({ page }, info) => {
  await page.goto('/resize-image')
  const buffer = Buffer.from(await page.evaluate(() => {
    const c = document.createElement('canvas'); c.width = 2000; c.height = 2000
    const ctx = c.getContext('2d')!; ctx.fillStyle = '#218040'; ctx.fillRect(0, 0, 2000, 2000)
    return c.toDataURL().split(',')[1]!
  }), 'base64')
  await select(page, buffer)
  const start = Date.now()
  await page.getByRole('button', { name: 'Resize locally', exact: true }).click()
  const output = page.getByAltText('Resized PNG preview'); await expect(output).toBeVisible()
  expect(await output.evaluate((img: HTMLImageElement) => [img.naturalWidth, img.naturalHeight])).toEqual([2000, 2000])
  await info.attach('bounded-fixture-timing', { body: JSON.stringify({ inputPixels: 4_000_000, outputPixels: 4_000_000, clickToPreviewMs: Date.now() - start }), contentType: 'application/json' })
  await page.getByRole('button', { name: 'Remove image' }).click()
  await expect(output).toHaveCount(0)
})

test('a replaced slow file cannot publish a stale preview', async ({ page }) => {
  await page.goto('/resize-image')
  const slow = await fixture(page)
  const small = Buffer.from(await page.evaluate(() => {
    const c = document.createElement('canvas'); c.width = 1; c.height = 1
    return c.toDataURL().split(',')[1]!
  }), 'base64')
  await page.evaluate(() => {
    const original = File.prototype.arrayBuffer
    File.prototype.arrayBuffer = async function () {
      if (this.name === 'slow') {
        await new Promise(resolve => setTimeout(resolve, 300))
        Object.defineProperty(window, 'slowReadFinished', { value: true })
      }
      return original.call(this)
    }
  })
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'slow', mimeType: 'image/png', buffer: slow })
  await select(page, small)
  await page.waitForFunction(() => (window as unknown as { slowReadFinished: boolean }).slowReadFinished)
  await expect(page.getByLabel('Width', { exact: true })).toHaveValue('1')
  await expect(page.getByLabel('Height', { exact: true })).toHaveValue('1')
  await expect(page.getByRole('alert')).toHaveCount(0)
})
