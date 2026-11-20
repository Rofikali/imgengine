import { fileURLToPath } from 'node:url'

// F5-STATIC-01: build-time isolation. Never use this flag to authorize upload.
const freeStatic = process.env.IMGENGINE_FREE_STATIC_BUILD === '1'
const freePaths = ['/', '/resize-image', '/crop-image', '/compress-image', '/rotate-image', '/convert-image']
const allowedFreePaths = new Set(freePaths)

export default defineNuxtConfig({
  // The ordinary server-backed P12/Studio code and routes remain in Git.
  // For Free, Nitro must NOT discover the normal server/api directory.
  serverDir: freeStatic ? fileURLToPath(new URL('./free-static/empty-server/', import.meta.url)) : undefined,
  hooks: freeStatic ? {
    'pages:extend'(pages) {
      const unknown = pages.filter(page => !allowedFreePaths.has(page.path))
      if (unknown.length || pages.length !== freePaths.length) {
        throw new Error('Free static build encountered unexpected or missing routes: ' + pages.map(p => p.path).join(', '))
      }
      const home = pages.find(page => page.path === '/')
      if (!home) throw new Error('Free static homepage is missing')
      home.file = fileURLToPath(new URL('./free-static/landing.vue', import.meta.url))
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
    // The Rust render credential is held only by the Nuxt server proxies
    // for P5, P8, and P12. Never inject a value in the Free static build.
    imgengineApiBase: freeStatic ? '' : process.env.IMGENGINE_API_BASE ?? 'http://localhost:8000',
    imgengineRenderApiKey: freeStatic ? '' : process.env.IMGENGINE_RENDER_API_KEY ?? '',
  },
  devtools: { enabled: !freeStatic },
  typescript: { strict: true },
})
