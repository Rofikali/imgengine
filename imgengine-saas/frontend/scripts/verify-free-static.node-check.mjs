import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, writeFileSync, rmSync, symlinkSync } from 'node:fs'
import { join } from 'node:path'
import { tmpdir } from 'node:os'
import { auditFreeStatic, FREE_ROUTES } from './verify-free-static.mjs'

function withFixture(fn) {
  const root = mkdtempSync(join(tmpdir(), 'imgengine-free-static-'))
  const write = (name, value = 'static browser content') => {
    const file = join(root, name)
    mkdirSync(join(file, '..'), { recursive: true })
    writeFileSync(file, value)
  }
  try {
    write('index.html', '<h1>Edit images privately.</h1>')
    write('404.html', 'not found')
    for (const route of FREE_ROUTES.slice(1)) write(route.slice(1) + '/index.html')
    write('_nuxt/assets.js')
    fn({ root, write })
  } finally { rmSync(root, { recursive: true, force: true }) }
}

test('allows all nine verified static routes and browser assets; produces deterministic digest', () => {
  withFixture(({root}) => {
    const a = auditFreeStatic(root), b = auditFreeStatic(root)
    assert.equal(a.fileCount, 11)
    assert.equal(a.staticTreeSha256, b.staticTreeSha256)
    assert.equal(a.staticTreeSha256.length, 64)
  })
})

for (const [label, path, data] of [
  ['SPA fallback','200.html','unexpected router'],
  ['Cloudflare function','_worker.js','fetch(){}'],
  ['Cloudflare route manifest','_routes.json','{}'],
  ['server folder','server/api.js',''],
  ['functions folder','functions/index.js',''],
  ['API endpoint','api/render/index.html',''],
  ['env','_nuxt/.env','sensitive'],
  ['source map','_nuxt/app.js.map','{}'],
  ['native shared library','_nuxt/libimgengine.so','blob'],
  ['secret in bundle','_nuxt/secret.js','IMGENGINE_RENDER_API_KEY'],
  ['leaked service endpoint','_nuxt/api.js','/api/sheet-render'],
  ['legacy server page','_nuxt/old.js','Server print layouts (uploads on Render)'],
  ['local Rust host','_nuxt/host.js','http://localhost:8000'],
]) test('rejects ' + label, () => {
  withFixture(({root,write}) => {
    write(path,data)
    assert.throws(() => auditFreeStatic(root))
  })
})

test('missing route fails closed', () => {
  withFixture(({root}) => {
    rmSync(join(root, 'convert-image'), { recursive: true })
    assert.throws(() => auditFreeStatic(root), /Missing pre-rendered Free route/)
  })
})
test('missing explicit 404 fails closed', () => {
  withFixture(({root}) => {
    rmSync(join(root, '404.html'))
    assert.throws(() => auditFreeStatic(root), /Missing real 404/)
  })
})
test('symlinks cannot escape public artifact', () => {
  withFixture(({root}) => {
    symlinkSync(join(root, 'index.html'), join(root, '_nuxt', 'shadow.js'))
    assert.throws(() => auditFreeStatic(root), /Symlink/)
  })
})
