import { pathToFileURL } from 'node:url'
import { JOBS, OUTPUT_KEYS } from './ci-scope-select.mjs'

export function verifyNeeds(needs) {
  if (!needs || typeof needs !== 'object' || Array.isArray(needs)) throw Error('invalid needs payload')
  const scope = needs['ci-scope']
  const policy = needs['ci-policy']
  if (scope?.result !== 'success' || policy?.result !== 'success') throw Error('scope or policy checker did not pass')
  const outputs = scope.outputs
  if (!outputs || !['ci-scope-02-v1', 'bootstrap-full-v1'].includes(outputs.version)) throw Error('unknown or missing classifier version')
  if (outputs.mode !== 'full' && outputs.mode !== 'selective') throw Error('unknown classification mode')
  if (outputs.version === 'bootstrap-full-v1' && outputs.mode !== 'full') throw Error('bootstrap must run full CI')
  let selected = 0
  for (const job of JOBS) {
    const selection = outputs[OUTPUT_KEYS[job]]
    const actual = needs[job]?.result
    if (selection !== 'true' && selection !== 'false') throw Error(`invalid classifier flag for ${job}`)
    if (outputs.version === 'bootstrap-full-v1' && selection !== 'true') throw Error('bootstrap omitted required test')
    if (!['success', 'failure', 'cancelled', 'skipped'].includes(actual)) throw Error(`missing or unknown job status: ${job}`)
    if (selection === 'true') {
      selected++
      if (actual !== 'success') throw Error(`required ${job} was ${actual}, not success`)
    } else if (actual !== 'skipped') {
      throw Error(`unselected ${job} ran unexpectedly (${actual})`)
    }
  }
  if (outputs.mode === 'full' && selected !== JOBS.length) throw Error('full mode omitted tests')
  if (outputs.mode === 'selective' && selected === JOBS.length) throw Error('selective mode selected all jobs')
  return { mode: outputs.mode, selected, skipped: JOBS.length - selected }
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const result = verifyNeeds(JSON.parse(process.env.CI_NEEDS_JSON ?? ''))
    console.log(`CI-SCOPE-02 PASS: ${result.selected} selected jobs succeeded; ${result.skipped} correctly skipped; mode=${result.mode}`)
  } catch (error) {
    console.error(`::error::CI-SCOPE-02 FAIL: ${error.message}`)
    process.exitCode = 1
  }
}
