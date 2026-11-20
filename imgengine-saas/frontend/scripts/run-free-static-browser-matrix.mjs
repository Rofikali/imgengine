import { spawnSync } from 'node:child_process'
import { fileURLToPath, pathToFileURL } from 'node:url'
import { resolve } from 'node:path'

// Keep the exact four-browser Free static verification matrix. Each project
// launches Playwright in its own process, giving the test-only static webServer
// a fresh lifecycle. This avoids sharing a single long-lived server across
// 200 successive checks; it does not relax assertions, deadlines, or privacy/CSP.
export const FREE_STATIC_BROWSER_PROJECTS = Object.freeze([
  'desktop-chromium',
  'mobile-chromium',
  'desktop-firefox',
  'desktop-webkit',
])

const frontendRoot = fileURLToPath(new URL('../', import.meta.url))

export function runFreeStaticBrowserMatrix(run = spawnSync) {
  for (const project of FREE_STATIC_BROWSER_PROJECTS) {
    console.log('Free static browser matrix: running ' + project + ' with an independent webServer lifecycle')
    const result = run(
      'pnpm',
      ['exec', 'playwright', 'test', '--config', 'playwright.free-static.config.ts', '--project=' + project],
      { cwd: frontendRoot, stdio: 'inherit' },
    )
    if (result.error) throw result.error
    if (result.status !== 0) {
      console.error('Free static browser matrix failed at ' + project + '; remaining projects not counted as passed')
      return Number.isInteger(result.status) && result.status > 0 ? result.status : 1
    }
  }
  console.log('Free static browser matrix: all four independent projects passed')
  return 0
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  process.exitCode = runFreeStaticBrowserMatrix()
}
