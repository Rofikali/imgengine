import { expect, test } from '@playwright/test'

test('real browser -> Nuxt -> Rust crop and P12 omission return decodable JPEGs and accounting', async ({ page }, testInfo) => {
  const explicitFields: boolean[] = []
  await page.route('**/api/sheet-render', async route => {
    expect(route.request().headers()['x-api-key']).toBeUndefined()
    explicitFields.push(route.request().postDataBuffer()!.toString().includes('name="crop_x"'))
    await route.continue()
  })
  await page.goto('/')
  await page.getByRole('button', { name: 'Physical sheet', exact: true }).click()
  const buffer = Buffer.from(await page.evaluate(() => {
    const canvas = document.createElement('canvas'); canvas.width = 401; canvas.height = 203
    const context = canvas.getContext('2d')!
    context.fillStyle = 'red'; context.fillRect(0, 0, 200, 203)
    context.fillStyle = 'blue'; context.fillRect(200, 0, 201, 203)
    return canvas.toDataURL('image/png').split(',')[1]!
  }), 'base64')
  for (const explicit of [true, false]) {
    await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer })
    await expect(page.getByLabel('Horizontal position:')).toBeVisible()
    // Small physical sheet keeps this contract smoke inexpensive.
    await page.getByLabel('Sheet width in millimetres').fill('25.4')
    await page.getByLabel('Sheet height in millimetres').fill('25.4')
    await page.getByLabel('Photo width in millimetres').fill('25.4')
    await page.getByLabel('Photo height in millimetres').fill('25.4')
    await page.getByLabel('Print DPI').fill('100')
    await page.getByLabel('Exact number of copies').fill('1')
    await page.getByLabel('Sheet margin in millimetres').fill('0')
    if (explicit) { await page.getByLabel('Horizontal position:').focus(); await page.keyboard.press('End') }
    const response = page.waitForResponse('**/api/sheet-render')
    await page.getByRole('button', { name: 'Create physical sheet' }).click()
    const completed = await response
    expect(explicitFields.at(-1)).toBe(explicit)
    expect(completed.status()).toBe(200)
    expect(completed.headers()['x-imgengine-executed-count']).toBe('1')
    const result = page.getByAltText('Rendered physical photo sheet preview')
    await expect(result).toBeVisible()
    await expect(page.getByRole('heading', { name: 'Physical sheet complete' })).toBeFocused()
    await expect.poll(() => result.evaluate((image: HTMLImageElement) => image.naturalWidth)).toBe(100)
    expect(await result.evaluate((image: HTMLImageElement) => image.naturalHeight)).toBe(100)
    await result.screenshot({ path: testInfo.outputPath(explicit ? 'selected-crop.png' : 'p12-centered.png') })
    await page.getByRole('button', { name: 'Create another sheet' }).click()
  }

  const fields = { sheet_width_mm: '25.4', sheet_height_mm: '25.4', photo_width_mm: '25.4', photo_height_mm: '25.4',
    dpi: '100', copies: '1', margin_mm: '0', gap_x_mm: '0', gap_y_mm: '0', output_format: 'jpeg' }
  const cases: Array<{ crop: Array<[string, string]>, status: number }> = [
    { crop: [['crop_x', '0']], status: 400 },
    { crop: [['crop_x', '0'], ['crop_y', '0'], ['crop_width', '0'], ['crop_height', '1000000']], status: 400 },
    { crop: [['crop_x', '-1'], ['crop_y', '0'], ['crop_width', '500000'], ['crop_height', '1000000']], status: 400 },
    { crop: [['crop_x', '0'], ['crop_x', '0'], ['crop_y', '0'], ['crop_width', '500000'], ['crop_height', '1000000']], status: 400 },
    { crop: [['crop_x', '0'], ['crop_y', '0'], ['crop_width', '1000000'], ['crop_height', '1000000']], status: 413 },
  ]
  for (const invalid of cases) {
    const form = new FormData()
    form.append('file', new Blob([buffer], { type: 'image/png' }), 'fixture.png')
    for (const [name, value] of [...Object.entries(fields), ...invalid.crop]) form.append(name!, value!)
    const response = await fetch('http://127.0.0.1:3100/api/sheet-render', { method: 'POST', body: form })
    expect(response.status).toBe(invalid.status)
    expect((await response.json()).code).toBe(invalid.status === 400 ? 'invalid_request' : 'resource_limit')
  }
})
