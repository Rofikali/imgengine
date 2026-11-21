import test from 'node:test'
import assert from 'node:assert/strict'
import { parseDiff, checkDocsSync, REQUIRED_DOCS, readDiff } from './docs-sync-check.mjs'

const sample = (extra = []) => [...REQUIRED_DOCS, ...extra].map(path => ({status:'M', path}))
test('material implementation change requires all four canonical documents', () => {
  assert.equal(checkDocsSync(sample(['imgengine/src/image.c'])).mode, 'four-file')
  assert.throws(() => checkDocsSync([{status:'M',path:'imgengine/src/image.c'}]), /mandatory docs-sync missing/)
  assert.throws(() => checkDocsSync(sample().filter(x=>x.path!=='skills.md')), /skills.md/)
})
test('security, CI and contract-only docs changes are also substantive', () => {
  for (const path of ['.github/workflows/ci.yml', 'docs/ENGINE_SPEC.md', 'scripts/ci-scope.mjs', 'backend/AGENTS.md']) {
    assert.throws(() => checkDocsSync([{status:'M',path}]), /mandatory docs-sync/)
    assert.equal(checkDocsSync(sample([path])).mode, 'four-file')
  }
})
test('bot generated snapshot is the sole allowed reduced transaction', () => {
  assert.equal(checkDocsSync([{status:'M',path:'next.md'},{status:'M',path:'docs/engineering/live-status.md'}]).mode, 'generated-only')
  assert.equal(checkDocsSync([{status:'M',path:'docs/engineering/live-status.md'}]).mode, 'generated-only')
  assert.throws(()=>checkDocsSync([{status:'D',path:'next.md'}]),/cannot delete/)
  assert.throws(()=>checkDocsSync([{status:'M',path:'next.md'},{status:'M',path:'docs/engineering/live-status.md'},{status:'M',path:'README.md'}]),/mandatory docs-sync/)
})
test('canonical docs cannot be deleted even if all other paths change', () => {
  assert.throws(()=>checkDocsSync(sample().map(c=>c.path==='AGENTS.md'?{...c,status:'D'}:c)), /mandatory docs-sync|deletion/)
})
test('git NUL diff parser rejects malformed and unsafe paths and accepts normal changes', () => {
  assert.deepEqual(parseDiff('M\0README.md\0A\0skills.md\0'),[{status:'M',path:'README.md'},{status:'A',path:'skills.md'}])
  assert.throws(()=>parseDiff('M\0README.md'),/NUL/)
  assert.throws(()=>parseDiff('R100\0old\0new\0'),/malformed|unsupported/)
  assert.throws(()=>parseDiff('M\0../escape\0'),/unsafe/)
  assert.throws(()=>parseDiff('M\0evil\npath\0'),/unsafe/)
  assert.throws(()=>readDiff('--exec=bad','HEAD'),/unsafe revision/)
})
