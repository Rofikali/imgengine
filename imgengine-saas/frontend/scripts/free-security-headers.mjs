import { createHash } from 'node:crypto'
import { readdirSync, readFileSync, statSync, existsSync, writeFileSync, lstatSync } from 'node:fs'
import { extname, join, relative, resolve, sep } from 'node:path'
import { fileURLToPath, pathToFileURL } from 'node:url'

// F5-HOST-HDR-01 — Cloudflare Pages _headers for STATIC ASSETS only.
// Strictly no Functions, Workers, runtime API, domain or deploy configured here.
const REQUIRED_HEADER_NAMES = Object.freeze([
  'Content-Security-Policy',
  'X-Content-Type-Options',
  'X-Frame-Options',
  'Referrer-Policy',
  'Permissions-Policy',
  'Cross-Origin-Opener-Policy',
  'Cross-Origin-Resource-Policy',
  'X-Robots-Tag',
])
const MAX_LINE = 2000

function staticallyRenderedHtml(root) {
  const files = []
  function walk(path) {
    if (lstatSync(path).isSymbolicLink()) throw Error('symlink is forbidden in static release: ' + relative(root, path))
    for (const entry of readdirSync(path, { withFileTypes: true })) {
      const current = join(path, entry.name)
      if (entry.isDirectory()) walk(current)
      else if (entry.isFile() && extname(entry.name) === '.html') files.push(current)
    }
  }
  walk(root)
  if (!files.length || !files.some(x => relative(root, x) === 'index.html')) {
    throw Error('cannot generate secure headers without pre-rendered index.html')
  }
  return files.sort()
}

export function buildFreeHeaders(dist) {
  const root = resolve(dist)
  const inline = new Set()
  for (const htmlFile of staticallyRenderedHtml(root)) {
    const html = readFileSync(htmlFile, 'utf8')
    for (const match of html.matchAll(/<script\b([^>]*)>([\s\S]*?)<\/script\s*>/gi)) {
      const attrs = match[1]
      const body = match[2]
      const type = attrs.match(/\btype\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+))/i)
      const contentType = (type?.[1] ?? type?.[2] ?? type?.[3] ?? '').toLowerCase()
      const src = attrs.match(/\bsrc\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+))/i)
      const value = src ? src[1] ?? src[2] ?? src[3] : null
      if (contentType === 'application/json' || contentType === 'application/ld+json') {
        if (value !== null) {
          // Nuxt's prerendered route data is a non-executable JSON data block,
          // but accept only the exact existing payload adjacent to this HTML route.
          const routeDir = relative(root, htmlFile).split(sep).slice(0, -1).join('/')
          const expected = '/' + (routeDir ? routeDir + '/' : '') + '_payload.json'
          const payloadPath = join(root, expected.slice(1))
          // Nuxt adds a UUIDv4 cache buster on its route-local JSON payloads.
          // Accept only the exact pathname with this one proven suffix.
          const suffixPrefix = expected + '?_b='
          const cacheBuster = value.startsWith(suffixPrefix) ? value.slice(suffixPrefix.length) : ''
          const validCacheBuster = /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(cacheBuster)
          const isExpectedPayload = value === expected || validCacheBuster
          if (contentType !== 'application/json' || !isExpectedPayload) {
            throw Error('unapproved JSON script payload path/type in ' + relative(root, htmlFile) +
              ': expected ' + expected + ', received ' + JSON.stringify(value.slice(0, 240)) + ', length ' + value.length)
          }
          if (!existsSync(payloadPath) || lstatSync(payloadPath).isSymbolicLink() || !statSync(payloadPath).isFile()) {
            throw Error('missing or unsafe route-local JSON payload in ' + relative(root, htmlFile))
          }
          // A non-executable JSON data block can contain inline serialized state
          // even with a src attribute. Require valid JSON, never execute or hash it.
          if (body.trim()) {
            try { JSON.parse(body) }
            catch { throw Error('invalid inline route JSON payload in ' + relative(root, htmlFile)) }
          }
        }
        continue
      }
      if (value !== null) {
        // Executable scripts must refer only to compiled same-origin Nuxt JS.
        if (!/^\/_nuxt\/[a-z0-9_.\/-]+\.m?js(?:\?[a-z0-9_-]+)?$/i.test(value) || value.includes('..')) {
          throw Error('external/unapproved script in ' + relative(root, htmlFile) + ': src pathname ' + JSON.stringify(value.split(/[?#]/, 1)[0].slice(0, 160)))
        }
        continue
      }
      // Import maps influence module resolution and must receive an exact CSP hash.
      if (!['', 'module', 'text/javascript', 'application/javascript', 'importmap'].includes(contentType)) {
        throw Error('unknown inline script type: ' + contentType)
      }
      if (body.trim()) inline.add("'sha256-" + createHash('sha256').update(body, 'utf8').digest('base64') + "'")
    }
  }
  const scriptSrc = ["'self'", ...[...inline].sort()].join(' ')
  const csp = [
    "default-src 'none'",
    'script-src ' + scriptSrc,
    "style-src 'self' 'unsafe-inline'",
    "connect-src 'self'",
    "img-src 'self' data: blob:",
    "font-src 'self' data:",
    "worker-src 'self' blob:",
    "object-src 'none'",
    "frame-src 'none'",
    "frame-ancestors 'none'",
    "base-uri 'none'",
    "form-action 'none'",
  ].join('; ')
  const rules = {
    'Content-Security-Policy': csp,
    'X-Content-Type-Options': 'nosniff',
    'X-Frame-Options': 'DENY',
    'Referrer-Policy': 'no-referrer',
    'Permissions-Policy': 'camera=(), microphone=(), geolocation=()',
    'Cross-Origin-Opener-Policy': 'same-origin',
    'Cross-Origin-Resource-Policy': 'same-origin',
    // Draft privacy/terms/support pages are explicitly not public-ready.
    // Remove noindex only after separate owner-approved production review.
    'X-Robots-Tag': 'noindex, nofollow',
  }
  const output = ['/*', ...Object.entries(rules).map(([name, value]) => '  ' + name + ': ' + value), ''].join('\n')
  for (const line of output.split('\n')) if (line.length > MAX_LINE) {
    throw Error('Cloudflare Pages _headers line > 2000 characters; use verified per-route CSP hash policy')
  }
  return output
}

export function parseFreeHeaders(text) {
  const lines = text.split(/\r?\n/)
  if (lines[0] !== '/*') throw Error('Free static headers must use one /* rule')
  const map = new Map()
  for (const line of lines.slice(1)) {
    if (!line) continue
    if (line.length > MAX_LINE) throw Error('Cloudflare _headers line exceeds 2000 characters')
    const match = line.match(/^  ([A-Za-z-]+): (.+)$/)
    if (!match || map.has(match[1])) throw Error('malformed/duplicate Free _headers directive: ' + line)
    map.set(match[1], match[2])
  }
  if (map.size !== REQUIRED_HEADER_NAMES.length || REQUIRED_HEADER_NAMES.some(name => !map.has(name))) {
    throw Error('Free _headers missing a required directive or contains unreviewed headers')
  }
  const csp = map.get('Content-Security-Policy')
  if (!csp.includes("script-src 'self'") || /script-src[^;]*'unsafe-inline'|script-src[^;]*'unsafe-eval'/.test(csp)) {
    throw Error('unsafe inline/eval script CSP or missing self restriction')
  }
  return map
}

export function verifyFreeHeaders(dist) {
  const root = resolve(dist)
  const path = join(root, '_headers')
  if (!existsSync(path)) throw Error('Free static artifact lacks Cloudflare _headers')
  if (lstatSync(path).isSymbolicLink() || !statSync(path).isFile()) throw Error('Free _headers must be a regular file')
  const actual = readFileSync(path, 'utf8')
  const expected = buildFreeHeaders(root)
  if (actual !== expected) throw Error('Cloudflare _headers do not match exactly the compiled HTML security policy')
  return parseFreeHeaders(actual)
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  const root = resolve(process.argv[2] ?? fileURLToPath(new URL('../.output/public/', import.meta.url)))
  try {
    const content = buildFreeHeaders(root)
    parseFreeHeaders(content)
    writeFileSync(join(root, '_headers'), content, { flag: 'w', mode: 0o644 })
    console.log('Generated approved Free static _headers from compiled HTML hashes')
  } catch (error) {
    console.error('::error::Free header generation failed closed: ' + error.message)
    process.exitCode = 1
  }
}
