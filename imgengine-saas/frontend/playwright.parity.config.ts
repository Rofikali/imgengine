import { defineConfig } from '@playwright/test'
import config from './playwright.live.config'

export default defineConfig({ ...config, testMatch: '**/crop-parity.spec.ts',
  reporter: [['list'], ['json', { outputFile: process.env.PARITY_REPORT || 'test-results/parity.json' }]],
})
