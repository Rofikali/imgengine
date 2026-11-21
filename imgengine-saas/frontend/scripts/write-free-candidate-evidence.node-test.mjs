import test from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, rmSync, writeFileSync, readFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { spawnSync } from 'node:child_process'
import { createCandidateManifest, assertDetachedLocation } from './write-free-candidate-evidence.mjs'

const sha = 'a'.repeat(40)
const digest = 'b'.repeat(64)
const routes = ['/', '/resize-image', '/crop-image', '/compress-image',
  '/rotate-image', '/convert-image', '/privacy', '/terms', '/support']
const inventory = { staticTreeSha256: digest, fileCount: 36,
  totalBytes: 292571, routes }

test('candidate provenance has stable exact-source and no accidental approval', () => {
  const m = createCandidateManifest(sha, inventory)
  assert.equal(m.source_sha, sha)
  assert.equal(m.static_tree_sha256, digest)
  assert.equal(m.file_count, 36)
  assert.equal(m.status, 'CANDIDATE_NOT_APPROVED')
  assert.equal(m.final_release_tag, null)
  assert.equal(m.owner_approval, null)
  assert.equal(m.cloudflare_deployment, null)
  assert.deepEqual(m.static_routes, routes)
  assert.equal(JSON.stringify(createCandidateManifest(sha, inventory)), JSON.stringify(m))
})

test('reject missing source SHA, incomplete inventory, and unknown public routes', () => {
  assert.throws(() => createCandidateManifest('main', inventory), /immutable Git source SHA/)
  assert.throws(() => createCandidateManifest(sha, { ...inventory, staticTreeSha256: 'none' }), /malformed/)
  assert.throws(() => createCandidateManifest(sha, { ...inventory, routes: routes.slice(1) }), /malformed/)
  assert.throws(() => createCandidateManifest(sha, { ...inventory, totalBytes: -10 }), /malformed/)
  assert.throws(() => createCandidateManifest(sha, { ...inventory, fileCount: 0 }), /malformed/)
})

test('detached evidence cannot be saved inside static output or replace evidence', () => {
  const root = mkdtempSync(join(tmpdir(), 'imgengine-rc-test-'))
  try {
    const dist = join(root, 'public')
    mkdirSync(dist)
    const detached = join(root, 'evidence.json')
    assert.equal(assertDetachedLocation(dist, detached), detached)
    assert.throws(() => assertDetachedLocation(dist, join(dist, 'evidence.json')), /outside/)
    assert.throws(() => assertDetachedLocation(dist, join(dist, 'nested', 'candidate.json')), /outside/)
    assert.throws(() => assertDetachedLocation(dist, join(root, 'not-json.txt')), /\.json/)
    writeFileSync(detached, 'original')
    assert.throws(() => assertDetachedLocation(dist, detached), /replace existing/)
  } finally { rmSync(root, { recursive: true, force: true }) }
})

test('CLI fails closed and never silently creates a release', () => {
  const call = spawnSync(process.execPath,
    ['scripts/write-free-candidate-evidence.mjs'],
    { encoding: 'utf8', cwd: process.cwd() })
  assert.notEqual(call.status, 0)
  assert.match(call.stderr, /BLOCKED/)
})

test('manual release workflow must run the candidate writer after static verification', () => {
  const workflow = readFileSync('../../.github/workflows/free-v1-release-preflight.yml', 'utf8')
  const staticGate = workflow.indexOf('pnpm run verify:free-static')
  const candidateWriter = workflow.indexOf('scripts/write-free-candidate-evidence.mjs')
  assert.ok(staticGate !== -1 && candidateWriter > staticGate)
  assert.match(workflow, /workflow_dispatch/)
  assert.match(workflow, /contents: read/)
  assert.doesNotMatch(workflow, /wrangler pages deploy|git tag -a/)
})
