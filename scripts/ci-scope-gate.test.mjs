import { test } from 'node:test'
import assert from 'node:assert/strict'
import { JOBS, OUTPUT_KEYS } from './ci-scope-select.mjs'
import { verifyNeeds } from './ci-scope-gate.mjs'
const fixture = (selected = JOBS, mode = 'full') => {
  const needs = {
    'ci-scope': { result: 'success', outputs: { version: 'ci-scope-02-v1', mode } },
    'ci-policy': { result: 'success', outputs: {} },
  }
  for (const job of JOBS) {
    const enabled = selected.includes(job)
    needs['ci-scope'].outputs[OUTPUT_KEYS[job]] = String(enabled)
    needs[job] = { result: enabled ? 'success' : 'skipped', outputs: {} }
  }
  return needs
}
test('all jobs actual PASS meets full policy', () => assert.equal(verifyNeeds(fixture()).selected, JOBS.length))
test('docs-only selected jobs none, all explicitly skipped', () => assert.equal(verifyNeeds(fixture([], 'selective')).skipped, JOBS.length))
test('frontend-only must pass both Nuxt checks', () => assert.equal(verifyNeeds(fixture(['frontend-verification', 'frontend-browser-quality'], 'selective')).selected, 2))
for (const bad of ['failure', 'cancelled', 'skipped']) test(`selected native job ${bad} blocks`, () => {
  const needs = fixture(); needs['native-build-test'].result = bad
  assert.throws(() => verifyNeeds(needs))
})
test('an accidentally skipped required browser blocks', () => {
  const n = fixture(['frontend-verification', 'frontend-browser-quality'], 'selective')
  n['frontend-browser-quality'].result = 'skipped'
  assert.throws(() => verifyNeeds(n))
})
test('unexpected execution and missing jobs fail closed', () => {
  const n = fixture([], 'selective'); n['native-fuzz'].result = 'success'
  assert.throws(() => verifyNeeds(n))
  const missing = fixture(); delete missing['native-build-test']
  assert.throws(() => verifyNeeds(missing))
})
test('selector/policy failure, unknown schema and missing output fail', () => {
  const n = fixture(); n['ci-scope'].result = 'failure'
  assert.throws(() => verifyNeeds(n))
  const p = fixture(); p['ci-policy'].result = 'skipped'
  assert.throws(() => verifyNeeds(p))
  const v = fixture(); v['ci-scope'].outputs.version = 'tampered'
  assert.throws(() => verifyNeeds(v))
  const m = fixture(); delete m['ci-scope'].outputs.native_build_test
  assert.throws(() => verifyNeeds(m))
})
test('bootstrap fallback requires full nine-job execution', () => {
  const n = fixture(); n['ci-scope'].outputs.version = 'bootstrap-full-v1'
  assert.equal(verifyNeeds(n).selected, JOBS.length)
  n['ci-scope'].outputs.native_build_test = 'false'
  assert.throws(() => verifyNeeds(n))
})
test('full mode cannot omit jobs', () => {
  const n = fixture([], 'full')
  assert.throws(() => verifyNeeds(n))
})
