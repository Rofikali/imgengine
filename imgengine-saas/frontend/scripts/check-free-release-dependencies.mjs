// Free release-candidate security preflight, NOT development CI or deployment.
// Collect registry audit output in a private temporary file and parse it through
// the same fail-closed checker as CI, but reject ALL moderate-or-higher findings.
// No registry response, credentials or package-manager output is printed raw.
import { spawnSync } from 'node:child_process'
import { mkdtempSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join, resolve } from 'node:path'
import { fileURLToPath, pathToFileURL } from 'node:url'

const checker = fileURLToPath(new URL('./check-production-audit.mjs', import.meta.url))
const maxAuditOutputBytes = 8 * 1024 * 1024

export function runFreeReleaseDependencyAudit(run = spawnSync) {
  const audited = run('pnpm', ['audit', '--prod', '--json'], {
    cwd: process.cwd(),
    encoding: 'utf8',
    maxBuffer: maxAuditOutputBytes,
    timeout: 90_000,
    shell: false,
  })

  if (audited.error || ![0, 1].includes(audited.status) ||
      typeof audited.stdout !== 'string' || !audited.stdout.trim()) {
    console.error('Free release dependency audit BLOCKED: package-manager audit failed or returned no usable JSON')
    return 2
  }

  // pnpm uses a nonzero status for advisories; never interpret that status
  // as an exemption from the review threshold. Empty/invalid JSON fails
  // in the checker and cannot be interpreted as zero findings.
  const folder = mkdtempSync(join(tmpdir(), 'imgengine-free-audit-'))
  try {
    const path = join(folder, 'audit.json')
    writeFileSync(path, audited.stdout, { encoding: 'utf8', mode: 0o600 })
    const checked = run(process.execPath, [checker, path, '--release-candidate'], {
      cwd: process.cwd(),
      stdio: 'inherit',
      timeout: 30_000,
      shell: false,
    })
    if (checked.error || !Number.isInteger(checked.status) ||
        ![0, 1, 2].includes(checked.status)) {
      console.error('Free release dependency audit BLOCKED: checker did not finish cleanly')
      return 2
    }
    // An audit command reporting an error with an otherwise clean JSON
    // summary is not proof of a clean registry scan. Reject the anomaly.
    if (audited.status !== 0 && checked.status === 0) {
      // pnpm may exit 1 on a LOW-only advisory, which the deliberately
      // moderate-or-higher release policy does not prohibit. Any other
      // inconsistent nonzero result is a tool failure, not a clean audit.
      let low
      try {
        low = JSON.parse(audited.stdout.replace(/^\uFEFF/, ''))?.metadata?.vulnerabilities?.low
      } catch {
        low = null
      }
      if (!Number.isSafeInteger(low) || low <= 0) {
        console.error('Free release dependency audit BLOCKED: nonzero pnpm status with an unsupported clean summary')
        return 2
      }
    }
    return checked.status
  } finally {
    rmSync(folder, { recursive: true, force: true })
  }
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  if (process.argv.length > 2) {
    console.error('usage: check-free-release-dependencies.mjs (no options)')
    process.exitCode = 2
  } else {
    process.exitCode = runFreeReleaseDependencyAudit()
  }
}
