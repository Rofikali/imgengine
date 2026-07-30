export default defineNuxtConfig({
  runtimeConfig: {
    apiBase: process.env.NUXT_API_BASE ?? 'http://localhost:8000',
    apiKey: process.env.NUXT_API_KEY ?? '',
  },
  devtools: { enabled: true },
  typescript: { strict: true },
})
