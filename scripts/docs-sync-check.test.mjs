import test from 'node:test'
import assert from 'node:assert/strict'
import { parseDiff, checkDocsSync, REQUIRED_DOCS, REQUIRED_WORK_LOG, readDiff } from './docs-sync-check.mjs'

const logged = (extra = []) => [...REQUIRED_DOCS, REQUIRED_WORK_LOG, ...extra].map(path => ({status:'M', path}))
test('engineering activity updates docs directly, regardless of PR status', () => {
  assert.equal(checkDocsSync(logged(['backend/imgengine-api/src/lib.rs'])).mode,'engineer-updated')
  assert.throws(()=>checkDocsSync([{status:'M',path:'backend/imgengine-api/src/lib.rs'}]),/agent-owned documentation missing/)
  assert.throws(()=>checkDocsSync(logged().filter(c=>c.path!=='skills.md')),/skills.md/)
  assert.throws(()=>checkDocsSync(logged().filter(c=>c.path!==REQUIRED_WORK_LOG)),/live-status.md/)
})
test('no bot generated-only exception and no deletion of required docs', () => {
  assert.throws(()=>checkDocsSync([{status:'M',path:'next.md'},{status:'M',path:'docs/engineering/live-status.md'}]),/agent-owned documentation missing/)
  assert.throws(()=>checkDocsSync(logged().map(c=>c.path==='AGENTS.md'?{...c,status:'D'}:c)),/agent-owned documentation missing|deleted/)
})
test('docs and CI changes also trigger same-task documentation', () => {
  for(const path of ['docs/ENGINE_SPEC.md','.github/workflows/ci.yml','scripts/check.js','backend/AGENTS.md']){
    assert.throws(()=>checkDocsSync([{status:'M',path}]),/agent-owned documentation missing/)
    assert.equal(checkDocsSync(logged([path])).mode,'engineer-updated')
  }
})
test('NUL-delimited git diff validation fails closed for unsafe or malformed input',()=>{
  assert.deepEqual(parseDiff('M\0README.md\0A\0skills.md\0'),[{status:'M',path:'README.md'},{status:'A',path:'skills.md'}])
  assert.throws(()=>parseDiff('M\0README.md'),/NUL/)
  assert.throws(()=>parseDiff('R100\0a\0b\0'),/malformed|unsupported/)
  assert.throws(()=>parseDiff('M\0../escape\0'),/unsafe/)
  assert.throws(()=>parseDiff('M\0bad\npath\0'),/unsafe/)
  assert.throws(()=>readDiff('--bad','HEAD'),/unsafe revision/)
})
