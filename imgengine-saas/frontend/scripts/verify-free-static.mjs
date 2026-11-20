import { createHash } from 'node:crypto'
import { existsSync, lstatSync, readFileSync, readdirSync, statSync } from 'node:fs'
import { basename, extname, join, relative, resolve, sep } from 'node:path'
import { fileURLToPath, pathToFileURL } from 'node:url'

export const FREE_ROUTES = Object.freeze(['/', '/resize-image', '/crop-image', '/compress-image', '/rotate-image', '/convert-image', '/privacy', '/terms', '/support'])
const MAX_FILES = 20000
const MAX_FILE_BYTES = 25 * 1024 * 1024
const allowed = new Set(['.html', '.js', '.mjs', '.css', '.json', '.png', '.jpg', '.jpeg', '.webp', '.svg', '.ico', '.woff', '.woff2', '.txt'])
const forbiddenSegments = new Set(['server', 'functions', 'node_modules', 'backend', '.git', '.nuxt', '.env', 'api', 'secrets', 'private'])
const forbiddenNames = new Set(['_worker.js', '_routes.json', '200.html', 'wrangler.toml', 'package.json', 'package-lock.json', 'pnpm-lock.yaml'])
const forbiddenContent = [
  /IMGENGINE_RENDER_API_KEY|IMGENGINE_API_KEYS|INTERNAL_API_TOKEN|WEB_API_KEY/,
  /Server print layouts \(uploads on Render\)/,
  /Create printable photo layouts\./,
  /\/api\/(?:render|layout-render|sheet-render|v2\/sheet-render)/,
  /https?:\/\/(?:localhost|127\.0\.0\.1):8000/,
]

function walk(root) {
  const files = []
  function visit(path) {
    if (lstatSync(path).isSymbolicLink()) throw Error('Symlink in static output: ' + relative(root, path))
    const st = statSync(path)
    if (st.isDirectory()) {
      for (const name of readdirSync(path).sort()) visit(join(path, name))
      return
    }
    if (!st.isFile()) throw Error('Non-file static output: ' + relative(root, path))
    files.push(path)
    if (files.length > MAX_FILES) throw Error('Cloudflare Pages static file count exceeds limit')
  }
  visit(root)
  return files
}

export function auditFreeStatic(root) {
  const dist = resolve(root)
  if (!existsSync(dist) || !lstatSync(dist).isDirectory()) throw Error('Missing Free static output directory')
  if (lstatSync(dist).isSymbolicLink()) throw Error('Static output root is a symlink')
  const files = walk(dist)
  const names = new Set(files.map(p => relative(dist, p).split(sep).join('/')))
  for (const route of FREE_ROUTES) {
    const candidates = route === '/' ? ['index.html'] : [route.slice(1) + '/index.html', route.slice(1) + '.html']
    if (!candidates.some(name => names.has(name))) throw Error('Missing pre-rendered Free route: ' + route)
  }
  if (!names.has('404.html')) throw Error('Missing real 404.html: cannot ship fallback-only SPA')
  if (names.has('200.html')) throw Error('SPA catchall 200.html is forbidden for Free-only static hosting')

  const records = []
  let total = 0
  for (const path of files) {
    const name = relative(dist, path).split(sep).join('/')
    const segments = name.split('/')
    if (segments.some(part => part.startsWith('.') || forbiddenSegments.has(part))) throw Error('Forbidden path in Free artifact: ' + name)
    if (segments.some(part => forbiddenNames.has(part))) throw Error('Forbidden executable/fallback in Free artifact: ' + name)
    if (extname(name).toLowerCase() === '.map') throw Error('Source map requires separate approval: ' + name)
    if (!allowed.has(extname(name).toLowerCase())) throw Error('Unapproved file type in Free artifact: ' + name)
    const data = readFileSync(path)
    if (data.length > MAX_FILE_BYTES) throw Error('Cloudflare Pages per-file limit exceeded: ' + name)
    total += data.length
    if (/\.(?:html|js|mjs|css|json|svg|txt)$/i.test(name)) {
      const text = data.toString('utf8')
      for (const rule of forbiddenContent) {
        if (rule.test(text)) throw Error('Server/secret boundary violation (' + rule.source + ') in ' + name)
      }
    }
    records.push(name + '\0' + createHash('sha256').update(data).digest('hex') + '\0')
  }
  const digest = createHash('sha256').update(records.join('')).digest('hex')
  return { fileCount: files.length, totalBytes: total, staticTreeSha256: digest, routes: FREE_ROUTES }
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  const root = process.argv[2] || fileURLToPath(new URL('../.output/public/', import.meta.url))
  try {
    console.log(JSON.stringify({ status: 'PASS', ...auditFreeStatic(root) }, null, 2))
  } catch (e) {
    console.error('::error::Free static artifact rejected: ' + e.message)
    process.exitCode = 1
  }
}
