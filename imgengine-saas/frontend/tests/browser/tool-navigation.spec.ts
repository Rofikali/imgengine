import { expect, test } from '@playwright/test'

const freeTools = [
  ['Resize locally — no upload', '/resize-image'],
  ['Crop locally — no upload', '/crop-image'],
  ['Compress locally — no upload', '/compress-image'],
  ['Rotate locally — no upload', '/rotate-image'],
  ['Convert locally — no upload', '/convert-image'],
] as const

test('the landing page exposes every browser-local Free tool in a labelled navigation region', async ({ page }) => {
  // This test verifies the navigable DOM and link destinations, not the
  // resource-load lifecycle. Keep full-load checks in Free privacy/CSP tests.
  await page.goto('/', { waitUntil: 'domcontentloaded' })

  const navigation = page.getByRole('navigation', { name: 'Free image tools' })
  await expect(navigation).toBeVisible()

  for (const [name, path] of freeTools) {
    await expect(navigation.getByRole('link', { name, exact: true })).toHaveAttribute('href', path)
  }

  await expect(page.getByRole('heading', { name: process.env.IMGENGINE_STATIC_E2E === '1' ? 'Edit images privately.' : 'Create printable photo layouts.' })).toBeVisible()
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= window.innerWidth)).toBe(true)
})
