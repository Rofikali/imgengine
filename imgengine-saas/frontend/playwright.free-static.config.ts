import { defineConfig } from '@playwright/test'
import localTools from './playwright.local-tools.config'

export default defineConfig({
  ...localTools,
  testMatch: ['**/local-*.spec.ts', '**/tool-navigation.spec.ts', '**/free-static.spec.ts'],
  webServer: {
    command: 'node scripts/serve-free-static.mjs',
    url: 'http://127.0.0.1:3100',
    env: { PORT: '3100' },
    reuseExistingServer: false,
    timeout: 30_000,
  },
})
