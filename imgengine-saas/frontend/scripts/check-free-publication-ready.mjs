import { execFileSync } from 'node:child_process'
import { createHash } from 'node:crypto'
import { readFileSync } from 'node:fs'
import { join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { auditFreeStatic } from './verify-free-static.mjs'

const frontend = fileURLToPath(new URL('../', import.meta.url))
const output = resolve(process.argv[2] || join(frontend, '.output/public'))
const placeholders = /pre-launch draft|launch (pending|blocker):|not yet approved:|pending actual publication|proposed text only|\bTODO\b/i

try {
  const plan = JSON.parse(readFileSync(new URL('../../../docs/releases/free/v1.0.0/release-manifest.json', import.meta.url)))
  if (plan.product !== 'imgengine-free' || plan.version !== '1.0.0' || plan.publicly_released !== false ||
      plan.state !== 'READY_FOR_RELEASE_REVIEW' || plan.distribution.hosting_target !== 'cloudflare-pages' ||
      plan.distribution.type !== 'verified-static-assets-only') throw Error('Unexpected Free release plan')
  const audit = auditFreeStatic(output)
  for (const name of ['privacy', 'terms', 'support', 'FreeNoticeContact']) {
    if (placeholders.test(readFileSync(join(frontend, 'free-static', name + '.vue'), 'utf8'))) throw Error('Placeholder in ' + name)
  }
  const report = readFileSync(join(output, '_nuxt/third-party-licenses.txt'))
  // Exact report reviewed against the production inventory at 214fb6a (16 packages).
  if (createHash('sha256').update(report).digest('hex') !== 'e3c00e1a449a0af05aa57b90189b39857e8f0b1e0d54357262ee17c07dc0c25e') throw Error('Reconcile bundled licenses with the production inventory')
  const inventory = new Set()
  function collect(dependencies = {}) {
    for (const [name, dependency] of Object.entries(dependencies)) {
      inventory.add(name + '@' + dependency.version)
      collect(dependency.dependencies)
    }
  }
  const projects = JSON.parse(execFileSync('pnpm', ['list', '--prod', '--depth', 'Infinity', '--json'],
    { cwd: frontend, encoding: 'utf8', maxBuffer: 64 * 1024 * 1024 }))
  for (const project of projects) collect(project.dependencies)
  for (const [, name, version] of report.toString().matchAll(/^## (.+) - ([^ ]+) \(MIT\)$/gm)) {
    if (!inventory.has(name + '@' + version)) throw Error('Bundled license absent from production inventory: ' + name)
  }
  for (const route of ['privacy', 'terms', 'support']) {
    const page = readFileSync(join(output, route, 'index.html'), 'utf8')
    if (placeholders.test(page) || !page.includes('mailto:alirofikr@gmail.com')) throw Error('Invalid compiled notice: ' + route)
    if (route === 'terms' && !page.includes('href="/_nuxt/third-party-licenses.txt"')) throw Error('Missing Terms license link')
  }
  console.log('Verified Free static candidate', audit.staticTreeSha256)
} catch (error) {
  console.error('PUBLICATION BLOCKED:', error.message)
  process.exitCode = 1
}