// TEST-ONLY static server. It never runs Nitro and is never deployed.
import { createServer } from 'node:http'
import { existsSync, readFileSync, realpathSync, statSync } from 'node:fs'
import { extname, join, resolve, sep } from 'node:path'
import { fileURLToPath } from 'node:url'
import { verifyFreeHeaders } from './free-security-headers.mjs'

const root = realpathSync(fileURLToPath(new URL('../.output/public/', import.meta.url)))
const providerHeaders = verifyFreeHeaders(root)
const types = {
  '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8',
  '.json': 'application/json', '.svg': 'image/svg+xml', '.png': 'image/png',
  '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.ico': 'image/x-icon',
  '.woff2': 'font/woff2', '.woff': 'font/woff', '.txt': 'text/plain; charset=utf-8',
}
const port = Number(process.env.PORT || 3100)
if (!Number.isInteger(port) || port < 1 || port > 65535) throw Error('invalid test port')

// Test-only Firefox navigation telemetry. A timeout can happen while
// Playwright waits for domcontentloaded. Record whether Node received and
// finished each HTTP request, without logging query strings, image data,
// request bodies, cookies, addresses or user-provided filenames.
const traceStaticRequests = process.env.IMGENGINE_STATIC_TEST_DIAGNOSTICS === '1'
let diagnosticRequests = 0

createServer((req, res) => {
  if (traceStaticRequests) {
    const id = ++diagnosticRequests
    const startNs = process.hrtime.bigint()
    const path = (req.url || '/').split(/[?#]/, 1)[0].slice(0, 96).replace(/[^a-zA-Z0-9_./-]/g, '_')
    console.error('[free-static-test-http] start id=' + id + ' method=' + req.method + ' path=' + path)
    const duration = () => (Number(process.hrtime.bigint() - startNs) / 1e6).toFixed(1)
    res.once('finish', () => console.error('[free-static-test-http] finish id=' + id + ' status=' + res.statusCode + ' ms=' + duration()))
    res.once('close', () => {
      if (!res.writableFinished) console.error('[free-static-test-http] incomplete id=' + id + ' status=' + res.statusCode + ' ms=' + duration())
    })
  }
  // Simulate Cloudflare Pages applying the generated _headers on static responses.
  for (const [name, value] of providerHeaders) res.setHeader(name, value)
  if (req.method !== 'GET' && req.method !== 'HEAD') {
    res.writeHead(405, { Allow: 'GET, HEAD' }); res.end(); return
  }
  let pathname
  try { pathname = decodeURIComponent(new URL(req.url || '/', 'http://localhost').pathname) }
  catch { res.writeHead(400); res.end(); return }
  if (pathname.includes('\\') || pathname.includes('\0') || pathname.split('/').includes('..') ||
      pathname.startsWith('/api/') || pathname.startsWith('/server/') || pathname === '/api' || pathname === '/_headers') {
    res.writeHead(404); res.end(); return
  }
  const rel = pathname.replace(/^\/+/, '')
  const base = resolve(root, rel)
  if (base !== root && !base.startsWith(root + sep)) { res.writeHead(404); res.end(); return }
  const candidates = pathname.endsWith('/') || !extname(pathname)
    ? [join(base, 'index.html'), base + '.html', base]
    : [base]
  const path = candidates.find(p => {
    try { return existsSync(p) && statSync(p).isFile() && realpathSync(p).startsWith(root + sep) } catch { return false }
  })
  if (!path) {
    const notFound = readFileSync(join(root, '404.html'))
    res.writeHead(404, { 'Content-Type': 'text/html; charset=utf-8' })
    if (req.method !== 'HEAD') res.end(notFound); else res.end()
    return
  }
  const bytes = readFileSync(path)
  res.writeHead(200, { 'Content-Type': types[extname(path)] || 'application/octet-stream' })
  if (req.method === 'HEAD') res.end(); else res.end(bytes)
}).listen(port, '127.0.0.1', () => console.log('Free static test server on 127.0.0.1:' + port))
