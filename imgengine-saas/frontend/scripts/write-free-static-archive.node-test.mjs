import test from 'node:test'
import assert from 'node:assert/strict'
import {
  mkdtempSync, mkdirSync, readFileSync, rmSync, symlinkSync, writeFileSync,
} from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { spawnSync } from 'node:child_process'
import { buildFreeHeaders } from './free-security-headers.mjs'
import { writeFreeArchiveCandidate } from './write-free-static-archive.mjs'

const sha = 'a'.repeat(40)
const routes = [
  '/', '/resize-image', '/crop-image', '/compress-image', '/rotate-image',
  '/convert-image', '/privacy', '/terms', '/support',
]

function withFixture(fn) {
  const root = mkdtempSync(join(tmpdir(), 'imgengine-free-archive-test-'))
  const dist = join(root, 'public')
  mkdirSync(dist)
  const write = (path, contents = 'safe static asset') => {
    const dst = join(dist, path)
    mkdirSync(join(dst, '..'), { recursive: true })
    writeFileSync(dst, contents)
  }
  try {
    write('index.html', '<h1>Edit images privately.</h1>')
    write('404.html', 'Not found')
    for (const route of routes.slice(1)) write(route.slice(1) + '/index.html')
    write('_nuxt/placeholder.js')
    write('_headers', buildFreeHeaders(dist))
    fn({ root, dist, write })
  } finally {
    rmSync(root, { recursive: true, force: true })
  }
}

test('detached normalized tar candidate has matching independently audited archive, no release authorization', () => {
  withFixture(({ root, dist }) => {
    const a = writeFreeArchiveCandidate(dist, join(root, 'candidate-a'), sha)
    const b = writeFreeArchiveCandidate(dist, join(root, 'candidate-b'), sha)
    assert.equal(a.schema_version, 1)
    assert.equal(a.source_sha, sha)
    assert.equal(a.status, 'CANDIDATE_NOT_APPROVED')
    assert.match(a.archive_sha256, /^[a-f0-9]{64}$/)
    assert.match(a.static_tree_sha256, /^[a-f0-9]{64}$/)
    assert.equal(a.archive_sha256, b.archive_sha256, 'same static bytes create reproducible tar')
    assert.deepEqual(a.routes, routes)
    assert.equal(a.owner_approval, null)
    assert.equal(a.signed_provenance, null)
    assert.equal(a.cloudflare_deployment, null)
    assert.equal(a.final_release_tag, null)
    const saved = JSON.parse(readFileSync(join(root, 'candidate-a', 'candidate-archive.json'), 'utf8'))
    assert.deepEqual(saved, a)
    const archive = join(root, 'candidate-a', 'candidate-static.tar')
    const names = spawnSync('tar', ['-tf', archive], { encoding: 'utf8', shell: false })
    assert.equal(names.status, 0)
    assert.match(names.stdout, /index.html/)
    assert.doesNotMatch(names.stdout, /server\/|_worker\.js|node_modules\//)
    assert.throws(() => writeFreeArchiveCandidate(dist, join(root, 'candidate-a'), sha), /overwrite/)
    assert.deepEqual(JSON.parse(readFileSync(join(root, 'candidate-a', 'candidate-archive.json'), 'utf8')), a)
  })
})

test('fails closed on bad SHA, embedded destination, unsafe symlink parent and invalid static output', () => {
  withFixture(({ root, dist, write }) => {
    assert.throws(() => writeFreeArchiveCandidate(dist, join(root, 'bad-sha'), 'main'), /source SHA/)
    assert.throws(() => writeFreeArchiveCandidate(dist, join(dist, 'nested-evidence'), sha), /outside static output/)
    symlinkSync(dist, join(root, 'symlinked-static-root'))
    assert.throws(() => writeFreeArchiveCandidate(join(root, 'symlinked-static-root'), join(root, 'root-alias-output'), sha), /Symlinked static root/)
    const safe = join(root, 'real-detached')
    mkdirSync(safe)
    symlinkSync(safe, join(root, 'alias-detached'))
    assert.throws(() => writeFreeArchiveCandidate(dist, join(root, 'alias-detached'), sha), /must not be a symlink/)
    write('api/render/index.html', 'server output forbidden')
    assert.throws(() => writeFreeArchiveCandidate(dist, join(root, 'invalid-static'), sha), /Forbidden/)
  })
})

test('CLI needs complete explicit arguments; manual workflow preserves archive JSON only', () => {
  const x = spawnSync(process.execPath, ['scripts/write-free-static-archive.mjs'], {
    cwd: process.cwd(), encoding: 'utf8',
  })
  assert.notEqual(x.status, 0)
  assert.match(x.stderr, /BLOCKED/)
  const workflow = readFileSync('../../.github/workflows/free-v1-release-preflight.yml', 'utf8')
  const audit = workflow.indexOf('pnpm run verify:free-static')
  const archive = workflow.indexOf('scripts/write-free-static-archive.mjs')
  assert.ok(audit >= 0 && archive > audit)
  assert.match(workflow, /candidate-archive\.json/)
  assert.doesNotMatch(workflow, /wrangler pages deploy|git tag -a/)
  const upload = workflow.slice(workflow.indexOf('name: Attach REVIEW-ONLY candidate evidence JSON'))
  assert.doesNotMatch(upload, /candidate-static\.tar|\.output\/public/)
})
