import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, rmSync } from 'node:fs'
import { join } from 'node:path'
import { tmpdir } from 'node:os'
import { createHash } from 'node:crypto'
import { buildFreeHeaders, parseFreeHeaders, verifyFreeHeaders } from './free-security-headers.mjs'

function fixture(html, fn) {
  const folder = mkdtempSync(join(tmpdir(), 'imgengine-cf-headers-'))
  try {
    writeFileSync(join(folder, 'index.html'), html)
    const generated = buildFreeHeaders(folder)
    writeFileSync(join(folder, '_headers'), generated)
    fn(folder, generated)
  } finally { rmSync(folder, { force: true, recursive: true }) }
}

test('uses exact SHA-256 of executable inline scripts; ignores JSON data scripts', () => {
  const inline='window.__NUXT__ = { hydrated: true }'
  fixture('<script type="application/json">{"safe": true}</script><script>' + inline + '</script><script type="module" src="/_nuxt/entry.A1b.js"></script>', (root, headers) => {
    const digest=createHash('sha256').update(inline).digest('base64')
    assert.match(headers, new RegExp('sha256-' + digest.replace(/[+]/g, '\\+')))
    const scriptPolicy = parseFreeHeaders(headers).get('Content-Security-Policy').split('; ').find(part => part.startsWith('script-src '))
    assert.ok(scriptPolicy, 'script-src must exist')
    assert.doesNotMatch(scriptPolicy, /unsafe-eval|unsafe-inline/)
    assert.match(headers, /connect-src 'self'/)
    assert.match(headers, /frame-ancestors 'none'/)
    assert.match(headers, /X-Robots-Tag: noindex, nofollow/)
    assert.equal(verifyFreeHeaders(root).get('X-Content-Type-Options'), 'nosniff')
    assert.equal(buildFreeHeaders(root), headers)
  })
})

test('rejects a changed executable script without regenerating CSP hashes', () => {
  fixture('<script>window.__NUXT__=1</script>', (root) => {
    writeFileSync(join(root, 'index.html'), '<script>window.__NUXT__=2</script>')
    assert.throws(() => verifyFreeHeaders(root), /do not match exactly/)
  })
})

test('rejects injection of extra rule or duplicate response header', () => {
  fixture('<h1>SAFE</h1>', (root, headers) => {
    writeFileSync(join(root, '_headers'), headers + '/api/*\n  Access-Control-Allow-Origin: *\n')
    assert.throws(() => verifyFreeHeaders(root), /do not match exactly/)
    assert.throws(() => parseFreeHeaders('/*\n  X-Frame-Options: DENY\n  X-Frame-Options: SAMEORIGIN\n'), /duplicate/)
  })
})

test('rejects external executable scripts in compiled HTML', () => {
  fixture('<h1>SAFE</h1>', (root) => {
    writeFileSync(join(root, 'index.html'), '<script src="https://cdn.example.com/script.js"></script>')
    assert.throws(() => buildFreeHeaders(root), /external\/unapproved script/)
  })
})

test('rejects Cloudflare _headers lines that exceed vendor line limit', () => {
  fixture('<h1>SAFE</h1>', (root) => {
    writeFileSync(join(root, 'index.html'), Array.from({length: 60}, (_,i)=>'<script>window.a'+i+'='+i+'</script>').join(''))
    assert.throws(() => buildFreeHeaders(root), /line > 2000 characters/)
  })
})

test('inline Nuxt import maps have exact CSP SHA-256 and cannot be silently changed', () => {
  const importMap = '{"imports":{"/_nuxt/entry.js":"/_nuxt/entry.js"}}'
  fixture('<script type="importmap">' + importMap + '</script>', (root, headers) => {
    const digest = createHash('sha256').update(importMap).digest('base64')
    assert.ok(headers.includes("'sha256-" + digest + "'"))
    writeFileSync(join(root, 'index.html'), '<script type="importmap">{"imports":{}}</script>')
    assert.throws(() => verifyFreeHeaders(root), /do not match exactly/)
  })
})

test('Nuxt data scripts accept only existing route-local JSON payloads, never executable URLs', () => {
  fixture('<h1>SAFE</h1>', root => {
    const routeDir = join(root, 'compress-image')
    mkdirSync(routeDir)
    const payload = join(routeDir, '_payload.json')
    const html = join(routeDir, 'index.html')
    writeFileSync(payload, '{"rendered":true}')
    writeFileSync(html, '<script type="application/json" data-nuxt-data src="/compress-image/_payload.json"></script>')
    assert.doesNotThrow(() => buildFreeHeaders(root))
    writeFileSync(html, '<script src="/compress-image/_payload.json"></script>')
    assert.throws(() => buildFreeHeaders(root), /external\/unapproved script/)
    writeFileSync(html, '<script type="application/json" src="/privacy/_payload.json"></script>')
    assert.throws(() => buildFreeHeaders(root), /unapproved JSON script payload/)
    writeFileSync(html, '<script type="application/json" src="https://cdn.example.com/data.json"></script>')
    assert.throws(() => buildFreeHeaders(root), /unapproved JSON script payload/)
    rmSync(payload)
    writeFileSync(html, '<script type="application/json" src="/compress-image/_payload.json"></script>')
    assert.throws(() => buildFreeHeaders(root), /unapproved JSON script payload/)
  })
})
