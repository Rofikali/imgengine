import test from 'node:test'
import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { readFileSync } from 'node:fs'
import { planStatusPublication, BOT_EMAIL } from './milestone-status-publication.mjs'

const sha = 'a'.repeat(40)
const shared = { remoteSha: sha, remoteAuthor: BOT_EMAIL, sameTree: true, openPrNumber: '' }

test('recover missing review PR even when existing bot branch tree matches', () => {
  assert.equal(planStatusPublication(shared), 'keep-open')
  assert.equal(planStatusPublication({ ...shared, openPrNumber: '92' }), 'keep-reuse')
})

test('refresh changed branch without duplicating an already open review PR', () => {
  assert.equal(planStatusPublication({ ...shared, sameTree: false, openPrNumber: '92' }), 'push-reuse')
  assert.equal(planStatusPublication({ ...shared, sameTree: false }), 'push-open')
  assert.equal(planStatusPublication({
    remoteSha: '', remoteAuthor: '', sameTree: false, openPrNumber: ''
  }), 'push-open')
})

test('fail closed on hand-edited branch, malformed ref, invalid PR and contradictory state', () => {
  assert.throws(() => planStatusPublication({ ...shared, remoteAuthor: 'other@example.com' }), /not owned/)
  assert.throws(() => planStatusPublication({ ...shared, remoteSha: '123' }), /Invalid remote/)
  assert.throws(() => planStatusPublication({ ...shared, openPrNumber: '1;echo' }), /Invalid status PR/)
  assert.throws(() => planStatusPublication({ ...shared, remoteSha: '' }), /Inconsistent/)
  assert.throws(() => planStatusPublication({ ...shared, sameTree: 'true' }), /Malformed/)
})

test('CLI produces deterministic action, fails on invalid inputs', () => {
  const valid = spawnSync(process.execPath, [
    'scripts/milestone-status-publication.mjs', sha, BOT_EMAIL, 'true', ''
  ], { encoding: 'utf8' })
  assert.equal(valid.status, 0)
  assert.equal(valid.stdout.trim(), 'keep-open')
  const invalid = spawnSync(process.execPath, [
    'scripts/milestone-status-publication.mjs', sha, 'untrusted@example.com', 'true', ''
  ], { encoding: 'utf8' })
  assert.notEqual(invalid.status, 0)
  assert.match(invalid.stderr, /not owned/)
})

test('workflow actually uses publication planner before deciding to open PR', () => {
  const workflow = readFileSync('.github/workflows/milestone-status-sync.yml', 'utf8')
  assert.match(workflow, /node scripts\/milestone-status-publication\.mjs/)
  assert.match(workflow, /keep-open\|push-open/)
  assert.match(workflow, /gh pr create/)
  assert.doesNotMatch(workflow, /Existing bot branch already has identical generated status[^\n]*\n\s*exit 0/)
  assert.match(workflow, /pull_request_target/)
  assert.match(workflow, /ref: main/)
})
