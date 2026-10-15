import { readFileSync } from 'node:fs'
import { describe, expect, it } from 'vitest'

describe('P7.1 API boundary security', () => {
  it('keeps the Rust credential out of public Nuxt runtime configuration', () => {
    const config = readFileSync(new URL('../nuxt.config.ts', import.meta.url), 'utf8')
    expect(config).not.toMatch(/public\s*:\s*{[^}]*imgengineRenderApiKey/s)
    expect(config).not.toMatch(/public\s*:\s*{[^}]*IMGENGINE_API_KEYS/s)
  })

  it('keeps authentication server-side and does not expose it to the composable', () => {
    const client = readFileSync(new URL('../composables/useImgengineApi.ts', import.meta.url), 'utf8')
    const proxy = readFileSync(new URL('../server/api/render.post.ts', import.meta.url), 'utf8')
    expect(client).not.toContain('X-API-Key')
    expect(client).not.toContain('imgengineRenderApiKey')
    expect(proxy).toContain("'X-API-Key': config.imgengineRenderApiKey")
  })
})
