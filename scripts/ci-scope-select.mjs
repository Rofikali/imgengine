import { execFileSync } from 'node:child_process'
import { appendFileSync, readFileSync } from 'node:fs'
import { pathToFileURL } from 'node:url'

export const JOBS = Object.freeze([
  'infra-p8-parser', 'native-build-test', 'native-sanitizers', 'native-fuzz',
  'saas-integration', 'frontend-verification', 'frontend-browser-quality',
  'rust-api-container', 'docker-linux-verification',
])
export const OUTPUT_KEYS = Object.freeze(Object.fromEntries(JOBS.map(id => [id, id.replaceAll('-', '_')])))
const ALL = new Set(JOBS)
const SHA = /^[a-f0-9]{40}$/

function result(jobs, reason, freeOnly = false) {
  const full = jobs.size === JOBS.length
  return { version: 'ci-scope-02-v1', full, reason, freeOnly, jobs: Object.fromEntries(JOBS.map(id => [id, jobs.has(id)])) }
}
export function full(reason = 'fail-closed') { return result(ALL, reason) }
const select = (...names) => new Set(names)
const FRONTEND = select('frontend-verification', 'frontend-browser-quality')
const HTTP = select(...FRONTEND, 'rust-api-container')

// Exact planning/agent Markdown paths only: executable policy and release/security docs remain full.
const PLANNING_DOCS = new Set([
  "AGENTS.md",
  "skills.md",
  "docs/engineering/live-status.md",
  "imgengine-saas/frontend/AGENTS.md",
  "docs/IMGENGINE_F6_SHARED_UI_DOMAIN_MONETIZATION_ROADMAP.md",
  "docs/IMGENGINE_FRONTEND_EXCELLENCE_PROGRAM_F6.md",
  "docs/IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md",
  "docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md"
])
const FREE_ONLY = new Set([
  'imgengine-saas/frontend/free-static/FreeSiteHeader.vue',
  'imgengine-saas/frontend/free-static/landing.vue',
  'imgengine-saas/frontend/free-static/tools.ts',
])
const FRONTEND_SHELL = new Set([
  "imgengine-saas/frontend/app.vue",
  "imgengine-saas/frontend/free-static/FreeSiteHeader.vue",
  "imgengine-saas/frontend/free-static/landing.vue",
  "imgengine-saas/frontend/free-static/tools.ts"
])

function impact(path) {
  if (PLANNING_DOCS.has(path)) return new Set()
  if (FRONTEND_SHELL.has(path)) return FRONTEND
  if (['README.md', 'next.md', 'next-history.md', 'docs/README.md'].includes(path) || /^docs\/notes\/[^/]+\.md$/.test(path)) return new Set()
  if (/^docs\/(FRONTEND_[A-Z0-9_]+|IMGENGINE_FRONTEND_[A-Z0-9_]+)\.md$/.test(path)) return FRONTEND
  if (path === 'docs/api.md' || /^docs\/P1[23]_[A-Z0-9_]+\.md$/.test(path)) return HTTP
  if (path.startsWith('docs/')) return null
  if (path.startsWith('imgengine/')) return null
  if (path.startsWith('backend/imgengine/')) return null
  if (path.startsWith('backend/imgengine-api/')) return select('rust-api-container')
  if (path.startsWith('backend/imgengine-supervisor/')) return select('rust-api-container', 'native-build-test')
  if (path.startsWith('backend/')) return null
  if (path.startsWith('imgengine-saas/backend/') || path.startsWith('imgengine-saas/infra/')) return null
  if (path.startsWith('imgengine-saas/frontend/')) {
    const rest = path.slice('imgengine-saas/frontend/'.length)
    if (rest === 'AGENTS.md' || rest.includes('lock') || /(^|\/)(Dockerfile[^/]*|[^/]+\.config\.[cm]?[jt]s|package\.json)$/.test(rest)) return null
    if (/^(pages|components|composables|utils|tests|types|assets|public|server)\//.test(rest)) {
      if (/^server\//.test(rest) || /^(types\/api\.ts|utils\/render-form\.ts|composables\/useImgengineApi\.ts)$/.test(rest)) return HTTP
      return FRONTEND
    }
    return null
  }
  if (path.startsWith('.github/') || path.startsWith('infra/') || path.startsWith('scripts/')) return null
  // No approved isolated browser/Wasm lane exists yet. Any unknown path is full.
  return null
}

export function classifyDiff(raw) {
  if (typeof raw !== 'string' || !raw.endsWith('\0')) return full('invalid-diff')
  const entries = raw.slice(0, -1).split('\0')
  if (entries.length < 2 || entries.length > 20000 || entries.length % 2) return full('empty-large-or-malformed-diff')
  const selected = new Set()
  let hasFree = false
  let onlyFree = true
  for (let i = 0; i < entries.length; i += 2) {
    const [status, path] = [entries[i], entries[i + 1]]
    // Renames, deletions, submodules and unresolved conflicts all fail closed.
    if (!['A', 'M'].includes(status)) return full('unsupported-git-status')
    if (!path || path.length > 1024 || /[\\\x00-\x1f\x7f]/.test(path) || path.startsWith('/') || path.split('/').some(p => !p || p === '.' || p === '..')) return full('unsafe-git-path')
    const required = impact(path)
    if (required === null) return full('shared-policy-unknown-or-high-risk')
    if (status === 'M' && FREE_ONLY.has(path)) hasFree = true
    else if (required.size !== 0) onlyFree = false
    for (const name of required) selected.add(name)
  }
  return result(selected, selected.size ? 'impacted-boundaries' : 'docs-only', hasFree && onlyFree)
}

function validateSha(value) {
  if (!SHA.test(value ?? '') || /^0{40}$/.test(value)) throw Error('invalid SHA')
  return value
}

export function selectFromEvent(eventName, event, git) {
  if (eventName === 'push' || eventName === 'merge_group') return full('push-or-merge-queue-full')
  if (eventName !== 'pull_request') return full('unsupported-event')
  try {
    const pr = event?.pull_request
    const number = pr?.number
    if (!Number.isSafeInteger(number) || number < 1 || number > 1000000000) return full('invalid-pr-number')
    const base = validateSha(pr.base.sha)
    const head = validateSha(pr.head.sha)
    // Read-only PR ref works with forks. Check the fetched object against the
    // webhook head SHA; never trust user-controlled branch names or shell input.
    git(['fetch', '--no-tags', 'origin', `+refs/pull/${number}/head:refs/remotes/origin/ci-scope-pr-head`])
    const fetched = git(['rev-parse', 'refs/remotes/origin/ci-scope-pr-head']).trim()
    if (fetched !== head) return full('head-changed-during-fetch')
    const mergeBase = validateSha(git(['merge-base', base, head]).trim())
    return classifyDiff(git(['diff', '--name-status', '-z', '--find-renames', mergeBase, head, '--']))
  } catch {
    return full('event-git-or-history-failure')
  }
}

export function encodeOutputs(selection) {
  if (selection.version !== 'ci-scope-02-v1' || !selection.jobs || JOBS.some(id => typeof selection.jobs[id] !== 'boolean')) throw Error('invalid selector result')
  const fields = [`version=${selection.version}`, `mode=${selection.full ? 'full' : 'selective'}`, `reason=${selection.reason}`, `free_only=${selection.freeOnly === true}`]
  for (const job of JOBS) fields.push(`${OUTPUT_KEYS[job]}=${selection.jobs[job]}`)
  return `${fields.join('\n')}\n`
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  let selection
  try {
    const event = JSON.parse(readFileSync(process.env.GITHUB_EVENT_PATH, 'utf8'))
    const git = args => execFileSync('git', args, { encoding: 'utf8', timeout: 45000, maxBuffer: 4 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'] })
    selection = selectFromEvent(process.env.GITHUB_EVENT_NAME, event, git)
  } catch { selection = full('unreadable-event') }
  // The caller supplied GITHUB_OUTPUT; do not emit raw paths, tokens or logs.
  if (!process.env.GITHUB_OUTPUT) throw Error('missing GITHUB_OUTPUT')
  appendFileSync(process.env.GITHUB_OUTPUT, encodeOutputs(selection))
  console.log(JSON.stringify({ version: selection.version, full: selection.full, reason: selection.reason, freeOnly: selection.freeOnly, selected: JOBS.filter(id => selection.jobs[id]) }))
}
