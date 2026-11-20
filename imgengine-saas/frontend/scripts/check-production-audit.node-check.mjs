import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { mkdtempSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { test } from 'node:test'
import {
  TEMPORARY_PICOMATCH_OPT_IN,
  assessTemporaryPicomatchWaiver,
} from './picomatch-temporary-waiver.mjs'

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
const picomatchAdvisory = {
  severity: 'high',
  module_name: 'picomatch',
  url: 'https://github.com/advisories/GHSA-fc5q-39xc-vjpf',
  patched_versions: '<0.0.0',
}
const vulnerableReport = () => report(
  { critical: 0, high: 2, moderate: 2, low: 0 },
  { picomatch: { ...picomatchAdvisory } },
)

test('only opted-in non-production GitHub frontend CI permits exact picomatch advisory', () => {
  const result = audit(vulnerableReport(), {
    args: [TEMPORARY_PICOMATCH_OPT_IN],
    env: githubFrontendContext,
  })
  assert.equal(result.status, 0)
  assert.match(result.stderr, /TEMPORARY NON-PRODUCTION SECURITY EXCEPTION/)
  assert.match(result.stderr, /remains VULNERABLE/)
})

test('default audit remains strict even on GitHub frontend CI', () => {
  const result = audit(vulnerableReport(), { env: githubFrontendContext })
  assert.equal(result.status, 1)
})

test('waiver denied on non-frontend GitHub job or local environment', () => {
  for (const env of [
    { ...githubFrontendContext, GITHUB_JOB: 'native-fuzz' },
    { ...githubFrontendContext, GITHUB_ACTIONS: 'false' },
  ]) {
    const result = audit(vulnerableReport(), {
      args: [TEMPORARY_PICOMATCH_OPT_IN], env,
    })
    assert.equal(result.status, 1)
    assert.match(result.stderr, /exception REJECTED/)
  }
})

test('waiver denied on other high-severity advisory', () => {
  const altered = vulnerableReport()
  altered.advisories.other = {
    severity: 'high', module_name: 'other-package',
    url: 'https://github.com/advisories/GHSA-xxxx-yyyy-zzzz', patched_versions: '<0.0.0',
  }
  const decision = assessTemporaryPicomatchWaiver(altered, {
    critical: 0, high: 2, nowMs: Date.parse('2026-11-20T00:00:00Z'), env: githubFrontendContext,
  })
  assert.equal(decision.allowed, false)
})

test('waiver denied when upstream publishes a patched version', () => {
  const altered = vulnerableReport()
  altered.advisories.picomatch.patched_versions = '>=4.0.8'
  const decision = assessTemporaryPicomatchWaiver(altered, {
    critical: 0, high: 2, nowMs: Date.parse('2026-11-20T00:00:00Z'), env: githubFrontendContext,
  })
  assert.equal(decision.allowed, false)
})

test('waiver denied on critical findings, changed counts, or missing detail', () => {
  for (const [body, critical, high] of [
    [vulnerableReport(), 1, 2],
    [vulnerableReport(), 0, 3],
    [report({ critical: 0, high: 2, moderate: 2, low: 0 }), 0, 2],
  ]) {
    assert.equal(assessTemporaryPicomatchWaiver(body, {
      critical, high, nowMs: Date.parse('2026-11-20T00:00:00Z'), env: githubFrontendContext,
    }).allowed, false)
  }
})

test('waiver expires exactly on December 2 UTC', () => {
  assert.equal(assessTemporaryPicomatchWaiver(vulnerableReport(), {
    critical: 0, high: 2, nowMs: Date.parse('2026-12-01T23:59:59Z'), env: githubFrontendContext,
  }).allowed, true)
  assert.equal(assessTemporaryPicomatchWaiver(vulnerableReport(), {
    critical: 0, high: 2, nowMs: Date.parse('2026-12-02T00:00:00Z'), env: githubFrontendContext,
  }).allowed, false)
})

test('unknown audit options fail closed', () => {
  const result = audit(vulnerableReport(), {
    args: ['--ignore-all-high'], env: githubFrontendContext,
  })
  assert.equal(result.status, 2)
  assert.match(result.stderr, /unrecognized audit option/)
})
