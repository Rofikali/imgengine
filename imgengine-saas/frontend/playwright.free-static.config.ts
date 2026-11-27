import { defineConfig } from '@playwright/test'
import localTools from './playwright.local-tools.config'
import { expectedLocalToolOrigin } from './tests/browser/static-payload'

const origin = expectedLocalToolOrigin()
const port = new URL(origin).port

export default defineConfig({
  ...localTools,
  use: { ...localTools.use, baseURL: origin },
  testMatch: ['**/local-*.spec.ts', '**/tool-navigation.spec.ts', '**/free-static.spec.ts'],
  webServer: {
    command: 'node scripts/serve-free-static.mjs',
    url: origin,
    env: { PORT: port },
    reuseExistingServer: false,
    // Forward test-only request-completion timing from the isolated Firefox server.
    stderr: 'pipe',
    timeout: 30_000,
  },
})
