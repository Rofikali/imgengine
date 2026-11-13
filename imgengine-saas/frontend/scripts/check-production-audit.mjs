import { readFile } from 'node:fs/promises'

const [reportPath] = process.argv.slice(2)
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
if (critical > 0 || high > 0) {
  console.error('critical or high production dependency advisories require remediation or an approved release decision')
  process.exit(1)
}
