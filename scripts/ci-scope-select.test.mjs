import { test } from 'node:test'
import { readFileSync } from 'node:fs'
import assert from 'node:assert/strict'
import { JOBS, classifyDiff, encodeOutputs, full, selectFromEvent } from './ci-scope-select.mjs'

const record = (path, status = 'M') => `${status}\0${path}\0`
const names = decision => JOBS.filter(id => decision.jobs[id])
const frontend = ['frontend-verification', 'frontend-browser-quality']
const all = [...JOBS]
for (const [path, expected] of [
  ['README.md', []], ['next.md', []], ['docs/README.md', []],
  ['docs/FRONTEND_F4_CONVERT_CONTRACT.md', frontend],
  ['imgengine-saas/frontend/pages/convert-image.vue', frontend],
  ['imgengine-saas/frontend/tests/browser/local-convert.spec.ts', frontend],
  ['imgengine-saas/frontend/assets/main.css', frontend],
  ['imgengine-saas/frontend/types/api.ts', [...frontend, 'rust-api-container']],
  ['imgengine-saas/frontend/server/api/sheet-render.post.ts', [...frontend, 'rust-api-container']],
  ['backend/imgengine-api/src/lib.rs', ['rust-api-container']],
  ['backend/imgengine-supervisor/src/lib.rs', ['native-build-test', 'rust-api-container']],
  ...['imgengine/src/a.c', 'imgengine/include/imgengine/api/v2/sheet.h', 'backend/imgengine/src/sys.rs',
    '.github/workflows/ci.yml', 'scripts/ci-scope-select.mjs', 'infra/docker-compose.production.yml',
    'imgengine-saas/frontend/pnpm-lock.yaml', 'imgengine-saas/frontend/package.json',
    'docs/api-policy.md', 'docs/IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md',
    'Cargo.lock', 'Makefile', 'future-wasm/src/lib.rs', 'imgengine-saas/backend/app.py'].map(path => [path, all]),
]) test(`impact: ${path}`, () => assert.deepEqual(names(classifyDiff(record(path))), expected))
for (const [label, raw] of [
  ['empty', ''], ['truncated', 'M\0x'], ['delete', record('next.md', 'D')],
  ['rename', 'R100\0old\0new\0'], ['conflict', record('next.md', 'U')],
  ['traversal', record('../README.md')], ['control', record('README\n.md')],
  ['backslash', record('docs\\README.md')], ['oversized', record('next.md').repeat(10001)],
]) test(`fail closed: ${label}`, () => assert.deepEqual(names(classifyDiff(raw)), all))

test('frontend+status docs does not pull native', () => assert.deepEqual(names(classifyDiff(record('next.md') + record('imgengine-saas/frontend/pages/convert-image.vue'))), frontend))
test('mixed frontend and native requires all', () => assert.deepEqual(names(classifyDiff(record('imgengine/src/x.c') + record('imgengine-saas/frontend/pages/convert-image.vue'))), all))
test('docs and frontend contract still require browser quality', () => assert.deepEqual(names(classifyDiff(record('README.md') + record('docs/FRONTEND_F4_CONVERT_CONTRACT.md'))), frontend))
test('outputs carry exact nine boolean jobs', () => {
  const s = encodeOutputs(classifyDiff(record('next.md')))
  assert.match(s, /version=ci-scope-02-v1/)
  assert.match(s, /frontend_browser_quality=false/)
  assert.equal(s.trim().split('\n').length, JOBS.length + 4)
})
const a = 'a'.repeat(40), b = 'b'.repeat(40), event = { pull_request: { number: 76, base: { sha: a }, head: { sha: b } } }
test('PR diff uses trusted SHA and read-only numeric PR ref', () => {
  const calls = []
  const out = selectFromEvent('pull_request', event, args => {
    calls.push(args)
    if (args[0] === 'rev-parse') return `${b}\n`
    if (args[0] === 'merge-base') return `${a}\n`
    if (args[0] === 'diff') return record('README.md')
    return ''
  })
  assert.deepEqual(names(out), [])
  assert.deepEqual(calls[0], ['fetch', '--no-tags', 'origin', '+refs/pull/76/head:refs/remotes/origin/ci-scope-pr-head'])
  assert.deepEqual(calls.at(-1), ['diff', '--name-status', '-z', '--find-renames', a, b, '--'])
})
test('changed or untrusted ref, invalid SHA, Git error are full', () => {
  const shas = selectFromEvent('pull_request', event, args => args[0] === 'rev-parse' ? a : '')
  assert.equal(shas.full, true)
  assert.equal(selectFromEvent('pull_request', { pull_request: { number: 76, base: { sha: '--oops' }, head: { sha: b } } }, () => assert.fail()).full, true)
  assert.equal(selectFromEvent('pull_request', event, () => { throw Error('network') }).full, true)
})
test('push, merge queue, unknown event always full', () => {
  for (const eventName of ['push', 'merge_group', 'workflow_dispatch', 'pull_request_target']) assert.deepEqual(names(selectFromEvent(eventName, {}, () => assert.fail())), all)
})
test('full policy has no missing job', () => assert.deepEqual(names(full()), all))

// Mandatory doc-sync must not turn an otherwise isolated frontend PR into full CI.
const planningDocs = ["AGENTS.md","skills.md","docs/engineering/live-status.md","imgengine-saas/frontend/AGENTS.md","docs/IMGENGINE_F6_SHARED_UI_DOMAIN_MONETIZATION_ROADMAP.md","docs/IMGENGINE_FRONTEND_EXCELLENCE_PROGRAM_F6.md","docs/IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md","docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md"]
const shellPaths = ["imgengine-saas/frontend/app.vue","imgengine-saas/frontend/free-static/FreeSiteHeader.vue","imgengine-saas/frontend/free-static/landing.vue","imgengine-saas/frontend/free-static/tools.ts"]
for (const path of planningDocs) test('planning only: ' + path, () => assert.deepEqual(names(classifyDiff(record(path))), []))
for (const path of shellPaths) test('shell + mandatory docs: ' + path, () => assert.deepEqual(names(classifyDiff(planningDocs.map(p => record(p)).join('') + record(path))), frontend))
test('PR 127 planning-only path set selects no runtime jobs', () => assert.deepEqual(names(classifyDiff([...planningDocs, 'README.md', 'next.md'].map(p => record(p)).join(''))), []))
for (const path of ['docs/releases/free/v1.0.0/release-manifest.json','docs/engineering/DOCUMENTATION_SYNC_CONTRACT.md','imgengine-saas/frontend/free-static/empty-server/unknown.ts','imgengine-saas/frontend/nuxt.config.ts','scripts/ci-scope-select.mjs','imgengine/src/a.c']) test('planning plus high-risk stays full: ' + path, () => assert.deepEqual(names(classifyDiff(record('AGENTS.md') + record(path))), all))
test('planning deletion still fails closed', () => assert.deepEqual(names(classifyDiff(record('AGENTS.md', 'D'))), all))

test('only existing Free presentation paths plus planning docs opt in', () => {
  for (const path of ['FreeSiteHeader.vue', 'landing.vue', 'tools.ts']) {
    const d = classifyDiff(record('AGENTS.md') + record('imgengine-saas/frontend/free-static/' + path))
    assert.equal(d.freeOnly, true)
    assert.deepEqual(names(d), frontend)
    assert.match(encodeOutputs(d), /free_only=true/)
  }
})
test('mixed, added, unknown, config, native and full scope never opt in', () => {
  const freePath = 'imgengine-saas/frontend/free-static/landing.vue'
  for (const path of ['imgengine-saas/frontend/app.vue', 'imgengine-saas/frontend/assets/css/tokens.css', 'imgengine-saas/frontend/pages/resize-image.vue', 'imgengine-saas/frontend/free-static/unknown.vue', 'imgengine-saas/frontend/nuxt.config.ts', 'imgengine/src/a.c', '.github/workflows/ci.yml']) {
    assert.equal(classifyDiff(record(freePath) + record(path)).freeOnly, false)
  }
  assert.equal(classifyDiff(record(freePath, 'A')).freeOnly, false)
  assert.equal(classifyDiff(record(freePath, 'D')).freeOnly, false)
  assert.equal(classifyDiff(record('README.md')).freeOnly, false)
  assert.equal(full().freeOnly, false)
})
test('Free-only workflow keeps static gates and normal mode fails safe', () => {
  const workflow = readFileSync(new URL('../.github/workflows/ci.yml', import.meta.url), 'utf8')
  for (const step of ['Run frontend unit tests', 'Build production Nuxt application', 'Generate Nuxt production output', 'Run local tool browser quality matrix']) {
    assert.ok(workflow.includes("      - name: " + step + "\n        if: ${{ needs.ci-scope.outputs.free_only != 'true' }}"))
  }
  for (const step of ['Generate static Free-only distribution (Docker Ubuntu 24.04)', 'Reject forbidden runtime, secrets, API and missing static routes', 'Record audited Free CSS and JavaScript size inventory', 'Run full local-tool and static-boundary browser matrix']) {
    assert.ok(workflow.includes('      - name: ' + step + '\n        working-directory:'))
  }
  assert.ok(workflow.includes("if: ${{ needs.ci-scope.outputs.free_only == 'true' }}"))
})
