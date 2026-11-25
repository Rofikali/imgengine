import { spawnSync } from 'node:child_process'
import { fileURLToPath, pathToFileURL } from 'node:url'
import { resolve } from 'node:path'

// Every verified browser project and every static-route assertion must run.
// Firefox and WebKit occasionally stall on full-load navigation even after
// the static server finishes all responses. One-test browser/server process
// lifetimes preserve every assertion and the original per-test deadlines,
// while removing cross-test accumulation. A failure remains a hard failure.
// Playwright with --fully-parallel uses TEST-level sharding, not file-level.
// The locked shard count must match the 55 discovered browser tests: empty
// shards cannot pass. Keep the count contract synchronized with the tests.
export const FREE_STATIC_BROWSER_PROJECTS = Object.freeze([
  'desktop-chromium',
  'mobile-chromium',
  'desktop-firefox',
  'desktop-webkit',
])
export const ISOLATED_BROWSER_TEST_SHARDS = 55

const frontendRoot = fileURLToPath(new URL('../', import.meta.url))

export function runFreeStaticBrowserMatrix(run = spawnSync) {
  for (const project of FREE_STATIC_BROWSER_PROJECTS) {
    const shards = ['desktop-firefox', 'desktop-webkit'].includes(project) ? ISOLATED_BROWSER_TEST_SHARDS : 1
    for (let i = 1; i <= shards; i++) {
      const label = project + (shards > 1 ? ' shard ' + i + '/' + shards : '')
      console.log('Free static browser matrix: running ' + label + ' with a new static test server')
      const args = [
        'exec', 'playwright', 'test',
        '--config', 'playwright.free-static.config.ts',
        '--project=' + project,
      ]
      if (shards > 1) args.push('--fully-parallel', '--shard=' + i + '/' + shards)
      const result = run('pnpm', args, {
        cwd: frontendRoot,
        stdio: 'inherit',
        env: {
          ...process.env,
          IMGENGINE_STATIC_E2E: '1',
          // Test-only server instrumentation: logs bounded static-route names,
          // status and completion times, never image/filename/metadata bytes.
          // Keep bounded, metadata-free request-completion diagnostics for
          // browser engines that have exhibited navigation-level failures.
          IMGENGINE_STATIC_TEST_DIAGNOSTICS: ['desktop-firefox', 'desktop-webkit'].includes(project) ? '1' : '0',
        },
      })
      if (result.error) throw result.error
      if (result.status !== 0) {
        console.error('Free static browser matrix failed at ' + label + '; remaining tests are NOT counted as passed')
        return Number.isInteger(result.status) && result.status > 0 ? result.status : 1
      }
    }
  }
  console.log('Free static browser matrix: all Chromium, Firefox and WebKit privacy/static tests passed')
  return 0
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  process.exitCode = runFreeStaticBrowserMatrix()
}
