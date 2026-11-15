import { expect, test, type Page } from '@playwright/test'
import AxeBuilder from '@axe-core/playwright'

async function fixture(page: Page) {
  return Buffer.from(await page.evaluate(() => {
    const canvas = document.createElement('canvas')
    canvas.width = 401; canvas.height = 203
    const context = canvas.getContext('2d')!
    context.fillStyle = '#b72222'; context.fillRect(0, 0, 200, 203)
    context.fillStyle = '#125a98'; context.fillRect(200, 0, 201, 203)
    return canvas.toDataURL('image/png').split(',')[1]!
  }), 'base64')
}

async function openEditor(page: Page) {
  await page.goto('/')
  await page.getByRole('button', { name: 'Physical sheet', exact: true }).click()
  const buffer = await fixture(page)
  await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer })
  await expect(page.getByRole('group', { name: 'Photo crop preview', exact: true })).toBeVisible()
  return buffer
}

test('keyboard, non-drag controls, reset, accessibility and no upload while editing', async ({ page }) => {
  const uploads: string[] = []
  page.on('request', request => { if (request.method() === 'POST') uploads.push(request.url()) })
  await openEditor(page)
  const frame = page.getByRole('group', { name: 'Photo crop preview', exact: true })
  await frame.focus(); await page.keyboard.press('ArrowRight')
  await expect(page.getByLabel('Horizontal position:')).toHaveValue('51')
  await page.getByLabel('Zoom:').focus(); await page.keyboard.press('ArrowRight')
  await expect(page.getByLabel('Zoom:')).toHaveValue('1.05')
  await page.getByRole('button', { name: 'Reset crop to centered fill' }).click()
  await expect(page.getByText('Default centered fill will be used.', { exact: false })).toBeVisible()
  await page.getByLabel('Photo width in millimetres').fill('45')
  await page.getByLabel('Photo height in millimetres').fill('45')
  const bounds = await frame.boundingBox()
  expect(Math.abs(bounds!.width - bounds!.height)).toBeLessThan(2)
  expect(uploads).toEqual([])
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= window.innerWidth)).toBe(true)
  const a11y = await new AxeBuilder({ page }).include('.sheet-panel').withTags(['wcag2a', 'wcag2aa', 'wcag21aa']).analyze()
  expect(a11y.violations.map(({ id, nodes }) => ({ id, targets: nodes.map(node => node.target) }))).toEqual([])
})

test('pointer drag and touch pan stay bounded and permit reset', async ({ page, isMobile }) => {
  await openEditor(page)
  const frame = page.getByRole('group', { name: 'Photo crop preview', exact: true })
  await frame.scrollIntoViewIfNeeded()
  const box = (await frame.boundingBox())!
  const x = box.x + box.width / 2, y = box.y + box.height / 2
  if (isMobile) {
    const session = await page.context().newCDPSession(page)
    await session.send('Input.dispatchTouchEvent', { type: 'touchStart', touchPoints: [{ x, y }] })
    await session.send('Input.dispatchTouchEvent', { type: 'touchMove', touchPoints: [{ x: x - 40, y }] })
    await session.send('Input.dispatchTouchEvent', { type: 'touchEnd', touchPoints: [] })
    await session.detach()
  } else {
    await page.mouse.move(x, y); await page.mouse.down(); await page.mouse.move(x - 40, y, { steps: 4 }); await page.mouse.up()
  }
  const position = Number(await page.getByLabel('Horizontal position:').inputValue())
  expect(position).toBeGreaterThan(50); expect(position).toBeLessThanOrEqual(100)
  await page.getByRole('button', { name: 'Reset crop to centered fill' }).click()
  await expect(page.getByLabel('Horizontal position:')).toHaveValue('50')
})

test('explicit multipart crop, safe 413 recovery and omitted P12 retry', async ({ page }) => {
  const bodies: string[] = []
  await page.route('**/api/sheet-render', async route => {
    bodies.push(route.request().postDataBuffer()!.toString())
    expect(route.request().headers()['x-api-key']).toBeUndefined()
    await route.fulfill({ status: 413, contentType: 'application/problem+json', body: JSON.stringify({
      type: 'https://imgengine.example/problems/resource_limit', title: 'Resource limit', status: 413,
      code: 'resource_limit', request_id: 'browser-fixture',
    }) })
  })
  const buffer = await openEditor(page)
  await page.getByLabel('Horizontal position:').focus(); await page.keyboard.press('End')
  await page.getByRole('button', { name: 'Create physical sheet' }).click()
  await expect(page.getByRole('alert')).toBeVisible()
  for (const name of ['crop_x', 'crop_y', 'crop_width', 'crop_height']) expect(bodies[0]).toContain(`name="${name}"`)
  await page.getByRole('button', { name: 'Create physical sheet' }).click()
  await expect.poll(() => bodies.length).toBe(2)
  expect(bodies[1]).not.toContain('name="crop_')
  await expect(page.getByRole('button', { name: 'Create physical sheet' })).toBeEnabled()
  await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer })
  await expect(page.getByRole('group', { name: 'Photo crop preview', exact: true })).toBeVisible()
})

test('source URLs are released on replacement, remove and mode unmount', async ({ page }) => {
  await page.addInitScript(() => {
    const active = new Set<string>()
    const create = URL.createObjectURL.bind(URL), revoke = URL.revokeObjectURL.bind(URL)
    URL.createObjectURL = blob => { const url = create(blob); active.add(url); return url }
    URL.revokeObjectURL = url => { active.delete(url); revoke(url) }
    Object.defineProperty(window, 'activeCropURLs', { get: () => active.size })
  })
  const buffer = await openEditor(page)
  const count = () => page.evaluate(() => (window as unknown as { activeCropURLs: number }).activeCropURLs)
  await expect.poll(count).toBe(1)
  await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'replacement.png', mimeType: 'image/png', buffer })
  await expect.poll(count).toBe(1)
  await page.getByRole('button', { name: 'Remove photo' }).click()
  await expect.poll(count).toBe(0)
  await page.getByLabel('Choose a JPEG or PNG photo').setInputFiles({ name: 'fixture.png', mimeType: 'image/png', buffer })
  await expect.poll(count).toBe(1)
  await page.getByRole('button', { name: 'FIT layout', exact: true }).click()
  await expect.poll(count).toBe(0)
})
