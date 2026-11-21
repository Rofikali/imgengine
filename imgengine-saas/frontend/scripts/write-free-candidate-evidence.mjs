// F5-RC-01 — independent, review-only Free candidate provenance.
// Never writes to the source distribution and never authorizes a deployment.
// The existing auditFreeStatic owns the static allowlist, privacy/CSP boundary,
// _headers contract, route checks, size limits and symlink rejection.
import { existsSync, mkdirSync, realpathSync, writeFileSync } from 'node:fs'
import { dirname, relative, resolve, sep } from 'node:path'
import { pathToFileURL } from 'node:url'
import { auditFreeStatic } from './verify-free-static.mjs'

const SHA = /^[a-f0-9]{40}$/
const SHA256 = /^[a-f0-9]{64}$/
const ROUTES = Object.freeze(['/', '/resize-image', '/crop-image', '/compress-image',
  '/rotate-image', '/convert-image', '/privacy', '/terms', '/support'])

export function createCandidateManifest(sourceSha, verified) {
  if (!SHA.test(sourceSha || '')) throw Error('Missing exact immutable Git source SHA')
  if (!verified || !SHA256.test(verified.staticTreeSha256 || '') ||
      !Number.isSafeInteger(verified.fileCount) || verified.fileCount < 1 ||
      !Number.isSafeInteger(verified.totalBytes) || verified.totalBytes < 1 ||
      !Array.isArray(verified.routes) || verified.routes.length !== ROUTES.length ||
      verified.routes.some((r, i) => r !== ROUTES[i])) {
    throw Error('Unverified or malformed Free static content inventory')
  }
  return {
    schema_version: 1,
    kind: 'imgengine-free-static-candidate-evidence',
    product: 'imgengine-free',
    version: '1.0.0',
    status: 'CANDIDATE_NOT_APPROVED',
    source_sha: sourceSha,
    source_lockfile: 'imgengine-saas/frontend/pnpm-lock.yaml',
    output_directory: 'imgengine-saas/frontend/.output/public',
    build_environment: 'ubuntu-24.04-playwright-1.58.2-node-24-pnpm-10.34.9',
    static_tree_sha256: verified.staticTreeSha256,
    file_count: verified.fileCount,
    total_bytes: verified.totalBytes,
    static_routes: [...verified.routes],
    immutable_archive_sha256: null,
    owner_approval: null,
    legal_security_hosting_approval: null,
    cloudflare_deployment: null,
    final_release_tag: null
  }
}

export function assertDetachedLocation(staticRoot, output) {
  const root = realpathSync(resolve(staticRoot))
  const target = resolve(output)
  if (!target.endsWith('.json')) throw Error('Evidence output must be a detached .json file')
  const rel = relative(root, target)
  if (rel === '' || (!rel.startsWith('..' + sep) && rel !== '..' && !rel.startsWith('..' + '/'))) {
    throw Error('Candidate evidence must be outside the compiled static asset tree')
  }
  if (existsSync(target)) throw Error('Refusing to replace existing release-candidate evidence')
  return target
}

export function writeCandidateEvidence(staticRoot, output, sourceSha) {
  const target = assertDetachedLocation(staticRoot, output)
  const manifest = createCandidateManifest(sourceSha, auditFreeStatic(staticRoot))
  mkdirSync(dirname(target), { recursive: true, mode: 0o700 })
  if (realpathSync(dirname(target)) !== resolve(dirname(target))) {
    throw Error('Detached candidate evidence parent must not be a symlink')
  }
  writeFileSync(target, JSON.stringify(manifest, null, 2) + '\n', {
    flag: 'wx', mode: 0o600
  })
  return manifest
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  try {
    if (process.argv.length !== 5) throw Error('Usage: node scripts/write-free-candidate-evidence.mjs STATIC_ROOT DETACHED_EVIDENCE.json EXACT_SOURCE_SHA')
    const m = writeCandidateEvidence(process.argv[2], process.argv[3], process.argv[4])
    console.log('FREE CANDIDATE EVIDENCE: ' + JSON.stringify({
      source_sha: m.source_sha, static_tree_sha256: m.static_tree_sha256,
      file_count: m.file_count, total_bytes: m.total_bytes, status: m.status
    }))
  } catch (error) {
    console.error('::error::Candidate evidence generation BLOCKED: ' + error.message)
    process.exitCode = 1
  }
}
