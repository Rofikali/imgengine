import { test } from 'node:test'
import assert from 'node:assert/strict'
import { readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'

test('Free public notice sources contain the operator contact component and final copy', () => {
  for (const name of ['privacy', 'terms', 'support']) {
    const source = readFileSync(fileURLToPath(new URL('../free-static/' + name + '.vue', import.meta.url)), 'utf8')
    assert.doesNotMatch(source, /PRE-LAUNCH DRAFT|pending actual publication approval/i)
    assert.ok(source.includes('<FreeNoticeContact />'))
  }
})

test('Shared metadata identifies the contact and effective event', () => {
  const source = readFileSync(new URL('../free-static/FreeNoticeContact.vue', import.meta.url), 'utf8')
  assert.match(source, /Rofik Ali — Independent operator of IMGENGINE/)
  assert.ok(source.includes('href="mailto:alirofikr@gmail.com"'))
  assert.match(source, /Effective from first public availability/)
})

test('Free public release preflight rejects missing build output', () => {
  const script = fileURLToPath(new URL('./check-free-publication-ready.mjs', import.meta.url))
  const check = spawnSync(process.execPath, [script, fileURLToPath(new URL("./missing-public-directory", import.meta.url))], { encoding: 'utf8' })
  assert.equal(check.status, 1, 'Unapproved Free v1 must never be marked ready for publication')
  assert.match(check.stderr, /PUBLICATION BLOCKED/)
  assert.match(check.stderr, /Missing Free static output directory/)
})
