import { expect, test } from '@playwright/test'
import { oriented } from './orientation'

test('P12 centered multi-copy partial row and P13 final one-pixel boundary', async ({ page }) => {
  await page.goto('/')
  const results = await page.evaluate(async () => {
    const canvas = document.createElement('canvas'); canvas.width = 3; canvas.height = 3
    const ctx = canvas.getContext('2d')!
    ctx.fillStyle = '#e02020'; ctx.fillRect(0, 0, 3, 3)
    ctx.fillStyle = '#20d030'; ctx.fillRect(2, 2, 1, 1)
    const file = await new Promise<Blob>(resolve => canvas.toBlob(blob => resolve(blob!)))
    const values = { sheet_width_mm: '20', sheet_height_mm: '20', photo_width_mm: '8', photo_height_mm: '8', dpi: '254', copies: '3', margin_mm: '0', gap_x_mm: '0', gap_y_mm: '0', output_format: 'jpeg' }
    const samples = []
    for (const crop of [false, true]) {
      const form = new FormData(); form.append('file', file, 'fixture.png')
      for (const [key, value] of Object.entries(values)) form.append(key, value)
      if (crop) for (const [key, value] of Object.entries({ crop_x: '666667', crop_y: '666667', crop_width: '333333', crop_height: '333333' })) form.append(key, value)
      const response = await fetch('/api/sheet-render', { method: 'POST', body: form })
      const url = URL.createObjectURL(await response.blob()), img = new Image()
      try {
        img.src = url; await img.decode()
        canvas.width = img.naturalWidth; canvas.height = img.naturalHeight; ctx.drawImage(img, 0, 0)
        const at = (x: number, y: number) => Array.from(ctx.getImageData(x, y, 1, 1).data).slice(0, 3)
        samples.push({ status: response.status, counts: ['expected-count', 'planned-count', 'executed-count'].map(k => response.headers.get(`x-imgengine-${k}`)), width: img.naturalWidth, height: img.naturalHeight,
          cells: [at(30, 30), at(110, 30), at(30, 110)], blank: at(150, 150), margin: at(5, 5) })
      } finally { URL.revokeObjectURL(url) }
    }
    return samples
  })
  for (const [index, result] of results.entries()) {
    expect(result.status).toBe(200); expect(result.counts).toEqual(['3', '3', '3'])
    expect([result.width, result.height]).toEqual([200, 200])
    for (const sample of [result.blank, result.margin]) expect(sample.every(v => v >= 250)).toBe(true)
    const color = index === 0 ? [224, 32, 32] : [32, 208, 48]
    for (const cell of result.cells) for (let c = 0; c < 3; c++) expect(Math.abs(cell[c]! - color[c]!)).toBeLessThanOrEqual(12)
  }
})

test('malformed EXIF, bounded preview, repeated replacement and no automatic upload', async ({ page }) => {
  const posts: string[] = []
  page.on('request', request => { if (request.method() === 'POST') posts.push(request.url()) })
  await page.addInitScript(() => {
    const urls = new Set<string>(), create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); urls.add(url); return url }
    URL.revokeObjectURL = url => { urls.delete(url); revoke(url) }
    Object.defineProperty(window, 'activeURLs', { get: () => urls.size })
  })
  await page.goto('/'); await page.getByRole('button', { name: 'Physical sheet', exact: true }).click()
  const buffer = Buffer.from(await page.evaluate(() => {
    const c = document.createElement('canvas'); c.width = 32; c.height = 16
    return c.toDataURL().split(',')[1]!
  }), 'base64')
  for (const value of [0, 9, 65535]) {
    await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer: oriented(buffer, value, true, false) })
    await expect(page.getByText('This image cannot be previewed safely.', { exact: false })).toBeVisible()
    await expect(page.getByAltText('Selected photo within the crop frame')).toHaveCount(0)
  }
  const tooWide = Buffer.from(buffer); tooWide.writeUInt32BE(8193, 16)
  await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer: tooWide })
  await expect(page.getByText('The crop preview supports images up to', { exact: false })).toBeVisible()
  const count = () => page.evaluate(() => (window as unknown as { activeURLs: number }).activeURLs)
  for (let i = 0; i < 10; i++) {
    await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer })
    await expect(page.getByAltText('Selected photo within the crop frame')).toBeVisible()
    await expect.poll(count).toBe(1)
  }
  await page.getByRole('button', { name: 'Remove photo' }).click()
  await expect.poll(count).toBe(0)
  expect(posts).toEqual([])
})

test('P12/P13 invalid HTTP fields retain safe status/code and body limits', async () => {
  const fields = { sheet_width_mm: '25.4', sheet_height_mm: '25.4', photo_width_mm: '25.4', photo_height_mm: '25.4',
    dpi: '100', copies: '1', margin_mm: '0', gap_x_mm: '0', gap_y_mm: '0', output_format: 'jpeg' }
  const cases: Array<[Record<string, string>, number, string]> = [
    [{ photo_width_mm: 'NaN' }, 400, 'invalid_request'], [{ dpi: '1201' }, 413, 'resource_limit'],
    [{ copies: '65' }, 413, 'resource_limit'], [{ output_format: 'png' }, 400, 'invalid_request'],
    [{ crop_x: '1e6', crop_y: '0', crop_width: '1', crop_height: '1' }, 400, 'invalid_request'],
    [{ crop_x: '4294967296', crop_y: '0', crop_width: '1', crop_height: '1' }, 400, 'invalid_request'],
    [{ crop_x: '0.5', crop_y: '0', crop_width: '1', crop_height: '1' }, 400, 'invalid_request'],
    [{ source_width: '100' }, 400, 'invalid_request'],
    [{ copies: '2' }, 422, 'does_not_fit'],
    [{ sheet_width_mm: '10000' }, 413, 'resource_limit'],
  ]
  for (const [changes, status, code] of cases) {
    const form = new FormData()
    form.append('file', new Blob(['invalid'], { type: 'image/png' }), 'fixture.png')
    for (const [key, value] of Object.entries({ ...fields, ...changes })) form.append(key, value)
    const response = await fetch('http://127.0.0.1:3100/api/sheet-render', { method: 'POST', body: form })
    expect(response.status, JSON.stringify(changes)).toBe(status)
    const body = await response.json()
    expect(body.code).toBe(code)
    expect(Object.keys(body).sort()).toEqual(['code', 'request_id', 'status', 'title', 'type'])
  }
  const form = new FormData()
  form.append('file', new Blob([new Uint8Array(1048577)], { type: 'image/png' }), 'fixture.png')
  const response = await fetch('http://127.0.0.1:3100/api/sheet-render', { method: 'POST', body: form })
  expect(response.status).toBe(413); expect((await response.json()).code).toBe('resource_limit')
})

// Fixed before measuring: whole-frame RGB MAE <= 4, P99 <= 24 (8-bit units).
// Geometry is exact; these budgets allow JPEG/chroma and resampler differences.
for (const [sw, sh] of [[401, 203], [203, 401], [257, 257], [640, 480]]) {
  for (const [tw, th] of [[80, 120], [120, 80]]) for (const codec of ['png', 'jpeg']) {
    for (const mode of ['p12', 'top-left', 'bottom-right', 'zoom', 'fractional']) {
      test(`${sw}x${sh} -> ${tw}x${th} ${codec} ${mode}`, async ({ page }, info) => {
        await page.goto('/')
        await page.getByRole('button', { name: 'Physical sheet', exact: true }).click()
        const source = await page.evaluate(({ sw, sh, codec }) => {
          const c = document.createElement('canvas'); c.width = sw!; c.height = sh!
          const ctx = c.getContext('2d')!, data = ctx.createImageData(sw!, sh!)
          for (let y = 0; y < sh!; y++) for (let x = 0; x < sw!; x++) {
            const i = (y * sw! + x) * 4
            data.data[i] = 20 + 210 * x / (sw! - 1)
            data.data[i + 1] = 25 + 200 * y / (sh! - 1)
            data.data[i + 2] = 80 + 50 * Math.sin(x / 17) + 40 * Math.cos(y / 23)
            data.data[i + 3] = 255
          }
          ctx.putImageData(data, 0, 0)
          return c.toDataURL(`image/${codec}`, 0.95)
        }, { sw, sh, codec })
        const buffer = Buffer.from(source.split(',')[1]!, 'base64')
        await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: `fixture.${codec}`, mimeType: `image/${codec}`, buffer })
        await expect(page.getByLabel('Zoom:')).toBeVisible()
        for (const label of ['Sheet width', 'Photo width']) await page.getByLabel(`${label} in millimetres`).fill(String(tw! / 10))
        for (const label of ['Sheet height', 'Photo height']) await page.getByLabel(`${label} in millimetres`).fill(String(th! / 10))
        await page.getByLabel('Print DPI').fill('254')
        await page.getByLabel('Exact number of copies').fill('1')
        await page.getByLabel('Sheet margin in millimetres').fill('0')
        if (mode !== 'p12') {
          for (const label of ['Horizontal position:', 'Vertical position:']) {
            await page.getByLabel(label).focus()
            await page.keyboard.press(mode === 'top-left' ? 'Home' : 'End')
          }
          if (mode === 'zoom') { await page.getByLabel('Zoom:').focus(); await page.keyboard.press('End') }
          if (mode === 'fractional') { await page.getByLabel('Zoom:').focus(); await page.keyboard.press('ArrowRight') }
        }
        let crop: number[] = []
        await page.route('**/api/sheet-render', async route => {
          const request = route.request()
          const form = await new Request('http://localhost', { method: 'POST', headers: { 'content-type': request.headers()['content-type']! }, body: new Uint8Array(request.postDataBuffer()!) }).formData()
          crop = ['crop_x', 'crop_y', 'crop_width', 'crop_height'].filter(k => form.has(k)).map(k => Number(form.get(k)))
          expect(crop.length).toBe(mode === 'p12' ? 0 : 4)
          expect(request.headers()['x-api-key']).toBeUndefined()
          await route.continue()
        })
        const response = page.waitForResponse('**/api/sheet-render')
        await page.getByRole('button', { name: 'Create physical sheet' }).click()
        const completed = await response
        expect(completed.status()).toBe(200)
        for (const header of ['expected-count', 'planned-count', 'executed-count']) expect(completed.headers()[`x-imgengine-${header}`]).toBe('1')
        expect(completed.headers()['x-imgengine-canvas-width']).toBe(String(tw))
        expect(completed.headers()['x-imgengine-canvas-height']).toBe(String(th))
        expect(completed.headers()['content-type']).toContain('image/jpeg')
        const result = page.getByAltText('Rendered physical photo sheet preview')
        await expect(result).toBeVisible()
        const metrics = await result.evaluate(async (output: HTMLImageElement, args) => {
          const { sw, sh, tw, th, crop, source, mode } = args
          await output.decode()
          const original = new Image(); original.src = source; await original.decode()
          // Independent integer oracle: no production geometry helper imported.
          let w = sw!, h = sh!
          if (BigInt(sw!) * BigInt(th!) > BigInt(sh!) * BigInt(tw!)) w = Number(BigInt(sh!) * BigInt(tw!) / BigInt(th!))
          else h = Number(BigInt(sw!) * BigInt(th!) / BigInt(tw!))
          let x = Math.floor((sw! - w) / 2), y = Math.floor((sh! - h) / 2)
          if (crop.length) {
            const scale = 1000000n
            const left = BigInt(crop[0]!) * BigInt(sw!) / scale
            const top = BigInt(crop[1]!) * BigInt(sh!) / scale
            const right = ((BigInt(crop[0]!) + BigInt(crop[2]!)) * BigInt(sw!) + scale - 1n) / scale
            const bottom = ((BigInt(crop[1]!) + BigInt(crop[3]!)) * BigInt(sh!) + scale - 1n) / scale
            x = Number(left); y = Number(top); w = Number(right - left); h = Number(bottom - top)
          }
          const canvas = document.createElement('canvas'); canvas.width = tw!; canvas.height = th!
          const ctx = canvas.getContext('2d')!
          ctx.drawImage(original, x, y, w, h, 0, 0, tw!, th!)
          const reference = ctx.getImageData(0, 0, tw!, th!).data
          ctx.drawImage(output, 0, 0)
          const actual = ctx.getImageData(0, 0, tw!, th!).data
          const errors: number[] = []
          for (let i = 0; i < actual.length; i++) if (i % 4 !== 3) errors.push(Math.abs(actual[i]! - reference[i]!))
          errors.sort((a, b) => a - b)
          return { mode, crop, rect: { x, y, w, h }, mae: errors.reduce((a, b) => a + b, 0) / errors.length, p99: errors[Math.floor(errors.length * .99)]!, max: errors.at(-1), width: output.naturalWidth, height: output.naturalHeight }
        }, { sw, sh, tw, th, crop, source, mode })
        await info.attach('parity-metrics', { body: JSON.stringify(metrics), contentType: 'application/json' })
        const zoom = mode === 'zoom' ? 4 : mode === 'fractional' ? 1.05 : 1
        let expectedW: number, expectedH: number
        if (sw! * th! > sh! * tw!) {
          expectedH = Math.floor(sh! / zoom)
          expectedW = Number(BigInt(expectedH) * BigInt(tw!) / BigInt(th!))
        } else {
          expectedW = Math.floor(sw! / zoom)
          expectedH = Number(BigInt(expectedW) * BigInt(th!) / BigInt(tw!))
        }
        expect(metrics.rect.w).toBe(expectedW); expect(metrics.rect.h).toBe(expectedH)
        if (mode === 'p12') {
          expect(metrics.rect.x).toBe(Math.floor((sw! - expectedW) / 2))
          expect(metrics.rect.y).toBe(Math.floor((sh! - expectedH) / 2))
        }
        expect(metrics.width).toBe(tw); expect(metrics.height).toBe(th)
        if (mode === 'top-left') { expect(metrics.rect.x).toBe(0); expect(metrics.rect.y).toBe(0) }
        if (mode === 'bottom-right' || mode === 'zoom') {
          expect(metrics.rect.x + metrics.rect.w).toBe(sw); expect(metrics.rect.y + metrics.rect.h).toBe(sh)
        }
        expect(metrics.mae).toBeLessThanOrEqual(4)
        expect(metrics.p99).toBeLessThanOrEqual(24)
      })
    }
  }
}
