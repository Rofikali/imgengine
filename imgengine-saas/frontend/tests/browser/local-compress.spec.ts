import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'
import { oriented } from './orientation'

async function fixture(page: Page, transparent = false) {
  return Buffer.from(await page.evaluate(transparent => {
    const canvas = document.createElement('canvas'); canvas.width = transparent ? 96 : 3; canvas.height = transparent ? 96 : 2
    const context = canvas.getContext('2d')!
    context.fillStyle = '#e02020'; context.fillRect(0, 0, transparent ? 48 : 2, transparent ? 96 : 2)
    context.clearRect(transparent ? 48 : 2, 0, transparent ? 48 : 1, transparent ? 96 : 2)
    if (!transparent) { context.fillStyle = '#2040e0'; context.fillRect(2, 0, 1, 2) }
    return canvas.toDataURL('image/png').split(',')[1]!
  }, transparent), 'base64')
}

/** Construct a real synthetic EXIF GPS IFD, not a customer photo or a word-only canary. */
function withGpsExif(jpeg: Buffer): Buffer {
  const tiff = Buffer.alloc(56)
  const view = new DataView(tiff.buffer, tiff.byteOffset, tiff.byteLength)
  view.setUint16(0, 0x4949)
  view.setUint16(2, 42, true)
  view.setUint32(4, 8, true)
  view.setUint16(8, 2, true)
  // Orientation (SHORT, one value): stored raster 3 x 2, orientation six.
  view.setUint16(10, 0x0112, true); view.setUint16(12, 3, true)
  view.setUint32(14, 1, true); view.setUint16(18, 6, true)
  // GPSInfoIFDPointer (LONG, one value), with an actual GPS IFD at offset 38.
  view.setUint16(22, 0x8825, true); view.setUint16(24, 4, true)
  view.setUint32(26, 1, true); view.setUint32(30, 38, true)
  view.setUint16(38, 1, true)
  view.setUint16(40, 1, true); view.setUint16(42, 2, true)
  view.setUint32(44, 2, true); tiff[48] = 'N'.charCodeAt(0)
  const header = Buffer.from('Exif\\0\\0', 'binary')
  const app1Length = tiff.length + header.length + 2
  return Buffer.concat([
    jpeg.subarray(0, 2),
    Buffer.from([0xff, 0xe1, app1Length >>> 8, app1Length & 0xff]),
    header, tiff, jpeg.subarray(2),
  ])
}

/** Independent JPEG marker parser: inspect APP/COM segments, not ASCII in entropy. */
function jpegMetadataSegments(bytes: Buffer) {
  if (bytes[0] !== 0xff || bytes[1] !== 0xd8) throw Error('Not JPEG')
  const segments: Array<{ marker: number, data: Buffer }> = []
  for (let offset = 2; offset + 3 < bytes.length;) {
    if (bytes[offset] !== 0xff) throw Error('Malformed JPEG marker')
    while (bytes[offset] === 0xff) offset++
    const marker = bytes[offset++]
    if (marker === 0xda || marker === 0xd9) break
    const size = bytes.readUInt16BE(offset)
    if (size < 2 || offset + size > bytes.length) throw Error('Malformed JPEG length')
    if ((marker >= 0xe0 && marker <= 0xef) || marker === 0xfe) {
      segments.push({ marker, data: bytes.subarray(offset + 2, offset + size) })
    }
    offset += size
  }
  return segments
}

async function exportedJpeg(page: Page): Promise<Buffer> {
  await expect(page.getByAltText('Compressed JPEG preview')).toBeVisible({ timeout: 20000 })
  const result = await page.getByAltText('Compressed JPEG preview').evaluate(async (image: HTMLImageElement) => {
    const blob = await (await fetch(image.src)).blob()
    return { type: blob.type, bytes: Array.from(new Uint8Array(await blob.arrayBuffer())) }
  })
  expect(result.type).toBe('image/jpeg')
  return Buffer.from(result.bytes)
}

async function choose(page: Page, buffer: Buffer) {
  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'private-fixture.png', mimeType: 'image/png', buffer })
  await expect(page.getByAltText('Local source image')).toBeVisible()
}

test('compresses locally to JPEG, composites alpha onto white, downloads, and passes accessibility', async ({ page }) => {
  const requests: Array<{ method: string, url: string, body: string | null }> = []
  await page.goto('/compress-image')
  page.on('request', request => { if (/^https?:/.test(request.url())) requests.push({ method: request.method(), url: request.url(), body: request.postData() }) })
  await choose(page, await fixture(page, true))
  await page.getByLabel('Quality', { exact: true }).fill('80')
  await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  const output = page.getByAltText('Compressed JPEG preview'); await expect(output).toBeVisible()
  const pixels = await output.evaluate(async (img: HTMLImageElement) => {
    await img.decode(); const canvas = document.createElement('canvas'); canvas.width = img.naturalWidth; canvas.height = img.naturalHeight
    const context = canvas.getContext('2d')!; context.drawImage(img, 0, 0)
    const bytes = new Uint8Array(await (await fetch(img.src)).blob().then(blob => blob.arrayBuffer()) as ArrayBuffer)
    const hasExif = (() => { for (let index = 0; index + 3 < bytes.byteLength; index++) if (bytes[index] === 0x45 && bytes[index + 1] === 0x78 && bytes[index + 2] === 0x69 && bytes[index + 3] === 0x66) return true; return false })()
    return { type: await (await fetch(img.src)).blob().then(blob => blob.type), size: [canvas.width, canvas.height], transparentArea: Array.from(context.getImageData(72, 48, 1, 1).data), hasExif }
  })
  expect(pixels.type).toBe('image/jpeg'); expect(pixels.size).toEqual([96, 96]); expect(pixels.hasExif).toBe(false)
  expect(pixels.transparentArea[0]).toBeGreaterThan(220); expect(pixels.transparentArea[1]).toBeGreaterThan(220); expect(pixels.transparentArea[2]).toBeGreaterThan(220)
  const download = page.waitForEvent('download'); await page.getByRole('link', { name: 'Download JPEG' }).click()
  expect((await download).suggestedFilename()).toBe('imgengine-compressed.jpg')
  expect((await new AxeBuilder({ page }).include('.local-compress').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()).violations.map(v => v.id)).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true)
  const localAsset = (request: typeof requests[number]) => {
    const url = new URL(request.url)
    return request.method === 'GET' && !request.body && url.origin === 'http://127.0.0.1:3100'
      && (/^\/_nuxt\//.test(url.pathname) || url.pathname === '/compress-image' || url.pathname === '/')
  }
  expect(requests.every(localAsset), JSON.stringify(requests)).toBe(true)
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})

test('JPEG orientation and GPS EXIF are stripped from the exported JPEG across browsers', async ({ page }) => {
  await page.goto('/compress-image')
  const jpeg = Buffer.from(await page.evaluate(() => {
    const canvas = document.createElement('canvas'); canvas.width = 3; canvas.height = 2
    canvas.getContext('2d')!.fillRect(0, 0, 3, 2)
    return canvas.toDataURL('image/jpeg').split(',')[1]!
  }), 'base64')
  const input = withGpsExif(jpeg)
  const inputSegments = jpegMetadataSegments(input)
  expect(inputSegments.some(s => s.marker === 0xe1 && s.data.subarray(0, 6).equals(Buffer.from('Exif\\0\\0', 'binary')))).toBe(true)
  expect(inputSegments.some(s => s.marker === 0xe1 && s.data.includes(Buffer.from([0x25, 0x88])))).toBe(true)

  await page.getByLabel('Choose a JPEG or PNG', { exact: true }).setInputFiles({ name: 'gps-orientation.jpg', mimeType: 'image/jpeg', buffer: input })
  await expect(page.getByText('Source: 3 × 2 pixels.', { exact: true })).toBeVisible()
  await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  const output = await exportedJpeg(page)
  const metadata = jpegMetadataSegments(output)
  expect(metadata.some(s => s.marker === 0xe1 || s.marker === 0xed || s.marker === 0xfe)).toBe(false)
  expect(output.length).toBeGreaterThan(0)
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

test('recovers locally if Worker image decoding fails, with no image-bearing request', async ({ page }) => {
  await page.goto('/compress-image')
  const requests: Array<{ method: string, url: string, body: string | null }> = []
  page.on('request', request => {
    if (/^https?:/.test(request.url())) requests.push({ method: request.method(), url: request.url(), body: request.postData() })
  })
  await choose(page, await fixture(page, true))
  await page.evaluate(() => {
    window.Worker = class {
      onerror: (() => void) | null = null
      postMessage() { setTimeout(() => this.onerror?.(), 0) }
      terminate() {}
    } as unknown as typeof Worker
  })
  await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  const output = await exportedJpeg(page)
  expect(jpegMetadataSegments(output).some(s => s.marker === 0xe1)).toBe(false)
  expect(requests.every(r => r.method === 'GET' && !r.body && new URL(r.url).origin === 'http://127.0.0.1:3100')).toBe(true)
  await page.getByRole('button', { name: 'Remove image' }).click()
  await expect(page.getByAltText('Compressed JPEG preview')).toHaveCount(0)
})

test('synchronous Canvas encoding failure rejects promptly and releases source on remove', async ({ page }) => {
  await page.goto('/compress-image')
  await choose(page, await fixture(page))
  await page.evaluate(() => {
    HTMLCanvasElement.prototype.toBlob = () => { throw Error('synthetic Canvas failure') }
  })
  await page.getByRole('button', { name: 'Compress locally', exact: true }).click()
  await expect(page.getByRole('alert')).toContainText('Local compression could not finish', { timeout: 20000 })
  await page.getByRole('button', { name: 'Remove image' }).click()
  await expect(page.getByAltText('Local source image')).toHaveCount(0)
})
