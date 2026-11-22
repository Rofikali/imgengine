import { test } from 'node:test'
import assert from 'node:assert/strict'
import { readFileSync } from 'node:fs'
import { spawnSync } from 'node:child_process'
import { fileURLToPath } from 'node:url'

test('Free public notice sources contain unmistakable pre-launch draft state', () => {
  for (const name of ['privacy', 'terms', 'support']) {
    const source = readFileSync(fileURLToPath(new URL('../free-static/' + name + '.vue', import.meta.url)), 'utf8')
    assert.match(source, /PRE-LAUNCH DRAFT/)
    assert.ok(source.includes('<FreeNoticeContact />'))
  }
})

test('Shared metadata identifies the contact and pending date', () => {
  const source = readFileSync(new URL('../free-static/FreeNoticeContact.vue', import.meta.url), 'utf8')
  assert.match(source, /Rofik Ali — Independent operator of IMGENGINE/)
  assert.ok(source.includes('href="mailto:alirofikr@gmail.com"'))
  assert.match(source, /Effective date pending publication approval/)
})

test('Free public release preflight refuses unsigned/unapproved draft publication', () => {
  const script = fileURLToPath(new URL('./check-free-publication-ready.mjs', import.meta.url))
  const check = spawnSync(process.execPath, [script], { encoding: 'utf8' })
  assert.equal(check.status, 1, 'Unapproved Free v1 must never be marked ready for publication')
  assert.match(check.stderr, /PUBLICATION BLOCKED/)
  assert.match(check.stderr, /detached owner approval/)
})
