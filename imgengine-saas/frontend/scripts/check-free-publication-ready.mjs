import { readFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'

// This is a pre-launch guard, NOT an automated production-deployment service.
// Final source/artifact/owner evidence MUST be generated out-of-tree after freeze,
// never stored as a self-referencing SHA in the tracked release-plan JSON.
const repoManifest = fileURLToPath(new URL('../../../docs/releases/free/v1.0.0/release-manifest.json', import.meta.url))
const plan = JSON.parse(readFileSync(repoManifest, 'utf8'))
const notices = ['privacy', 'terms', 'support']
const failures = []

for (const notice of notices) {
  const source = fileURLToPath(new URL('../free-static/' + notice + '.vue', import.meta.url))
  const content = readFileSync(source, 'utf8')
  if (/PRE-LAUNCH DRAFT|Launch pending:|Launch blocker:|Not yet approved:/i.test(content)) {
    failures.push(notice + ' notice has unapproved draft language')
  }
}
if (plan.publicly_released !== false) failures.push('source release plan must not already claim public release')
if (plan.state === 'RELEASED') failures.push('cannot use pre-deployment verification after publication')

// Do not accept arbitrary in-repo metadata as authorization to deploy.
// A separate verified *post-freeze* approval/attestation system is a
// release-governance milestone that remains unimplemented.
failures.push('detached owner approval, exact artifact provenance, production legal/security and hosting verification are NOT implemented')

for (const reason of failures) console.error('PUBLICATION BLOCKED: ' + reason)
process.exitCode = 1
