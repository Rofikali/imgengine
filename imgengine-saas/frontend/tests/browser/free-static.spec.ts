import { expect, test } from '@playwright/test'

const freeTools = [
  ['Resize locally — no upload', '/resize-image', 'Resize an image'],
  ['Crop locally — no upload', '/crop-image', 'Crop an image'],
  ['Compress locally — no upload', '/compress-image', 'Compress an image'],
  ['Rotate locally — no upload', '/rotate-image', 'Rotate an image'],
  ['Convert locally — no upload', '/convert-image', 'Convert an image'],
] as const


const freeRoutes = ['/', ...freeTools.map(item => item[1]), '/privacy', '/terms', '/support'] as const

// Independent URLs are independent tests: each route receives its own 30 s
// navigation budget instead of competing for one 30 s aggregate loop budget.
for (const route of freeRoutes) {
  test(`Free route ${route} is genuine static HTML with no server UI`, async ({ page, request }) => {
    const response = await request.get(route, { maxRedirects: 0 })
    expect(response.status(), route).toBe(200)
    expect(response.headers()['content-type']).toContain('text/html')
    await page.goto(route, { waitUntil: 'domcontentloaded' })
    await expect(page.getByRole('main')).toBeVisible()
    await expect(page.getByText('Server print layouts (uploads on Render)')).toHaveCount(0)
    await expect(page.getByText('Create printable photo layouts.')).toHaveCount(0)
  })
}

test('Free landing page links to exactly five local tools', async ({ page }) => {
  test.slow() // One deliberate SPA journey through all five tools.
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
// Each independently sharded route gets a fresh browser/server lifetime in
// Firefox. CI demonstrated that even separate Pages in one six-route test
// sometimes stall on the fifth/sixth full-load navigation despite 200 responses.
// Do not weaken the privacy assertions or the load event to hide this failure.
const privacyRoutes = ['/', ...freeTools.map(([, path]) => path)] as const
for (const route of privacyRoutes) {
  test(`Free privacy audit on ${route}: no image, filename or server-bound requests`, async ({ context }) => {
    const requests: string[] = []
    context.on('request', request => {
      if (!request.url().startsWith('http')) return
      const url = new URL(request.url())
      if (url.origin !== 'http://127.0.0.1:3100' ||
          request.method() !== 'GET' ||
          request.postData() ||
          url.pathname.startsWith('/api/')) requests.push(request.url())
    })

    const page = await context.newPage()
    try {
      // Check full load on the committed document using its own navigation timing.
      // loadEventEnd is positive only after its load handlers have finished.
      await page.goto(route, { waitUntil: 'commit' })
      await page.waitForFunction(() => {
        const [navigation] = performance.getEntriesByType('navigation') as PerformanceNavigationTiming[]
        return navigation && navigation.loadEventEnd > 0
      })
      await expect(page.getByRole('main')).toBeVisible()
      expect(await page.evaluate(async () => [
        localStorage.length, sessionStorage.length, (await indexedDB.databases()).length,
      ]), 'No persistent browser storage on ' + route).toEqual([0, 0, 0])
      expect(await context.cookies(), 'No cookies on ' + route).toEqual([])
      expect(requests, 'No outbound, upload or server-bound requests on ' + route).toEqual([])
    } finally {
      await page.close()
    }
  })
}


for (const [route, heading] of [
  ['/privacy', 'Privacy notice'],
  ['/terms', 'Terms of use'],
  ['/support', 'Support and contact'],
] as const) {
  test(`Free legal notice ${route} remains visibly draft`, async ({ page, request }) => {
    const response = await request.get(route, { maxRedirects: 0 })
    expect(response.status(), route).toBe(200)
    await page.goto(route, { waitUntil: 'domcontentloaded' })
    await expect(page.getByRole('heading', { name: heading })).toBeVisible()
    await expect(page.getByText('IMGENGINE · FREE · PRE-LAUNCH DRAFT')).toBeVisible()
    await expect(page.getByRole('navigation', { name: 'Privacy, terms and support' })).toBeVisible()
    await expect(page.getByRole('heading', { name: 'Create printable photo layouts.' })).toHaveCount(0)
    await expect(page.getByText('Server print layouts (uploads on Render)')).toHaveCount(0)
    const links = page.getByRole('navigation', { name: 'Privacy, terms and support' })
    for (const [name, href] of [['Privacy', '/privacy'], ['Terms', '/terms'], ['Support', '/support']] as const) {
      await expect(links.getByRole('link', { name })).toHaveAttribute('href', href)
    }
  })
}

for (const [, route] of freeTools) {
  test(`Free tool ${route} links to legal notices without uploading`, async ({ page }) => {
    await page.goto(route, { waitUntil: 'domcontentloaded' })
    const nav = page.getByRole('navigation', { name: 'Privacy, terms and support' })
    await expect(nav.getByRole('link')).toHaveCount(3)
    await expect(nav.getByRole('link', { name: 'Privacy' })).toHaveAttribute('href', '/privacy')
  })
}

for (const route of freeRoutes) {
  test(`Free route ${route} enforces CSP and Cloudflare response headers`, async ({ page, request }) => {
    const response = await request.get(route, { maxRedirects: 0 })
    expect(response.status(), route).toBe(200)
    const headers = response.headers()
    const csp = headers['content-security-policy']
    expect(csp, route).toContain("default-src 'none'")
    expect(csp, route).toContain("script-src 'self'")
    expect(csp, route).toContain("connect-src 'self' blob:")
    expect(csp, route).toContain("frame-ancestors 'none'")
    expect(csp, route).not.toContain('unsafe-eval')
    const scriptDirective = csp?.split('script-src ')[1]?.split(';')[0] ?? ''
    expect(scriptDirective).not.toContain('unsafe-inline')
    expect(headers['x-content-type-options']).toBe('nosniff')
    expect(headers['x-frame-options']).toBe('DENY')
    expect(headers['referrer-policy']).toBe('no-referrer')
    expect(headers['x-robots-tag']).toContain('noindex')
    await page.addInitScript(() => {
      const store = window as Window & { __imgengineCspViolations?: string[] }
      store.__imgengineCspViolations = []
      document.addEventListener('securitypolicyviolation', event => {
        store.__imgengineCspViolations?.push(event.violatedDirective + ': ' + event.blockedURI)
      })
    })
    await page.goto(route) // Full load intentionally required for CSP observation.
    await expect(page.getByRole('main')).toBeVisible()
    await expect(page.locator('link[rel~="prefetch"]')).toHaveCount(0)
    const violations = await page.evaluate(() => (window as Window & { __imgengineCspViolations?: string[] }).__imgengineCspViolations ?? [])
    expect(violations, route).toEqual([])
    expect((await request.get('/_headers')).status()).toBe(404)
  })
}
