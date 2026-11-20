import { existsSync, rmSync } from 'node:fs'
import { fileURLToPath } from 'node:url'

const folder = fileURLToPath(new URL('../.output/public/', import.meta.url))
if (!existsSync(folder)) throw Error('Nuxt did not generate .output/public')
const spaFallback = fileURLToPath(new URL('../.output/public/200.html', import.meta.url))
// Nuxt can emit this fallback. Cloudflare must return a genuine 404 for
// missing /api URLs, not silently serve the Vue router with HTTP 200.
if (existsSync(spaFallback)) {
  rmSync(spaFallback)
  console.log('Removed generated SPA 200.html fallback')
}
if (!existsSync(fileURLToPath(new URL('../.output/public/404.html', import.meta.url)))) {
  throw Error('Free static build requires an explicit 404.html')
}
