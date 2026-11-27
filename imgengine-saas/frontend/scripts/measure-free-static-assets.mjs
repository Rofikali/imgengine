// F6-00: detached, read-only size evidence. Never authorizes publication.
import { lstatSync, readdirSync, readFileSync } from 'node:fs'
import { join, relative, resolve, sep } from 'node:path'
import { pathToFileURL } from 'node:url'
import { gzipSync, brotliCompressSync, constants } from 'node:zlib'
import { auditFreeStatic } from './verify-free-static.mjs'

const MAX_FILE_BYTES = 25 * 1024 * 1024
const MAX_FILES = 20000

// Each asset compressed separately (as independent HTTP responses).
// This is not a route waterfall, CDN transfer measurement or runtime metric.
export function measureAssetBytes(data) {
  if (!Buffer.isBuffer(data) || data.length > MAX_FILE_BYTES) throw Error('Invalid asset buffer')
  return {
    raw: data.length,
    gzip: gzipSync(data, { level: 6 }).length,
    brotli: brotliCompressSync(data, { params: { [constants.BROTLI_PARAM_QUALITY]: 5 } }).length,
  }
}

export function inventoryAssetSizes(root) {
  const dist = resolve(root)
  const groups = { css: { files: 0, raw: 0, gzip: 0, brotli: 0 }, js: { files: 0, raw: 0, gzip: 0, brotli: 0 } }
  const records = []
  let count = 0
  function visit(path) {
    const stat = lstatSync(path)
    if (stat.isSymbolicLink()) throw Error('Symlink in static output')
    if (++count > MAX_FILES * 2) throw Error('Too many static entries')
    if (stat.isDirectory()) {
      for (const name of readdirSync(path).sort()) visit(join(path, name))
      return
    }
    if (!stat.isFile() || stat.size > MAX_FILE_BYTES) throw Error('Invalid static file')
    const kind = /\.css$/i.test(path) ? 'css' : /\.m?js$/i.test(path) ? 'js' : null
    if (!kind) return
    const bytes = measureAssetBytes(readFileSync(path))
    const group = groups[kind]
    group.files++
    for (const key of ['raw', 'gzip', 'brotli']) group[key] += bytes[key]
    records.push({ path: relative(dist, path).split(sep).join('/'), kind, ...bytes })
  }
  if (!lstatSync(dist).isDirectory()) throw Error('Invalid static root')
  visit(dist)
  if (!groups.css.files || !groups.js.files) throw Error('Missing compiled CSS or JavaScript')
  return { groups, assets: records }
}

export function measureFreeStatic(root, sourceSha) {
  if (!/^[a-f0-9]{40}$/.test(sourceSha || '') || /^0{40}$/.test(sourceSha)) throw Error('Exact source SHA required')
  const before = auditFreeStatic(root)
  const inventory = inventoryAssetSizes(root)
  const after = auditFreeStatic(root)
  if (before.staticTreeSha256 !== after.staticTreeSha256) throw Error('Static output changed during measurement')
  return {
    kind: 'free-static-size-inventory-not-release-approval',
    claimed_source_sha: sourceSha,
    static_tree_sha256: before.staticTreeSha256,
    node_version: process.version,
    zlib_version: process.versions.zlib,
    brotli_version: process.versions.brotli,
    compression: { gzip_level: 6, brotli_quality: 5 },
    static_file_count: before.fileCount,
    static_raw_bytes: before.totalBytes,
    ...inventory,
  }
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  try {
    if (process.argv.length !== 4) throw Error('Usage: node scripts/measure-free-static-assets.mjs STATIC_ROOT EXACT_SOURCE_SHA')
    console.log(JSON.stringify(measureFreeStatic(process.argv[2], process.argv[3]), null, 2))
  } catch (error) {
    console.error('Static measurement rejected: ' + error.message)
    process.exitCode = 1
  }
}
