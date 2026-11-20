import test from 'node:test'
import assert from 'node:assert/strict'
import {
  validateRegistry, determinePRStatus, resolveMilestones, renderStatus,
  replaceGeneratedRegion, createClient, generate
} from './milestone-sync.mjs'

const sha = 'a'.repeat(40)
const sha2 = 'b'.repeat(40)
function pr(number, opts = {}) {
  return { number, state: 'open', draft: false, head: { sha }, base: { ref: 'main' },
    merge_commit_sha: null, merged_at: null, ...opts }
}
function registry() {
  return { schema_version: 1, release: 'IMGENGINE Free v1.0.0', release_state: 'NOT_RELEASED',
    release_blockers: ['Owner launch authorization pending'],
    milestones: [
      { id: 'F5-SUPPLY-04', title: 'Security gate', priority: 30, pr: 89,
        declared_state: 'PLANNED', depends_on: [] },
      { id: 'F5-SUPPLY-05', title: 'Dependency remediation', priority: 10, pr: null,
        declared_state: 'READY', depends_on: ['F5-SUPPLY-04'] },
      { id: 'F5-RC-01', title: 'Release artifact', priority: 20, pr: null,
        declared_state: 'PLANNED', depends_on: ['F5-SUPPLY-05'] }
    ] }
}

test('merged requires verifiable merge SHA, not only closed state', () => {
  const merged = pr(89, { state: 'closed', merged_at: '2026-11-20T16:17:06Z', merge_commit_sha: sha2 })
  assert.equal(determinePRStatus(merged, []), 'MERGED')
  assert.equal(determinePRStatus(pr(89, { state: 'closed' }), []), 'CLOSED_UNMERGED')
  assert.throws(() => determinePRStatus(pr(89, { state: 'closed', merged_at: 'now' }), []), /merge SHA/)
})

test('no green CI claim without exact PR-head verification gate', () => {
  assert.equal(determinePRStatus(pr(89), []), 'CI_PENDING')
  assert.equal(determinePRStatus(pr(89, { draft: true }), []), 'DRAFT')
  const gate = [{ name: 'CI verification gate', status: 'completed', conclusion: 'success' }]
  assert.equal(determinePRStatus(pr(89), gate), 'READY_FOR_REVIEW')
  assert.equal(determinePRStatus(pr(89), [{ ...gate[0], conclusion: 'failure' }]), 'CI_FAILED')
  assert.equal(determinePRStatus(pr(89), [{ name: 'other', status: 'completed', conclusion: 'success' }]), 'CI_PENDING')
})

test('dependencies advance NOW automatically only after confirmed merge', () => {
  const data = registry()
  const details = new Map([[89, { pr: pr(89), checks: [] }]])
  let report = resolveMilestones(data, details)
  assert.equal(report.now.id, 'F5-SUPPLY-04')
  assert.deepEqual(report.all.find(x => x.id === 'F5-SUPPLY-05').unmet, ['F5-SUPPLY-04'])
  details.set(89, { pr: pr(89, { state: 'closed', merged_at: 'now', merge_commit_sha: sha2 }), checks: [] })
  report = resolveMilestones(data, details)
  assert.equal(report.now.id, 'F5-SUPPLY-05')
  assert.equal(report.next.id, 'F5-RC-01')
  assert.equal(report.completed.length, 1)
  assert.match(renderStatus(data, report), /NOT_RELEASED/)
})

test('registry rejects missing dependencies, duplicate IDs and release-state escalation', () => {
  const data = registry()
  data.release_state = 'RELEASED'
  assert.throws(() => validateRegistry(data), /Invalid milestone/)
  data.release_state = 'NOT_RELEASED'
  data.milestones[1].depends_on = ['UNKNOWN']
  assert.throws(() => validateRegistry(data), /dependency/)
  data.milestones[1].depends_on = ['F5-SUPPLY-04']
  data.milestones[1].id = 'F5-SUPPLY-04'
  assert.throws(() => validateRegistry(data), /milestone record/)
})

test('generated region is idempotent and never erases historical notes', () => {
  const original = '# IMGENGINE milestones\n\nOld historical update\n'
  const first = replaceGeneratedRegion(original, '### NOW\n\nF5-SUPPLY-05\n')
  assert.match(first, /Old historical update/)
  assert.equal(replaceGeneratedRegion(first, '### NOW\n\nF5-SUPPLY-05\n'), first)
  assert.match(replaceGeneratedRegion(first, '### NOW\n\nF5-RC-01\n'), /F5-RC-01/)
  assert.throws(() => replaceGeneratedRegion('# title\n\n<!-- BEGIN AUTO-GENERATED PR MILESTONES -->', 'test'), /Broken/)
})

test('untrusted PR titles cannot inject new Markdown rows or HTML', () => {
  const data = registry()
  const details = new Map([[89, { pr: pr(89), checks: [] }]])
  const report = resolveMilestones(data, details)
  const rendered = renderStatus(data, report, [
    { number: 99, title: '<script>bad</script>|row\nanother row', head: { ref: 'feature/unsafe' } },
    { number: 100, title: 'Bot PR', head: { ref: 'docs/auto-milestone-status' } }
  ])
  assert.doesNotMatch(rendered, /<script>/)
  assert.match(rendered, /PR #99/)
  assert.doesNotMatch(rendered, /PR #100/)
})

test('GitHub API errors fail closed; no false merged status', async () => {
  const client = createClient('example', 'Rofikali/imgengine', async () => ({ ok: false, status: 403 }))
  await assert.rejects(client('/pulls/89'), /403/)
  const data = registry()
  await assert.rejects(generate({ registry: data, nextText: '# next\n', client }), /403/)
})

test('remote facts verify PR number and target base', async () => {
  const data = registry()
  const client = async path => {
    if (path.startsWith('/pulls/89')) return pr(89, { base: { ref: 'another-branch' } })
    throw Error('Unexpected path: ' + path)
  }
  await assert.rejects(generate({ registry: data, nextText: '# next\n', client }), /base mismatch/)
})
