import { defineConfig, devices } from '@playwright/test'
import config from './playwright.config'

export default defineConfig({
  ...config,
  testMatch: '**/local-compress.spec.ts',
  projects: [
    { name: 'desktop-chromium', use: { ...devices['Desktop Chrome'] } },
    { name: 'mobile-chromium', use: { ...devices['Pixel 7'] } },
    { name: 'desktop-firefox', use: { ...devices['Desktop Firefox'] } },
    { name: 'desktop-webkit', use: { ...devices['Desktop Safari'] } },
  ],
})
