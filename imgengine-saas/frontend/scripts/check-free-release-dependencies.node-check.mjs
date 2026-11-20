import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { existsSync, statSync } from 'node:fs'
import { fileURLToPath } from 'node:url'
import { test } from 'node:test'
import { runFreeReleaseDependencyAudit } from './check-free-release-dependencies.mjs'

const cli = fileURLToPath(new URL('./check-free-release-dependencies.mjs', import.meta.url))
const vulnerabilities = (critical = 0, high = 0, moderate = 0, low = 0) => ({
  metadata: { vulnerabilities: { critical, high, moderate, low } },
})
function trial(stdout, pnpmStatus = 0) {
  const calls = []
  const audit = runFreeReleaseDependencyAudit((command, args, options) => {
    calls.push({ command, args, options })
    if (command === 'pnpm') return { status: pnpmStatus, stdout }
    assert.equal(command, process.execPath)
    assert.equal(args[2], '--release-candidate')
    assert.equal(args.length, 3)
    assert.equal(options.shell, false)
    assert.equal(statSync(args[1]).mode & 0o777, 0o600)
    const check = spawnSync(command, args, { ...options, stdio: 'pipe', encoding: 'utf8' })
    calls.at(-1).check = check
    return check
  })
  assert.equal(calls[0]?.command, 'pnpm')
  assert.deepEqual(calls[0]?.args, ['audit', '--prod', '--json'])
  assert.equal(calls[0]?.options.shell, false)
  assert.equal(calls[0]?.options.maxBuffer, 8 * 1024 * 1024)
  if (calls[1]) assert.equal(existsSync(calls[1].args[1]), false, 'temporary raw registry report must be deleted')
  return { audit, calls }
}

test('clean Free release dependency audit passes and removes private report', () => {
  const { audit, calls } = trial(JSON.stringify(vulnerabilities()))
  assert.equal(audit, 0)
  assert.match(calls[1].check.stdout, /release-candidate dependency threshold PASSED/)
})

test('published moderate vulnerabilities block release candidate even if pnpm exits one', () => {
  const report = vulnerabilities(0, 0, 2, 0)
  report.advisories = {
    glob: { severity: 'moderate', module_name: 'glob', url: 'https://github.com/advisories/GHSA-3g3m-2fjr-6pw4', patched_versions: '>=14.0.2' },
    minimatch: { severity: 'moderate', module_name: 'minimatch', url: 'https://github.com/advisories/GHSA-g3hv-75wg-m249', patched_versions: '>=11.0.1' },
  }
  const { audit, calls } = trial(JSON.stringify(report), 1)
  assert.equal(audit, 1)
  assert.match(calls[1].check.stderr, /release-candidate audit BLOCKED/)
  assert.match(calls[1].check.stdout, /severity=moderate package=glob/)
})

test('critical/high always fail candidate audit, and low alone is informational', () => {
  for (const counts of [[1, 0, 0, 0], [0, 1, 0, 0]]) {
    const { audit } = trial(JSON.stringify(vulnerabilities(...counts)), 1)
    assert.equal(audit, 1)
  }
  const low = trial(JSON.stringify(vulnerabilities(0, 0, 0, 1)), 1)
  assert.equal(low.audit, 0)
  assert.match(low.calls[1].check.stdout, /moderate=0 low=1/)
})

test('malformed, missing or truncated registry report fails closed', () => {
  for (const payload of ['{', '', JSON.stringify({ metadata: {} }), JSON.stringify(vulnerabilities(0, 0, '0', 0))]) {
    const { audit } = trial(payload, payload === '' ? 0 : 1)
    assert.equal(audit, 2)
  }
})

test('nonzero pnpm status without a real advisory is not accepted as a clean audit', () => {
  const { audit } = trial(JSON.stringify(vulnerabilities()), 1)
  assert.equal(audit, 2)
})

test('package-manager invocation failures never reach checker or leak raw output', () => {
  for (const failed of [
    { status: 2, stdout: JSON.stringify(vulnerabilities()) },
    { status: null, stdout: JSON.stringify(vulnerabilities()), error: Error('timed out') },
    { status: 0, stdout: null },
  ]) {
    let calls = 0
    const result = runFreeReleaseDependencyAudit((command) => {
      calls++
      assert.equal(command, 'pnpm')
      return failed
    })
    assert.equal(result, 2)
    assert.equal(calls, 1)
  }
})

test('release audit wrapper rejects caller-provided flags', () => {
  const result = spawnSync(process.execPath, [cli, '--allow-moderate'], { encoding: 'utf8' })
  assert.equal(result.status, 2)
  assert.match(result.stderr, /no options/)
})
