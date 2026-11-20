// TEST-ONLY static server. It never runs Nitro and is never deployed.
import { createServer } from 'node:http'
import { existsSync, readFileSync, realpathSync, statSync } from 'node:fs'
import { extname, join, resolve, sep } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = realpathSync(fileURLToPath(new URL('../.output/public/', import.meta.url)))
const types = {
  '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8',
  '.json': 'application/json', '.svg': 'image/svg+xml', '.png': 'image/png',
  '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.ico': 'image/x-icon',
  '.woff2': 'font/woff2', '.woff': 'font/woff', '.txt': 'text/plain; charset=utf-8',
}
const port = Number(process.env.PORT || 3100)
if (!Number.isInteger(port) || port < 1 || port > 65535) throw Error('invalid test port')

createServer((req, res) => {
  res.setHeader('X-Content-Type-Options', 'nosniff')
  if (req.method !== 'GET' && req.method !== 'HEAD') {
    res.writeHead(405, { Allow: 'GET, HEAD' }); res.end(); return
  }
  let pathname
  try { pathname = decodeURIComponent(new URL(req.url || '/', 'http://localhost').pathname) }
  catch { res.writeHead(400); res.end(); return }
  if (pathname.includes('\\') || pathname.includes('\0') || pathname.split('/').includes('..') ||
      pathname.startsWith('/api/') || pathname.startsWith('/server/') || pathname === '/api') {
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
