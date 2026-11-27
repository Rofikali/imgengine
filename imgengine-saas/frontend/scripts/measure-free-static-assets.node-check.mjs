import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, writeFileSync, symlinkSync, rmSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { gzipSync, brotliCompressSync, constants } from 'node:zlib'
import { FREE_ROUTES, auditFreeStatic } from './verify-free-static.mjs'
import { buildFreeHeaders } from './free-security-headers.mjs'
import { inventoryAssetSizes, measureAssetBytes, measureFreeStatic } from './measure-free-static-assets.mjs'

function fixture(fn) {
  const root = mkdtempSync(join(tmpdir(), 'imgengine-size-fixture-'))
  try { fn(root) } finally { rmSync(root, { recursive: true, force: true }) }
}

test('raw and fixed independent compression sizes are exact', () => {
  const data = Buffer.from('body { color: red; }\n'.repeat(100))
  assert.deepEqual(measureAssetBytes(data), {
    raw: data.length,
    gzip: gzipSync(data, { level: 6 }).length,
    brotli: brotliCompressSync(data, { params: { [constants.BROTLI_PARAM_QUALITY]: 5 } }).length,
  })
  assert.throws(() => measureAssetBytes('not a buffer'))
})

test('inventory sums unique CSS and JS assets, excludes HTML', () => fixture(root => {
  mkdirSync(join(root, '_nuxt'))
  writeFileSync(join(root, '_nuxt', 'a.css'), 'body{}')
  writeFileSync(join(root, '_nuxt', 'b.js'), 'const a=1')
  writeFileSync(join(root, '_nuxt', 'c.mjs'), 'export{}')
  writeFileSync(join(root, 'index.html'), '<html></html>')
  const result = inventoryAssetSizes(root)
  assert.deepEqual(result.assets.map(a => a.path), ['_nuxt/a.css', '_nuxt/b.js', '_nuxt/c.mjs'])
  assert.equal(result.groups.css.files, 1)
  assert.equal(result.groups.js.files, 2)
  for (const k of ['css', 'js']) for (const m of ['raw', 'gzip', 'brotli']) {
    assert.equal(result.groups[k][m], result.assets.filter(a => a.kind === k).reduce((sum, a) => sum + a[m], 0))
  }
}))

test('missing CSS/JS and symlinks fail closed', () => fixture(root => {
  writeFileSync(join(root, 'a.css'), 'body{}')
  assert.throws(() => inventoryAssetSizes(root), /Missing/)
  symlinkSync(join(root, 'a.css'), join(root, 'a.js'))
  assert.throws(() => inventoryAssetSizes(root), /Symlink/)
}))

test('public measurement rejects invalid SHA and incomplete static tree', () => fixture(root => {
  assert.throws(() => measureFreeStatic(root, 'main'), /Exact source SHA/)
  assert.throws(() => measureFreeStatic(root, '0'.repeat(40)), /Exact source SHA/)
  assert.throws(() => measureFreeStatic(root, 'a'.repeat(40)), /Missing pre-rendered/)
}))

function auditedFixture(root) {
  writeFileSync(join(root, 'index.html'), '<h1>Image tools</h1>')
  writeFileSync(join(root, '404.html'), 'Not found')
  for (const route of FREE_ROUTES.slice(1)) {
    mkdirSync(join(root, route.slice(1)), { recursive: true })
    writeFileSync(join(root, route.slice(1), 'index.html'), 'Static tool')
  }
  mkdirSync(join(root, '_nuxt'))
  writeFileSync(join(root, '_nuxt', 'a.css'), 'body{}')
  writeFileSync(join(root, '_nuxt', 'b.js'), 'export{}')
  writeFileSync(join(root, '_headers'), buildFreeHeaders(root))
}

test('audited inventory retains exact tree hash and does not mutate output', () => fixture(root => {
  auditedFixture(root)
  const before = auditFreeStatic(root)
  const a = measureFreeStatic(root, 'a'.repeat(40))
  const b = measureFreeStatic(root, 'a'.repeat(40))
  assert.deepEqual(a, b)
  assert.equal(a.static_tree_sha256, before.staticTreeSha256)
  assert.equal(a.static_file_count, before.fileCount)
  assert.equal(a.static_raw_bytes, before.totalBytes)
  assert.equal(a.groups.css.files, 1)
  assert.equal(a.groups.js.files, 1)
  assert.equal(a.claimed_source_sha, 'a'.repeat(40))
  assert.equal(auditFreeStatic(root).staticTreeSha256, before.staticTreeSha256)
}))

test('measurement refuses forbidden server or worker output', () => fixture(root => {
  auditedFixture(root)
  writeFileSync(join(root, '_worker.js'), 'export{}')
  assert.throws(() => measureFreeStatic(root, 'a'.repeat(40)), /Forbidden/)
}))
