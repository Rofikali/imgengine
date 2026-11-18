// Temporary, deliberately narrow CI decision. Not a vulnerability fix or release authorization.
export const TEMPORARY_PICOMATCH_GHSA = 'GHSA-fc5q-39xc-vjpf'
export const TEMPORARY_PICOMATCH_EXPIRES = '2026-12-02T00:00:00.000Z'
export const TEMPORARY_PICOMATCH_OPT_IN = '--allow-temporary-picomatch-in-frontend-ci'
const EXPECTED_ADVISORY_URL = `https://github.com/advisories/${TEMPORARY_PICOMATCH_GHSA}`
const EXPIRY_MS = Date.parse(TEMPORARY_PICOMATCH_EXPIRES)

export function assessTemporaryPicomatchWaiver(report, {
  critical,
  high,
  nowMs = Date.now(),
  env = process.env,
} = {}) {
  const reject = (reason) => ({ allowed: false, reason })
  if (!Number.isFinite(nowMs) || nowMs >= EXPIRY_MS) {
    return reject('expired on 2026-12-02 UTC or invalid clock; renew only by separate security review')
  }
  // This workflow runs PR and main verification, NOT a production deployment.
  if (
    env.GITHUB_ACTIONS !== 'true' ||
    env.GITHUB_WORKFLOW !== 'imgengine-native-ci' ||
    env.GITHUB_JOB !== 'frontend-verification' ||
    !['push', 'pull_request'].includes(env.GITHUB_EVENT_NAME)
  ) {
    return reject('only the IMGENGINE Nuxt verification job on GitHub Actions is eligible')
  }
  // Current picomatch advisory impacts two locked dependency paths. Fail closed
  // if their count changes or any critical/new high finding is reported.
  if (critical !== 0 || high !== 2) {
    return reject('severity totals changed: only the reviewed two high findings are eligible')
  }
  const entries = report?.advisories
  if (!entries || typeof entries !== 'object' || Array.isArray(entries)) {
    return reject('missing or invalid detailed audit advisories')
  }
  const severe = Object.values(entries).filter(
    (a) => a && (a.severity === 'high' || a.severity === 'critical'),
  )
  if (severe.length !== 1) {
    return reject('unexpected number of high/critical advisory records')
  }
  const advisory = severe[0]
  if (
    advisory.severity !== 'high' ||
    (advisory.module_name ?? advisory.name) !== 'picomatch' ||
    advisory.url !== EXPECTED_ADVISORY_URL ||
    advisory.patched_versions !== '<0.0.0'
  ) {
    return reject('advisory changed, a different package is affected, or upstream patch is reported')
  }
  return { allowed: true, reason: 'one exact unpatched picomatch advisory accepted for non-production frontend CI only' }
}
