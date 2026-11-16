import { fileURLToPath } from 'node:url'
import { defineConfig } from 'vitest/config'

export default defineConfig({
  test: { exclude: ['**/node_modules/**', '**/.nuxt/**', '**/.output/**', 'tests/browser/**'] },
  resolve: {
    alias: {
      '~': fileURLToPath(new URL('./', import.meta.url)),
    },
  },
})
