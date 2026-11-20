import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, writeFileSync, rmSync, symlinkSync } from 'node:fs'
import { join } from 'node:path'
import { tmpdir } from 'node:os'
import { auditFreeStatic, FREE_ROUTES } from './verify-free-static.mjs'
import { buildFreeHeaders } from './free-security-headers.mjs'
import { FREE_STATIC_BROWSER_PROJECTS, FIREFOX_TEST_SHARDS, runFreeStaticBrowserMatrix } from './run-free-static-browser-matrix.mjs'

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
    write('_headers', buildFreeHeaders(root))
    fn({ root, write })
  } finally { rmSync(root, { recursive: true, force: true }) }
}

test('allows all nine verified static routes and browser assets; produces deterministic digest', () => {
  withFixture(({root}) => {
    const a = auditFreeStatic(root), b = auditFreeStatic(root)
    assert.equal(a.fileCount, 12)
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

test('Free static audit requires generated, intact Cloudflare security headers', () => {
  withFixture(({root}) => {
    rmSync(join(root, '_headers'))
    assert.throws(() => auditFreeStatic(root), /_headers missing/)
  })
})
test('tampered CSP is rejected even if other artifact files are safe', () => {
  withFixture(({root, write}) => {
    write('_headers', buildFreeHeaders(root).replace("script-src 'self'", "script-src 'self' 'unsafe-inline'"))
    assert.throws(() => auditFreeStatic(root), /do not match|unsafe/)
  })
})


test('Free static matrix runs all projects and all fifty disjoint Firefox test shards', () => {
  const invocations = []
  const result = runFreeStaticBrowserMatrix((command, args, options) => {
    invocations.push({ command, args, options })
    return { status: 0 }
  })
  assert.equal(result, 0)
  assert.equal(FREE_STATIC_BROWSER_PROJECTS.length, 4)
  assert.equal(FIREFOX_TEST_SHARDS, 50)
  assert.equal(invocations.length, FIREFOX_TEST_SHARDS + 3) // Chromium, mobile, fifty Firefox, WebKit.
  assert.deepEqual(invocations.map(({ args }) => args[5]), [
    '--project=desktop-chromium',
    '--project=mobile-chromium',
    ...Array.from({ length: FIREFOX_TEST_SHARDS }, () => '--project=desktop-firefox'),
    '--project=desktop-webkit',
  ])
  for (const [i, { command, args, options }] of invocations.entries()) {
    assert.equal(command, 'pnpm')
    assert.deepEqual(args.slice(0, 5), ['exec', 'playwright', 'test', '--config', 'playwright.free-static.config.ts'])
    assert.equal(args[5].startsWith('--project='), true)
    assert.equal(options.stdio, 'inherit')
    assert.match(options.cwd, /frontend\/?$/)
    assert.equal(options.env.IMGENGINE_STATIC_TEST_DIAGNOSTICS, i >= 2 && i < FIREFOX_TEST_SHARDS + 2 ? '1' : '0')
    if (i >= 2 && i < FIREFOX_TEST_SHARDS + 2) {
      assert.deepEqual(args.slice(6), ['--fully-parallel', '--shard=' + (i - 1) + '/' + FIREFOX_TEST_SHARDS])
    } else {
      assert.equal(args.length, 6)
    }
  }
})

test('Free static matrix fails closed on a failing Firefox shard, never skipping remaining checks', () => {
  const executed = []
  const code = runFreeStaticBrowserMatrix((_command, args) => {
    executed.push(args.at(-1))
    return { status: executed.length === 5 ? 2 : 0 }
  })
  assert.equal(code, 2)
  assert.deepEqual(executed, [
    '--project=desktop-chromium',
    '--project=mobile-chromium',
    '--shard=1/50',
    '--shard=2/50',
    '--shard=3/50',
  ])
})

test('Free static matrix treats an interrupted/unknown process status as failure', () => {
  assert.equal(runFreeStaticBrowserMatrix(() => ({ status: null, signal: 'SIGTERM' })), 1)
  assert.throws(() => runFreeStaticBrowserMatrix(() => ({ status: null, error: Error('spawn denied') })), /spawn denied/)
})
