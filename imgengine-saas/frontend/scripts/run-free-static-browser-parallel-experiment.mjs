// Opt-in measurement only. Required CI continues to use the serial launcher.
import { spawn } from 'node:child_process'
import { closeSync, mkdirSync, mkdtempSync, openSync } from 'node:fs'
import { join, resolve } from 'node:path'
import { fileURLToPath, pathToFileURL } from 'node:url'
import { performance } from 'node:perf_hooks'

const frontendRoot = fileURLToPath(new URL('../', import.meta.url))
export const LANES = Object.freeze([
  Object.freeze({ id: 'a', port: '3100', projects: Object.freeze(['desktop-chromium', 'mobile-chromium', 'desktop-webkit']) }),
  Object.freeze({ id: 'b', port: '3101', projects: Object.freeze(['desktop-firefox']) }),
])

export function laneEnvironment(lane, output, parent = process.env) {
  return { ...parent, IMGENGINE_STATIC_E2E: '1', IMGENGINE_STATIC_PARALLEL_EXPERIMENT: '1',
    IMGENGINE_STATIC_TEST_PORT: lane.port, PLAYWRIGHT_OUTPUT_DIR: output }
}

export async function runLanes(runLane) {
  // allSettled keeps the other lane observable even if a child cannot start.
  const results = await Promise.allSettled(LANES.map(lane => Promise.resolve().then(() => runLane(lane))))
  return results.every(r => r.status === 'fulfilled' && r.value === 0) ? 0 : 1
}

async function runExperiment() {
  const outputBase = join(frontendRoot, 'test-results')
  mkdirSync(outputBase, { recursive: true })
  const out = mkdtempSync(join(outputBase, 'free-parallel-'))
  const start = performance.now()
  const status = await runLanes(lane => new Promise((resolveLane, reject) => {
    const log = join(out, 'lane-' + lane.id + '.log')
    const fd = openSync(log, 'wx')
    let child
    try {
      child = spawn(process.execPath, [join(frontendRoot, 'scripts/run-free-static-browser-matrix.mjs'), '--experiment-lane=' + lane.id], {
        cwd: frontendRoot, stdio: ['ignore', fd, fd],
        env: laneEnvironment(lane, join(out, 'lane-' + lane.id)),
      })
    } finally { closeSync(fd) }
    child.once('error', reject)
    child.once('close', (code, signal) => {
      console.log('Free parallel lane ' + lane.id + ': code=' + code + ' signal=' + signal + ' log=' + log)
      resolveLane(signal ? 1 : code)
    })
  }))
  console.log('Free parallel experiment: elapsed_ms=' + Math.round(performance.now() - start) + ' status=' + status + ' evidence=' + out)
  return status
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  if (process.argv.length !== 2) { console.error('No arguments accepted'); process.exitCode = 1 }
  else {
    try { process.exitCode = await runExperiment() }
    catch (error) { console.error(error); process.exitCode = 1 }
  }
}
