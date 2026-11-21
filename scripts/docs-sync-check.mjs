import { execFileSync } from 'node:child_process'
import { pathToFileURL } from 'node:url'

export const REQUIRED_DOCS = Object.freeze(['AGENTS.md', 'next.md', 'README.md', 'skills.md'])
export const REQUIRED_WORK_LOG = 'docs/engineering/live-status.md'

export function parseDiff(raw) {
  if (typeof raw !== 'string' || !raw.endsWith('\0')) throw Error('invalid NUL-delimited git diff')
  const fields = raw.slice(0, -1).split('\0')
  if (fields.length % 2 || fields.length > 40000) throw Error('malformed or oversized diff')
  const changes = []
  for (let i = 0; i < fields.length; i += 2) {
    const status = fields[i], path = fields[i + 1]
    if (!/^[AMDT]$/.test(status)) throw Error('unsupported diff status: ' + status)
    if (!path || path.length > 2048 || /[\\\x00-\x1f\x7f]/.test(path) ||
        path.startsWith('/') || path.split('/').some(x => !x || x === '.' || x === '..'))
      throw Error('unsafe diff path')
    changes.push({ status, path })
  }
  return changes
}

export function checkDocsSync(changes) {
  if (!Array.isArray(changes)) throw Error('changes must be an array')
  if (!changes.length) return { mode: 'empty', count: 0 }
  const required = [...REQUIRED_DOCS, REQUIRED_WORK_LOG]
  const missing = required.filter(path => !changes.some(c => c.path === path && ['A', 'M'].includes(c.status)))
  if (missing.length) throw Error('agent-owned documentation missing in same change: ' + missing.join(', '))
  if (changes.some(c => required.includes(c.path) && c.status === 'D')) throw Error('required agent documentation deleted')
  return { mode: 'engineer-updated', count: changes.length }
}

export function readDiff(base, head) {
  if (!/^(?:HEAD(?:\^)?|[a-f0-9]{40})$/.test(base) ||
      !/^(?:HEAD(?:\^)?|[a-f0-9]{40})$/.test(head)) throw Error('unsafe revision argument')
  return execFileSync('git', ['diff', '--name-status', '-z', '--no-renames', base, head, '--'],
    { encoding: 'utf8', timeout: 30000, maxBuffer: 4 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'] })
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    if (process.argv.length !== 4) throw Error('usage: node scripts/docs-sync-check.mjs HEAD^ HEAD')
    const result = checkDocsSync(parseDiff(readDiff(process.argv[2], process.argv[3])))
    console.log('DOCSYNC PASS: ' + result.mode + ', changed paths=' + result.count)
  } catch (error) {
    console.error('::error::DOCSYNC FAIL: ' + error.message)
    process.exitCode = 1
  }
}
