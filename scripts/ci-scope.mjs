import { execFileSync } from 'node:child_process'
import { readFileSync } from 'node:fs'
import { pathToFileURL } from 'node:url'

export const jobs = ['native-build-test', 'native-sanitizers', 'native-fuzz', 'saas-integration', 'frontend-verification', 'rust-api-container', 'docker-linux-verification']
const layers = ['docs', 'frontend', 'proxy', 'rust', 'native', 'wasm', 'legacy', 'infra', 'unknown']
export function fallback(reason) {
  return { mode: 'dry-run', full: true, affected: Object.fromEntries(layers.map(k => [k, true])), reasons: [reason], proposedJobs: [...jobs], actualJobs: [...jobs] }
}

// NUL-delimited Git records preserve whitespace and cannot become shell commands.
export function classify(raw) {
  if (typeof raw !== 'string' || !raw.endsWith('\0')) return fallback('missing-or-invalid-diff')
  const fields = raw.slice(0, -1).split('\0')
  if (fields.length % 2 || fields.length > 20000) return fallback('ambiguous-or-large-diff')
  const affected = Object.fromEntries(layers.map(k => [k, false]))
  for (let i = 0; i < fields.length; i += 2) {
    const [status, path] = fields.slice(i, i + 2)
    if (!['A', 'M'].includes(status)) return fallback('rename-delete-or-unsupported-status')
    if (!path || /[\\\x00-\x1f\x7f]/.test(path) || path.startsWith('/') || path.split('/').some(p => !p || p === '.' || p === '..')) return fallback('invalid-path')
    if (/(^|\/)(AGENTS\.md|skills\.md|.*lock.*|Dockerfile.*|.*\.ya?ml|.*\.json|.*\.toml|\.env.*)$/.test(path)) return fallback('policy-dependency-or-build-config')
    if (path === 'docs/README.md' || path === 'README.md' || /^docs\/notes\/[^/]+\.md$/.test(path)) affected.docs = true
    else if (path.startsWith('docs/')) return fallback('contract-or-policy-document')
    else if (path.startsWith('imgengine/')) affected.native = true
    else if (path.startsWith('backend/imgengine/')) affected.native = true
    else if (/^backend\/(imgengine-api|imgengine-supervisor)\//.test(path)) affected.rust = true
    else if (/^imgengine-saas\/frontend\/(assets|public)\/.*\.(css|scss)$/.test(path)) affected.frontend = true
    else if (/^imgengine-saas\/frontend\/(components|pages|composables|utils|types|server|tests)\//.test(path)) { affected.frontend = true; affected.proxy = true }
    else if (path.startsWith('imgengine-saas/backend/')) affected.legacy = true
    else if (path.startsWith('infra/') || path.startsWith('.github/') || path.startsWith('scripts/')) affected.infra = true
    else return fallback('unknown-or-unapproved-isolation')
  }
  // Wasm has no approved isolated path today: never infer isolation from a name.
  if (affected.native || affected.legacy || affected.infra) return fallback('shared-native-legacy-or-infrastructure')
  const proposedJobs = []
  if (affected.frontend) proposedJobs.push('frontend-verification')
  if (affected.proxy || affected.rust) proposedJobs.push('rust-api-container')
  return { mode: 'dry-run', full: false, affected, reasons: Object.keys(affected).filter(k => affected[k]), proposedJobs, actualJobs: [...jobs] }
}

export function fromEvent(eventName, event, git) {
  try {
    const sha = value => { if (!/^[a-f0-9]{40}$/.test(value ?? '')) throw Error(); return value }
    let base, head
    if (eventName === 'pull_request') {
      head = sha(event.pull_request.head.sha)
      base = git(['merge-base', sha(event.pull_request.base.sha), head]).trim()
      sha(base)
    } else if (eventName === 'push') {
      base = sha(event.before); head = sha(event.after)
      if (/^0+$/.test(base)) return fallback('initial-push')
      // Record the push range, but initial rollout always proposes the full matrix.
      git(['diff', '--name-status', '-z', '--find-renames', base, head, '--'])
      return fallback('main-push-full-policy')
    } else return fallback('unsupported-event')
    return classify(git(['diff', '--name-status', '-z', '--find-renames', base, head, '--']))
  } catch { return fallback('event-or-git-failure') }
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  let result
  try {
    const event = JSON.parse(readFileSync(process.env.GITHUB_EVENT_PATH, 'utf8'))
    result = fromEvent(process.env.GITHUB_EVENT_NAME, event, args => execFileSync('git', args, { encoding: 'utf8', maxBuffer: 4 * 1024 * 1024, timeout: 30000, stdio: ['ignore', 'pipe', 'pipe'] }))
  } catch { result = fallback('unreadable-event') }
  // Informational only: no GITHUB_OUTPUT, job conditions or user-controlled paths.
  console.log(JSON.stringify(result))
}
