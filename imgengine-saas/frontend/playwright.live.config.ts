import { defineConfig } from '@playwright/test'
import config from './playwright.config'

// Requires an explicitly configured local Rust API; never use a public service.
export default defineConfig({ ...config, testMatch: '**/sheet-crop-live.spec.ts',
  projects: config.projects?.filter(project => project.name === 'desktop-chromium'),
})
