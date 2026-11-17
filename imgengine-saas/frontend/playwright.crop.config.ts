import { defineConfig } from '@playwright/test'
import config from './playwright.config'
export default defineConfig({ ...config, testMatch: '**/local-crop.spec.ts' })
