import { existsSync, readFileSync, rmSync, writeFileSync } from 'node:fs'
import { buildFreeHeaders, parseFreeHeaders, stripFreeSpeculativePrefetch, assertNoFreeSpeculativePrefetch } from './free-security-headers.mjs'
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

const licenseFile = new URL('../.output/public/_nuxt/third-party-licenses.txt', import.meta.url)
if (!/^## .+/m.test(readFileSync(licenseFile, 'utf8'))) throw Error('Free bundle license report is empty')

const strippedPrefetch = stripFreeSpeculativePrefetch(folder)
assertNoFreeSpeculativePrefetch(folder)
console.log('Removed ' + strippedPrefetch + ' Nuxt speculative prefetch hints from Free HTML')
const headers = buildFreeHeaders(folder)
parseFreeHeaders(headers)
writeFileSync(fileURLToPath(new URL('../.output/public/_headers', import.meta.url)), headers, { mode: 0o644 })
console.log('Free static security headers generated from compiled HTML')
