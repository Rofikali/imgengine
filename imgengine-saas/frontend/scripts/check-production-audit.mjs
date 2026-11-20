import { readFile } from 'node:fs/promises'
import {
  TEMPORARY_PICOMATCH_EXPIRES,
  TEMPORARY_PICOMATCH_OPT_IN,
  assessTemporaryPicomatchWaiver,
} from './picomatch-temporary-waiver.mjs'

const [reportPath, ...flags] = process.argv.slice(2)
const temporarilyAllowPicomatch = flags.length === 1 && flags[0] === TEMPORARY_PICOMATCH_OPT_IN
if (flags.length && !temporarilyAllowPicomatch) {
  console.error('unrecognized audit option; refusing to skip the security gate')
  process.exit(2)
}
if (!reportPath) {
  console.error('usage: check-production-audit.mjs AUDIT_REPORT.json')
  process.exit(2)
}

let report
try {
  // PowerShell commonly writes UTF-8 reports with a BOM; CI's shell redirect
  // does not, but accepting both keeps the gate portable.
  report = JSON.parse((await readFile(reportPath, 'utf8')).replace(/^\uFEFF/, ''))
} catch (error) {
  console.error(`cannot parse pnpm production audit report: ${error.message}`)
  process.exit(2)
}

const vulnerabilities = report?.metadata?.vulnerabilities
if (!vulnerabilities || typeof vulnerabilities !== 'object') {
  console.error('pnpm production audit report has no vulnerability summary')
  process.exit(2)
}

const count = (severity) => {
  const value = vulnerabilities[severity]
  return Number.isSafeInteger(value) && value >= 0 ? value : null
}

const critical = count('critical')
const high = count('high')
const moderate = count('moderate')
const low = count('low')
if ([critical, high, moderate, low].some((value) => value === null)) {
  console.error('pnpm production audit report has an invalid vulnerability summary')
  process.exit(2)
}

console.log(`Production dependency audit: critical=${critical} high=${high} moderate=${moderate} low=${low}`)


 // Keep the security gate strict, but make the next unpatched finding diagnosable
 // from the failed CI step without downloading a full private audit artifact.
const advisories = report?.advisories
const auditEntries = advisories && typeof advisories === 'object'
  ? Object.values(advisories).filter((item) => item && typeof item === 'object')
  : []
const severeAdvisories = auditEntries.filter((item) => item.severity === 'critical' || item.severity === 'high')
const moderateAdvisories = auditEntries.filter((item) => item.severity === 'moderate')

// Moderate findings do not change the existing CI severity threshold. Their
// identities must still be visible to reviewers before any Free release
// decision, without storing the full raw audit report in public job output.
const safeLabel = (value) => String(value ?? 'unknown').replace(/[\u0000-\u001f\u007f]/g, ' ').slice(0, 160)
for (const item of [...severeAdvisories.slice(0, 12), ...moderateAdvisories.slice(0, 12)]) {
  const ghsa = String(item.url ?? '').match(/GHSA-[a-z0-9-]+/i)?.[0] ?? 'GHSA-not-provided'
  const packageName = safeLabel(item.module_name ?? item.name)
  const patchedVersions = safeLabel(item.patched_versions ?? 'no published fix reported')
  console.log(`Production advisory: severity=${item.severity} package=${packageName} id=${ghsa} patched=${patchedVersions}`)
}
if ((critical > 0 || high > 0) && severeAdvisories.length === 0) {
  console.log('Production advisory detail unavailable in audit JSON; summary remains blocking')
}
if (severeAdvisories.length > 12) {
  console.log(`Production advisory detail truncated: ${severeAdvisories.length} total high/critical advisory records`)
}
if (moderate > 0 && moderateAdvisories.length === 0) {
  console.log('Moderate advisory detail unavailable in audit JSON; Free release review remains pending')
}
if (moderateAdvisories.length > 12) {
  console.log(`Moderate advisory detail truncated: ${moderateAdvisories.length} total moderate advisory records`)
}

if (critical > 0 || high > 0) {
  if (temporarilyAllowPicomatch) {
    const decision = assessTemporaryPicomatchWaiver(report, { critical, high })
    if (decision.allowed) {
      console.warn(`TEMPORARY NON-PRODUCTION SECURITY EXCEPTION: picomatch GHSA-fc5q-39xc-vjpf remains VULNERABLE; allowed in this frontend CI job ONLY; expires ${TEMPORARY_PICOMATCH_EXPIRES}. No production release is approved.`)
      process.exit(0)
    }
    console.error(`Temporary picomatch exception REJECTED: ${decision.reason}`)
  }
  console.error('critical or high production dependency advisories require remediation or an approved release decision')
  process.exit(1)
}
