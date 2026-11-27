import { test } from 'node:test'
import assert from 'node:assert/strict'
import { LANES, laneEnvironment, runLanes } from './run-free-static-browser-parallel-experiment.mjs'
import { FREE_STATIC_BROWSER_PROJECTS, ISOLATED_BROWSER_TEST_SHARDS, runFreeStaticBrowserMatrix } from './run-free-static-browser-matrix.mjs'

test('two lanes retain every required project and all 112 invocations with separate ports', () => {
  assert.deepEqual(LANES.flatMap(l => l.projects).sort(), [...FREE_STATIC_BROWSER_PROJECTS].sort())
  assert.equal(new Set(LANES.map(l => l.port)).size, LANES.length)
  assert.equal(ISOLATED_BROWSER_TEST_SHARDS, 55)
  const calls = []
  for (const lane of LANES) {
    assert.equal(runFreeStaticBrowserMatrix((cmd, args) => { calls.push(args); return { status: 0 } }, lane.projects), 0)
  }
  assert.equal(calls.length, 112)
  for (const project of ['desktop-firefox', 'desktop-webkit']) {
    const shards = calls.filter(args => args.includes('--project=' + project))
    assert.deepEqual(shards.map(args => args.at(-1)), Array.from({length:55}, (_, i) => '--shard=' + (i+1) + '/55'))
  }
})

test('isolated lane sets strict origin and independent output path', () => {
  const env = laneEnvironment(LANES[1], '/tmp/lane-b', { CI: 'true', IMGENGINE_STATIC_E2E: '0' })
  assert.equal(env.IMGENGINE_STATIC_E2E, '1')
  assert.equal(env.IMGENGINE_STATIC_TEST_PORT, '3101')
  assert.equal(env.IMGENGINE_STATIC_PARALLEL_EXPERIMENT, '1')
  assert.equal(env.PLAYWRIGHT_OUTPUT_DIR, '/tmp/lane-b')
})

test('both lanes must succeed, including spawn errors and missing status', async () => {
  const seen = []
  assert.equal(await runLanes(lane => { seen.push(lane.id); return 0 }), 0)
  assert.deepEqual(seen.sort(), ['a','b'])
  assert.equal(await runLanes(lane => lane.id === 'a' ? 0 : 2), 1)
  assert.equal(await runLanes(lane => { if (lane.id === 'a') throw Error('spawn failed'); return 0 }), 1)
  assert.equal(await runLanes(() => null), 1)
})

test('serial runner rejects invalid project lists and fails on child error', () => {
  assert.throws(() => runFreeStaticBrowserMatrix(() => ({status:0}), []))
  assert.throws(() => runFreeStaticBrowserMatrix(() => ({status:0}), ['desktop-firefox','desktop-firefox']))
  assert.equal(runFreeStaticBrowserMatrix(() => ({ status: 9 }), ['desktop-firefox']), 9)
})
