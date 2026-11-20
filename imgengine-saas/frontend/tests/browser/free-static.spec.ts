import { expect, test } from '@playwright/test'

const freeTools = [
  ['Resize locally — no upload', '/resize-image', 'Resize an image'],
  ['Crop locally — no upload', '/crop-image', 'Crop an image'],
  ['Compress locally — no upload', '/compress-image', 'Compress an image'],
  ['Rotate locally — no upload', '/rotate-image', 'Rotate an image'],
  ['Convert locally — no upload', '/convert-image', 'Convert an image'],
] as const

test('all six Free routes are genuine static HTML, with no server layout offered', async ({ page, request }) => {
  for (const route of ['/', ...freeTools.map(x => x[1])]) {
    const response = await request.get(route, { maxRedirects: 0 })
    expect(response.status(), route).toBe(200)
    expect(response.headers()['content-type']).toContain('text/html')
    await page.goto(route)
    await expect(page.getByRole('main')).toBeVisible()
    await expect(page.getByText('Server print layouts (uploads on Render)')).toHaveCount(0)
    await expect(page.getByText('Create printable photo layouts.')).toHaveCount(0)
  }
})
test('Free landing page links to exactly five local tools', async ({ page }) => {
  await page.goto('/')
  await expect(page.getByRole('heading', { name: 'Edit images privately.' })).toBeVisible()
  const nav = page.getByRole('navigation', { name: 'Free image tools' })
  await expect(nav.getByRole('link')).toHaveCount(5)
  for (const [name, path, heading] of freeTools) {
    await expect(nav.getByRole('link', { name, exact: true })).toHaveAttribute('href', path)
    await nav.getByRole('link', { name, exact: true }).click()
    await expect(page.getByRole('heading', { name: heading })).toBeVisible()
    await page.goto('/')
  }
})
test('forbidden dynamic endpoints are true HTTP 404, never SPA fallback success', async ({ request }) => {
  for (const endpoint of ['/api', '/api/render', '/api/layout-render', '/api/sheet-render', '/api/v2/sheet-render', '/server/api/render', '/does-not-exist']) {
    const response = await request.get(endpoint, { maxRedirects: 0 })
    expect(response.status(), endpoint).toBe(404)
    expect((await response.text()).slice(0, 300)).not.toContain('Create printable photo layouts.')
  }
})
test('static Free browsing sends no image, filename, or server-bound requests', async ({ page }) => {
  const requests: string[] = []
  page.on('request', request => {
    if (!request.url().startsWith('http')) return
    const url = new URL(request.url())
    if (url.origin !== 'http://127.0.0.1:3100' || request.method() !== 'GET' || request.postData() || url.pathname.startsWith('/api/')) requests.push(request.url())
  })
  await page.goto('/')
  for (const [, path] of freeTools) await page.goto(path)
  expect(requests).toEqual([])
  expect(await page.evaluate(async () => [localStorage.length, sessionStorage.length, (await indexedDB.databases()).length])).toEqual([0, 0, 0])
})
