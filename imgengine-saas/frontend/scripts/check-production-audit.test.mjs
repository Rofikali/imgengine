import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { mkdtempSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { test } from 'node:test'

const scriptPath = fileURLToPath(new URL('./check-production-audit.mjs', import.meta.url))

function audit(report) {
  const directory = mkdtempSync(join(tmpdir(), 'imgengine-audit-'))
  try {
    const path = join(directory, 'audit.json')
    writeFileSync(path, JSON.stringify(report))
    return spawnSync(process.execPath, [scriptPath, path], { encoding: 'utf8' })
  } finally {
    rmSync(directory, { recursive: true, force: true })
  }
}

function report(counts, advisories = {}) {
  return { metadata: { vulnerabilities: counts }, advisories }
}

test('clean production audit passes', () => {
  const result = audit(report({ critical: 0, high: 0, moderate: 0, low: 0 }))
  assert.equal(result.status, 0)
  assert.match(result.stdout, /critical=0 high=0/)
})

test('known high advisory outputs name, GHSA and fix state, and still fails', () => {
  const result = audit(report(
    { critical: 0, high: 2, moderate: 2, low: 0 },
    { example: { severity: 'high', module_name: 'picomatch', url: 'https://github.com/advisories/GHSA-fc5q-39xc-vjpf', patched_versions: '<0.0.0' } },
  ))
  assert.equal(result.status, 1)
  assert.match(result.stdout, /severity=high package=picomatch id=GHSA-fc5q-39xc-vjpf patched=<0\.0\.0/)
  assert.match(result.stderr, /require remediation/)
})

test('missing detailed advisories cannot bypass high severity failure', () => {
  const result = audit(report({ critical: 0, high: 1, moderate: 0, low: 0 }))
  assert.equal(result.status, 1)
  assert.match(result.stdout, /summary remains blocking/)
})

test('invalid vulnerability count is rejected', () => {
  const result = audit(report({ critical: 0, high: '1', moderate: 0, low: 0 }))
  assert.equal(result.status, 2)
  assert.match(result.stderr, /invalid vulnerability summary/)
})

test('advisory metadata is sanitized before logging', () => {
  const result = audit(report(
    { critical: 1, high: 0, moderate: 0, low: 0 },
    { suspicious: { severity: 'critical', module_name: 'bad\ncontrol', url: 'https://github.com/advisories/GHSA-abcd-1234-wxyz', patched_versions: null } },
  ))
  assert.equal(result.status, 1)
  assert.match(result.stdout, /package=bad control/)
  assert.match(result.stdout, /no published fix reported/)
})
