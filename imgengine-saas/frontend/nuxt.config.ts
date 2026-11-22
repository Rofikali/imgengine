import { fileURLToPath } from 'node:url'

// F5-STATIC-01: build-time isolation. Never use this flag to authorize upload.
const freeStatic = process.env.IMGENGINE_FREE_STATIC_BUILD === '1'
const freeToolPaths = ['/', '/resize-image', '/crop-image', '/compress-image', '/rotate-image', '/convert-image']
const freeNoticePaths = ['/privacy', '/terms', '/support']
const freePaths = [...freeToolPaths, ...freeNoticePaths]
const allowedFreePaths = new Set(freePaths)

export default defineNuxtConfig({
  // The ordinary server-backed P12/Studio code and routes remain in Git.
  // For Free, Nitro must NOT discover the normal server/api directory.
  serverDir: freeStatic ? fileURLToPath(new URL('./free-static/empty-server/', import.meta.url)) : undefined,
  hooks: freeStatic ? {
    'pages:extend'(pages) {
      const unknown = pages.filter(page => !allowedFreePaths.has(page.path))
      if (unknown.length || pages.length !== freeToolPaths.length) {
        throw new Error('Free static build encountered unexpected or missing routes: ' + pages.map(p => p.path).join(', '))
      }
      const home = pages.find(page => page.path === '/')
      if (!home) throw new Error('Free static homepage is missing')
      home.file = fileURLToPath(new URL('./free-static/landing.vue', import.meta.url))
      for (const route of freeNoticePaths) {
        pages.push({
          name: 'free-' + route.slice(1),
          path: route,
          file: fileURLToPath(new URL('./free-static/' + route.slice(1) + '.vue', import.meta.url)),
        })
      }
    },
  } : {},
  nitro: freeStatic ? {
    prerender: {
      crawlLinks: false,
      routes: freePaths,
      failOnError: true,
    },
  } : {},
  runtimeConfig: {
    public: { imgengineFreeStatic: freeStatic },
    // The Rust render credential is held only by the Nuxt server proxies
    // for P5, P8, and P12. Never inject a value in the Free static build.
    imgengineApiBase: freeStatic ? '' : process.env.IMGENGINE_API_BASE ?? 'http://localhost:8000',
    imgengineRenderApiKey: freeStatic ? '' : process.env.IMGENGINE_RENDER_API_KEY ?? '',
  },
  // Free-only CSP retains default-src 'none'; avoid speculative NuxtLink
  // asset prefetch (which may fall back to default-src across browsers).
  // Normal server-backed Nuxt behavior remains unchanged.
  experimental: freeStatic ? { defaults: { nuxtLink: { prefetch: false } } } : {},
  vite: freeStatic ? { build: { license: { fileName: '_nuxt/third-party-licenses.txt' } } } : {},
  devtools: { enabled: !freeStatic },
  typescript: { strict: true },
})
