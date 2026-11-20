import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { mkdtempSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { test } from 'node:test'
const scriptPath = fileURLToPath(new URL('./check-production-audit.mjs', import.meta.url))

function audit(report, { args = [], env = {} } = {}) {
  const directory = mkdtempSync(join(tmpdir(), 'imgengine-audit-'))
  try {
    const path = join(directory, 'audit.json')
    writeFileSync(path, JSON.stringify(report))
    return spawnSync(process.execPath, [scriptPath, path, ...args], { encoding: 'utf8', env: { ...process.env, ...env } })
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

test('moderate production advisories print names, IDs and available fixes without weakening CI', () => {
  const result = audit(report(
    { critical: 0, high: 0, moderate: 2, low: 0 },
    {
      glob: { severity: 'moderate', module_name: 'glob', url: 'https://github.com/advisories/GHSA-aaaa-bbbb-cccc', patched_versions: '>=13.0.0' },
      minimatch: { severity: 'moderate', module_name: 'minimatch', url: 'https://github.com/advisories/GHSA-dddd-eeee-ffff', patched_versions: null },
    },
  ))
  assert.equal(result.status, 0)
  assert.match(result.stdout, /critical=0 high=0 moderate=2 low=0/)
  assert.match(result.stdout, /severity=moderate package=glob id=GHSA-aaaa-bbbb-cccc patched=>=13\.0\.0/)
  assert.match(result.stdout, /severity=moderate package=minimatch id=GHSA-dddd-eeee-ffff patched=no published fix reported/)
})

test('missing moderate details cannot be mistaken for a clean release audit', () => {
  const result = audit(report({ critical: 0, high: 0, moderate: 2, low: 0 }))
  assert.equal(result.status, 0) // Existing CI severity policy is unchanged.
  assert.match(result.stdout, /Moderate advisory detail unavailable/)
  assert.match(result.stdout, /Free release review remains pending/)
})

test('moderate advisory names are sanitized before appearing in public CI logs', () => {
  const result = audit(report(
    { critical: 0, high: 0, moderate: 1, low: 0 },
    { control: { severity: 'moderate', module_name: 'foo\nbar', url: 'https://example.test/', patched_versions: 'unknown\rfix' } },
  ))
  assert.equal(result.status, 0)
  assert.match(result.stdout, /severity=moderate package=foo bar id=GHSA-not-provided patched=unknown fix/)
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

const githubFrontendContext = {
  GITHUB_ACTIONS: 'true',
  GITHUB_WORKFLOW: 'imgengine-native-ci',
  GITHUB_JOB: 'frontend-verification',
  GITHUB_EVENT_NAME: 'pull_request',
}

test('withdrawn picomatch finding is not a privileged CI bypass', () => {
  const result = audit(report(
    { critical: 0, high: 2, moderate: 2, low: 0 },
    { picomatch: {
      severity: 'high',
      module_name: 'picomatch',
      url: 'https://github.com/advisories/GHSA-fc5q-39xc-vjpf',
      patched_versions: '<0.0.0',
    } },
  ), { env: githubFrontendContext })
  assert.equal(result.status, 1)
  assert.match(result.stdout, /severity=high package=picomatch/)
  assert.match(result.stderr, /require remediation/)
})

test('every critical or high finding fails even in GitHub frontend CI', () => {
  for (const counts of [
    { critical: 1, high: 0, moderate: 0, low: 0 },
    { critical: 0, high: 1, moderate: 0, low: 0 },
    { critical: 1, high: 2, moderate: 2, low: 0 },
  ]) {
    const result = audit(report(counts), { env: githubFrontendContext })
    assert.equal(result.status, 1)
    assert.match(result.stderr, /require remediation/)
  }
})

test('unknown and retired picomatch audit flags fail closed', () => {
  for (const flag of ['--ignore-all-high', '--allow-temporary-picomatch-in-frontend-ci']) {
    const result = audit(report(
      { critical: 0, high: 0, moderate: 2, low: 0 },
    ), { args: [flag], env: githubFrontendContext })
    assert.equal(result.status, 2)
    assert.match(result.stderr, /unrecognized audit option/)
  }
})


test('release-candidate mode passes only a clean moderate-or-worse audit', () => {
  const result = audit(report({ critical: 0, high: 0, moderate: 0, low: 1 }), {
    args: ['--release-candidate'],
  })
  assert.equal(result.status, 0)
  assert.match(result.stdout, /release-candidate dependency threshold PASSED/)
  assert.match(result.stdout, /critical=0 high=0 moderate=0 low=1/)
})

test('release-candidate mode refuses moderate production findings with complete identities', () => {
  const result = audit(report(
    { critical: 0, high: 0, moderate: 2, low: 0 },
    {
      a: { severity: 'moderate', module_name: 'glob', url: 'https://github.com/advisories/GHSA-3g3m-2fjr-6pw4', patched_versions: '>=14.0.2' },
      b: { severity: 'moderate', module_name: 'minimatch', url: 'https://github.com/advisories/GHSA-g3hv-75wg-m249', patched_versions: '>=11.0.1' },
    },
  ), { args: ['--release-candidate'] })
  assert.equal(result.status, 1)
  assert.match(result.stdout, /severity=moderate package=glob/)
  assert.match(result.stdout, /severity=moderate package=minimatch/)
  assert.match(result.stderr, /release-candidate audit BLOCKED/)
  assert.doesNotMatch(result.stdout, /threshold PASSED/)
})

test('release-candidate mode refuses moderate summary when advisory details are missing', () => {
  const result = audit(report({ critical: 0, high: 0, moderate: 1, low: 0 }), {
    args: ['--release-candidate'],
  })
  assert.equal(result.status, 1)
  assert.match(result.stdout, /Moderate advisory detail unavailable/)
  assert.match(result.stderr, /release-candidate audit BLOCKED/)
})

test('release-candidate mode never overrides a high or critical finding', () => {
  for (const counts of [
    { critical: 1, high: 0, moderate: 0, low: 0 },
    { critical: 0, high: 1, moderate: 2, low: 0 },
  ]) {
    const result = audit(report(counts), { args: ['--release-candidate'] })
    assert.equal(result.status, 1)
    assert.match(result.stderr, /critical or high/)
  }
})

test('release-candidate option cannot combine with bypass or duplicates', () => {
  for (const flags of [
    ['--release-candidate', '--ignore-all-high'],
    ['--release-candidate', '--release-candidate'],
    ['--ignore-all-high', '--release-candidate'],
  ]) {
    const result = audit(report({ critical: 0, high: 0, moderate: 0, low: 0 }), {
      args: flags,
    })
    assert.equal(result.status, 2)
    assert.match(result.stderr, /unrecognized audit option/)
  }
})

test('release-candidate mode rejects malformed audit summaries', () => {
  const result = audit(report({ critical: 0, high: 0, moderate: '0', low: 0 }), {
    args: ['--release-candidate'],
  })
  assert.equal(result.status, 2)
  assert.match(result.stderr, /invalid vulnerability summary/)
})
