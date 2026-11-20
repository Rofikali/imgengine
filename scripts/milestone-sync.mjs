import { readFileSync, writeFileSync, mkdirSync, existsSync } from 'node:fs'
import { pathToFileURL } from 'node:url'

const BEGIN = '<!-- BEGIN AUTO-GENERATED PR MILESTONES -->'
const END = '<!-- END AUTO-GENERATED PR MILESTONES -->'
const ID = /^[A-Z][A-Z0-9-]{1,63}$/

export function validateRegistry(data) {
  if (!data || data.schema_version !== 1 || !Array.isArray(data.milestones) ||
      data.release_state !== 'NOT_RELEASED' || typeof data.release !== 'string' ||
      !Array.isArray(data.release_blockers)) throw Error('Invalid milestone registry or release-state override')
  const ids = new Set()
  for (const m of data.milestones) {
    if (!m || !ID.test(m.id) || ids.has(m.id) || typeof m.title !== 'string' ||
        m.title.length > 160 || !Number.isInteger(m.priority) || m.priority < 0 ||
        m.priority > 9999 || !Array.isArray(m.depends_on) ||
        (m.pr !== null && (!Number.isInteger(m.pr) || m.pr < 1)) ||
        !['READY', 'PLANNED', 'BLOCKED', 'IN_PROGRESS'].includes(m.declared_state)) {
      throw Error('Invalid milestone record')
    }
    ids.add(m.id)
  }
  for (const m of data.milestones) {
    if (m.depends_on.some(dep => !ids.has(dep) || dep === m.id)) throw Error('Invalid milestone dependency: ' + m.id)
  }
  if (data.release_blockers.some(b => typeof b !== 'string' || b.length > 180)) throw Error('Invalid blocker')
  return data
}

export function escapeCell(value) {
  return String(value).replace(/[\r\n\t]+/g, ' ').replace(/[|<>\[\]]/g, '\\$&').slice(0, 160)
}

export function determinePRStatus(pr, checks) {
  if (!pr || typeof pr.number !== 'number' || !pr.head?.sha) throw Error('Malformed GitHub PR')
  if (pr.merged_at) {
    if (!/^[a-f0-9]{40}$/.test(pr.merge_commit_sha || '')) throw Error('Merged PR missing merge SHA')
    return 'MERGED'
  }
  if (pr.state === 'closed') return 'CLOSED_UNMERGED'
  if (pr.state !== 'open') throw Error('Unknown PR state')
  if (pr.draft) return 'DRAFT'
  const gate = (checks || []).filter(c => c.name === 'CI verification gate')
    .sort((a, b) => String(b.completed_at || b.started_at || '').localeCompare(String(a.completed_at || a.started_at || '')))[0]
  if (!gate || gate.status !== 'completed') return 'CI_PENDING'
  return gate.conclusion === 'success' ? 'READY_FOR_REVIEW' : 'CI_FAILED'
}

export function resolveMilestones(data, details) {
  validateRegistry(data)
  const resolved = data.milestones.map(m => {
    const remote = m.pr === null ? null : details.get(m.pr)
    if (m.pr !== null && !remote) throw Error('Missing PR fact: ' + m.pr)
    return { ...m, status: remote ? determinePRStatus(remote.pr, remote.checks) : m.declared_state,
      merge_sha: remote?.pr.merge_commit_sha || null }
  })
  const byId = new Map(resolved.map(m => [m.id, m]))
  for (const m of resolved) {
    m.unmet = m.depends_on.filter(dep => byId.get(dep).status !== 'MERGED')
  }
  const outstanding = resolved.filter(m => m.status !== 'MERGED')
    .sort((a, b) => a.priority - b.priority || a.id.localeCompare(b.id))
  const now = outstanding.find(m => m.unmet.length === 0 &&
    !['BLOCKED', 'CLOSED_UNMERGED', 'PLANNED'].includes(m.status)) || null
  const next = outstanding.find(m => m.id !== now?.id) || null
  const completed = resolved.filter(m => m.status === 'MERGED')
  const working = resolved.filter(m => ['DRAFT', 'CI_PENDING', 'READY_FOR_REVIEW', 'CI_FAILED', 'IN_PROGRESS'].includes(m.status))
  return { now, next, completed, working, all: resolved }
}

function recordLine(m) {
  if (!m) return 'None declared; an owner-reviewed update is required.'
  const label = '**' + escapeCell(m.id) + '** — ' + escapeCell(m.title) + ' (' + m.status + ')'
  const link = m.pr === null ? '' : ' — [PR #' + m.pr + '](https://github.com/Rofikali/imgengine/pull/' + m.pr + ')'
  const dep = m.unmet.length ? ' — waiting on: ' + m.unmet.map(escapeCell).join(', ') : ''
  return label + link + dep
}

export function renderStatus(data, report, openPRs = []) {
  const tracked = new Set(data.milestones.filter(m => m.pr !== null).map(m => m.pr))
  const other = openPRs.filter(p => !tracked.has(p.number) &&
    !String(p.head?.ref || '').startsWith('docs/auto-milestone-status'))
    .sort((a, b) => b.number - a.number).slice(0, 20)
  const lines = [
    '## GitHub-verified NOW / NEXT / WORKING / COMPLETED',
    '',
    'Generated from the owner-reviewed milestone registry and GitHub PR/check facts. This is a **snapshot**, not a live GitHub view or release approval. Historical notes elsewhere in this repository may describe older PR states.',
    '',
    '**Product:** ' + escapeCell(data.release) + ' — **release state: NOT_RELEASED** (owner approval required).',
    '',
    '### NOW', '', recordLine(report.now), '',
    '### NEXT', '', recordLine(report.next), '',
    '### WORKING / REVIEW', '',
    ...(report.working.length ? report.working.map(m => '- ' + recordLine(m)) : ['No tracked PR is currently in progress.']),
    '',
    '### COMPLETED (merged PRs)', '',
    ...(report.completed.length ? report.completed.map(m => '- ' + recordLine(m) +
      ' — merge ' + m.merge_sha.slice(0, 12)) : ['No recorded milestone PRs are verified merged.']),
    '',
    '### Other open PRs (not automatically assigned a milestone)', '',
    ...(other.length ? other.map(p => '- [PR #' + p.number +
      '](https://github.com/Rofikali/imgengine/pull/' + p.number + ') — ' +
      escapeCell(p.title || '(untitled)')) : ['None found.']),
    '',
    '### Independent public-release blockers', '',
    ...data.release_blockers.map(b => '- ' + escapeCell(b)),
    '',
    '**Safety:** PR merge is not product release. Exact-SHA CI, dependency audit, static privacy/security, legal/support, hosting/rollback, cost and owner go/no-go remain independent. No automatic merge, deployment, tag, audit waiver or change of authorization is performed.',
    ''
  ]
  return lines.join('\n')
}

export function replaceGeneratedRegion(original, content) {
  const region = BEGIN + '\n' + content.trimEnd() + '\n' + END
  const a = original.indexOf(BEGIN), b = original.indexOf(END)
  if ((a === -1) !== (b === -1)) throw Error('Broken generated region')
  if (a !== -1) {
    if (original.indexOf(BEGIN, a + BEGIN.length) !== -1 || original.indexOf(END, b + END.length) !== -1 || b < a) {
      throw Error('Duplicate or reordered generated region')
    }
    return original.slice(0, a) + region + original.slice(b + END.length)
  }
  const match = original.match(/^# [^\n]+\n/)
  if (!match) throw Error('next.md has no valid heading')
  return match[0] + '\n' + region +
    '\n\n## Historical and human-maintained notes (dated; current facts above take precedence)\n' +
    original.slice(match[0].length)
}

export async function gather(client, data) {
  const details = new Map()
  for (const item of data.milestones) {
    if (item.pr === null || details.has(item.pr)) continue
    const pr = await client('/pulls/' + item.pr)
    if (pr.number !== item.pr || pr.base?.ref !== 'main') throw Error('PR number/base mismatch for ' + item.id)
    const checks = pr.state === 'open' && !pr.draft ?
      (await client('/commits/' + encodeURIComponent(pr.head.sha) +
        '/check-runs?check_name=CI%20verification%20gate&per_page=100')).check_runs : []
    if (!Array.isArray(checks)) throw Error('Invalid GitHub check-runs response')
    details.set(item.pr, { pr, checks })
  }
  const open = []
  for (let page = 1; page <= 5; page += 1) {
    const items = await client('/pulls?state=open&per_page=100&page=' + page)
    if (!Array.isArray(items)) throw Error('Invalid open PR list')
    open.push(...items)
    if (items.length < 100) break
    if (page === 5) throw Error('Open PR pagination bound exceeded')
  }
  return { details, open }
}

export function createClient(token, repository, callFetch = fetch) {
  if (!token || !/^[\w.-]+\/[\w.-]+$/.test(repository || '')) throw Error('Missing GitHub token/repository')
  return async path => {
    const url = 'https://api.github.com/repos/' + repository + path
    const response = await callFetch(url, {
      headers: { 'Accept': 'application/vnd.github+json', 'Authorization': 'Bearer ' + token,
        'User-Agent': 'imgengine-milestone-sync', 'X-GitHub-Api-Version': '2022-11-28' },
      signal: AbortSignal.timeout(15000)
    })
    if (!response.ok) throw Error('GitHub API request failed (' + response.status + '): ' + path.split('?')[0])
    return response.json()
  }
}

export async function generate({ registry, client, nextText }) {
  const data = validateRegistry(registry)
  const facts = await gather(client, data)
  const report = resolveMilestones(data, facts.details)
  const status = '# IMGENGINE milestone status\n\n' + renderStatus(data, report, facts.open)
  const next = replaceGeneratedRegion(nextText, renderStatus(data, report, facts.open))
  return { next, status }
}

async function main() {
  if (process.argv.length !== 3 || process.argv[2] !== '--write') throw Error('Usage: node scripts/milestone-sync.mjs --write')
  const registry = JSON.parse(readFileSync('docs/engineering/milestones.json', 'utf8'))
  const client = createClient(process.env.GH_TOKEN || process.env.GITHUB_TOKEN, process.env.GITHUB_REPOSITORY)
  const next = readFileSync('next.md', 'utf8')
  const generated = await generate({ registry, client, nextText: next })
  if (generated.next !== next) writeFileSync('next.md', generated.next)
  mkdirSync('docs/engineering', { recursive: true })
  if (!existsSync('docs/engineering/live-status.md') ||
      readFileSync('docs/engineering/live-status.md', 'utf8') !== generated.status) {
    writeFileSync('docs/engineering/live-status.md', generated.status)
  }
  console.log('Milestone snapshot generated; release state remains NOT_RELEASED')
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  main().catch(err => { console.error(err.message); process.exitCode = 1 })
}
