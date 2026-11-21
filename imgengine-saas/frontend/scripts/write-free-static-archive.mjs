// F5-RC-ARCHIVE-01: reproducible, DETACHED Free static candidate archive.
// Evidence-only; never publishes a site, signs an approval, or creates a tag.
// Require the existing static allowlist/CSP/privacy audit before and after tar.
import { createHash } from 'node:crypto'
import { spawnSync } from 'node:child_process'
import {
  closeSync, existsSync, lstatSync, mkdirSync, mkdtempSync, openSync,
  readSync, readdirSync, realpathSync, rmSync, writeFileSync,
} from 'node:fs'
import { tmpdir } from 'node:os'
import { dirname, join, relative, resolve, sep } from 'node:path'
import { pathToFileURL } from 'node:url'
import { auditFreeStatic } from './verify-free-static.mjs'

const SHA = /^[a-f0-9]{40}$/
const SHA256 = /^[a-f0-9]{64}$/
const ARCHIVE = 'candidate-static.tar'
const MANIFEST = 'candidate-archive.json'

function isInside(root, target) {
  const r = relative(root, target)
  return !r || (r !== '..' && !r.startsWith('..' + sep) && !r.startsWith('../'))
}

function filesAt(root) {
  const result = []
  function visit(dir) {
    for (const ent of readdirSync(dir, { withFileTypes: true })) {
      const full = join(dir, ent.name)
      if (lstatSync(full).isSymbolicLink()) throw Error('Symlink in static candidate')
      if (ent.isDirectory()) visit(full)
      else if (ent.isFile()) result.push(relative(root, full).split(sep).join('/'))
      else throw Error('Non-file in static candidate')
    }
  }
  visit(root)
  return result.sort()
}

function digestFile(path) {
  const sha = createHash('sha256')
  const file = openSync(path, 'r')
  const buffer = Buffer.alloc(1024 * 1024)
  try {
    for (;;) {
      const n = readSync(file, buffer, 0, buffer.length, null)
      if (n === 0) break
      sha.update(buffer.subarray(0, n))
    }
  } finally {
    closeSync(file)
  }
  return sha.digest('hex')
}

function checkedTar(args, options = {}) {
  const r = spawnSync('tar', args, {
    encoding: 'utf8', shell: false, timeout: 300_000,
    maxBuffer: 64 * 1024, ...options,
  })
  if (r.error || r.status !== 0) throw Error('Deterministic tar creation/verification failed')
}

// The resulting archive is an UNAPPROVED CANDIDATE, never a signed release
// artifact. It must be retained and verified separately before publication.
export function writeFreeArchiveCandidate(staticRoot, detachedDir, sourceSha) {
  if (!SHA.test(sourceSha || '')) throw Error('Missing immutable 40-hex source SHA')
  const root = realpathSync(resolve(staticRoot))
  if (!lstatSync(root).isDirectory()) throw Error('Invalid static output root')
  const dir = resolve(detachedDir)
  if (isInside(root, dir)) throw Error('Detached archive must be outside static output')
  if (existsSync(join(dir, ARCHIVE)) || existsSync(join(dir, MANIFEST))) {
    throw Error('Refusing to overwrite existing candidate archive/evidence')
  }
  const verified = auditFreeStatic(root)
  const names = filesAt(root)
  if (names.length !== verified.fileCount || names.length === 0) {
    throw Error('Candidate file count changed after static audit')
  }

  mkdirSync(dir, { recursive: true, mode: 0o700 })
  if (realpathSync(dir) !== dir) throw Error('Detached archive parent must not be a symlink')
  const archive = join(dir, ARCHIVE)
  const evidence = join(dir, MANIFEST)
  let created = false
  try {
    const out = openSync(archive, 'wx', 0o600)
    created = true
    try {
      // Null-delimited explicit file inventory, no recursive directory entries.
      // No unapproved empty directories or filename-as-command interpolation.
      // GNU tar normalization removes source timestamps, ownership and
      // unstable PAX atime/ctime from the archive hash.
      checkedTar([
        '-C', root, '--null', '--verbatim-files-from', '--no-recursion',
        '--format=posix', '--mtime=@0', '--owner=0', '--group=0',
        '--numeric-owner', '--pax-option=delete=atime,delete=ctime',
        '-cf', '-', '-T', '-',
      ], {
        input: Buffer.from(names.map(name => name + '\0').join('')),
        stdio: ['pipe', out, 'pipe'],
      })
    } finally {
      closeSync(out)
    }

    // Verify the complete serialized archive with the SAME policy (including
    // all nine routes, _headers, true 404, no Functions/server or symlinks).
    const extractDir = mkdtempSync(join(tmpdir(), 'imgengine-free-archive-check-'))
    try {
      checkedTar(['-xf', archive, '-C', extractDir, '--no-same-owner', '--no-same-permissions'])
      const fromArchive = auditFreeStatic(extractDir)
      const afterBuild = auditFreeStatic(root)
      for (const field of ['staticTreeSha256', 'fileCount', 'totalBytes']) {
        if (verified[field] !== fromArchive[field] || verified[field] !== afterBuild[field]) {
          throw Error('Archive/source does not match approved static audit')
        }
      }
    } finally {
      rmSync(extractDir, { recursive: true, force: true })
    }
    const archiveSha256 = digestFile(archive)
    if (!SHA256.test(archiveSha256)) throw Error('Invalid archive checksum')
    const manifest = {
      schema_version: 1,
      kind: 'imgengine-free-static-archive-candidate',
      status: 'CANDIDATE_NOT_APPROVED',
      product: 'imgengine-free',
      version: '1.0.0',
      source_sha: sourceSha,
      build_environment: 'ubuntu-24.04-playwright-1.58.2-node-24-pnpm-10.34.9',
      archive_filename: ARCHIVE,
      archive_format: 'normalized-gnu-tar-posix',
      archive_sha256: archiveSha256,
      static_tree_sha256: verified.staticTreeSha256,
      file_count: verified.fileCount,
      total_bytes: verified.totalBytes,
      routes: [...verified.routes],
      signed_provenance: null,
      sbom_and_licenses_approved: null,
      owner_approval: null,
      cloudflare_deployment: null,
      final_release_tag: null,
    }
    writeFileSync(evidence, JSON.stringify(manifest, null, 2) + '\n', {
      encoding: 'utf8', flag: 'wx', mode: 0o600,
    })
    return manifest
  } catch (error) {
    if (created) rmSync(archive, { force: true })
    throw error
  }
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  try {
    if (process.argv.length !== 5) {
      throw Error('Usage: node scripts/write-free-static-archive.mjs STATIC_ROOT DETACHED_DIR EXACT_SOURCE_SHA')
    }
    const manifest = writeFreeArchiveCandidate(process.argv[2], process.argv[3], process.argv[4])
    console.log('REVIEW-ONLY archive checksum: ' + JSON.stringify({
      source_sha: manifest.source_sha,
      archive_sha256: manifest.archive_sha256,
      static_tree_sha256: manifest.static_tree_sha256,
      status: manifest.status,
    }))
  } catch (error) {
    console.error('::error::Free candidate archive BLOCKED: ' + error.message)
    process.exitCode = 1
  }
}
