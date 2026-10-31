export default defineNuxtConfig({
  runtimeConfig: {
    // The Rust render credential is held only by the Nuxt server proxies
    // for P5, P8, and P12. A browser must never receive IMGENGINE_API_KEYS
    // or this single active key.
    imgengineApiBase: process.env.IMGENGINE_API_BASE ?? 'http://localhost:8000',
    imgengineRenderApiKey: process.env.IMGENGINE_RENDER_API_KEY ?? '',
  },
  devtools: { enabled: true },
  typescript: { strict: true },
})
