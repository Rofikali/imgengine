import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtempSync, writeFileSync, rmSync, readFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { execFileSync } from 'node:child_process'
import { classify, fallback, fromEvent, jobs } from './ci-scope.mjs'

const record = (path, status = 'M') => `${status}\0${path}\0`
for (const [path, expected] of [
  ['docs/README.md', []], ['imgengine-saas/frontend/assets/main.css', ['frontend-verification']],
  ['imgengine-saas/frontend/components/SheetRenderPanel.vue', ['frontend-verification', 'rust-api-container']],
  ['imgengine-saas/frontend/utils/render-form.ts', ['frontend-verification', 'rust-api-container']],
  ['imgengine-saas/frontend/server/api/sheet-render.post.ts', ['frontend-verification', 'rust-api-container']],
  ['backend/imgengine-api/src/lib.rs', ['rust-api-container']],
  ['backend/imgengine-supervisor/src/lib.rs', ['rust-api-container']],
  ...['backend/imgengine/src/sys.rs', 'imgengine/include/imgengine/api/v2/sheet.h', 'imgengine/src/runtime/worker_loop.c', 'infra/docker-compose.production.yml', '.github/workflows/ci.yml', 'Cargo.lock', 'imgengine-saas/frontend/pnpm-lock.yaml', 'Makefile', 'docs/api.md', 'AGENTS.md', 'future-wasm/src/lib.rs', 'imgengine-saas/backend/app.py'].map(p => [p, jobs]),
]) test(`path: ${path}`, () => {
  const result = classify(record(path))
  assert.deepEqual(result.proposedJobs, expected)
  assert.deepEqual(result.actualJobs, jobs)
})
for (const [name, raw] of [
  ['empty', ''], ['truncated', 'M\0x'], ['rename', 'R100\0old\0new\0'],
  ['delete', record('docs/README.md', 'D')], ['unmerged', record('README.md', 'U')],
  ['traversal', record('../README.md')], ['newline', record('docs/notes/a\nb.md')],
  ['backslash', record('docs\\README.md')], ['large', record('README.md').repeat(10001)],
]) test(`fail closed: ${name}`, () => assert.deepEqual(classify(raw).proposedJobs, jobs))
test('mixed UI/native preserves full verification', () => assert.deepEqual(classify(record('imgengine-saas/frontend/assets/main.css') + record('imgengine/src/a.c')).proposedJobs, jobs))
test('mixed docs/proxy unions affected boundaries', () => {
  const r = classify(record('README.md') + record('imgengine-saas/frontend/types/api.ts'))
  assert.equal(r.affected.docs && r.affected.frontend && r.affected.proxy, true)
})
const a = 'a'.repeat(40), b = 'b'.repeat(40), event = { pull_request: { base: { sha: a }, head: { sha: b } } }
test('fork PR uses object SHAs and merge base, never branch names', () => {
  const calls = []
  const r = fromEvent('pull_request', event, args => { calls.push(args); return calls.length === 1 ? a : record('README.md') })
  assert.equal(r.full, false)
  assert.deepEqual(calls, [['merge-base', a, b], ['diff', '--name-status', '-z', '--find-renames', a, b, '--']])
})
test('missing base / Git failure is full', () => assert.deepEqual(fromEvent('pull_request', event, () => { throw Error() }).proposedJobs, jobs))
test('malicious SHA never reaches Git', () => assert.deepEqual(fromEvent('pull_request', { pull_request: { head: { sha: '--help' } } }, () => assert.fail()).proposedJobs, jobs))
test('push range always full', () => {
  let called = false
  assert.equal(fromEvent('push', { before: a, after: b }, () => { called = true; return '' }).full, true)
  assert.equal(called, true)
})
test('initial push / unknown event full', () => {
  assert.equal(fromEvent('push', { before: '0'.repeat(40), after: b }, () => assert.fail()).full, true)
  assert.equal(fromEvent('pull_request_target', {}, () => assert.fail()).full, true)
})
test('real Git diff: spaces, branches, renames and deletions', () => {
  const cwd = mkdtempSync(join(tmpdir(), 'imgengine-ci-scope-'))
  const git = args => execFileSync('git', args, { cwd, encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] })
  try {
    git(['init']); git(['config', 'user.email', 'fixture@example.invalid']); git(['config', 'user.name', 'Fixture'])
    writeFileSync(join(cwd, 'README.md'), 'base'); git(['add', '.']); git(['commit', '-m', 'base'])
    const base = git(['rev-parse', 'HEAD']).trim()
    git(['checkout', '-b', 'feature']); git(['mv', 'README.md', 'file with spaces.md']); git(['commit', '-m', 'rename'])
    const head = git(['rev-parse', 'HEAD']).trim()
    assert.equal(fromEvent('pull_request', { pull_request: { base: { sha: base }, head: { sha: head } } }, git).full, true)
    git(['rm', 'file with spaces.md']); git(['commit', '-m', 'delete'])
    assert.equal(classify(git(['diff', '--name-status', '-z', head, 'HEAD'])).full, true)
  } finally { rmSync(cwd, { recursive: true, force: true }) }
})
test('fallback always retains seven jobs', () => assert.deepEqual(fallback('fixture').actualJobs, jobs))
test('CLI missing event fails closed without credentials', () => {
  const result = JSON.parse(execFileSync(process.execPath, [fileURLToPath(new URL('./ci-scope.mjs', import.meta.url))], { env: { ...process.env, GITHUB_EVENT_PATH: 'nonexistent-event-fixture' }, encoding: 'utf8' }))
  assert.equal(result.full, true); assert.deepEqual(result.actualJobs, jobs)
})
test('workflow retains seven mandatory jobs and approved additive quality jobs without path filters', () => {
  const workflow = readFileSync(new URL('../.github/workflows/ci.yml', import.meta.url), 'utf8')
  const jobSection = workflow.split('\njobs:')[1]
  // Existing seven required jobs stay unconditional; the P8 parser and F2c
  // browser-quality jobs are additive, not substitutes for any required gate.
  const expected = [...jobs, 'infra-p8-parser', 'frontend-browser-quality']
  assert.deepEqual([...jobSection.matchAll(/^  ([\w-]+):\r?$/gm)].map(m => m[1]).sort(), expected.sort())
  assert.doesNotMatch(workflow, /^\s+(paths|paths-ignore|pull_request_target):/m)
  assert.doesNotMatch(jobSection, /^    (if|needs):/m)
  assert.doesNotMatch(workflow, /needs\..*scope/)
})
