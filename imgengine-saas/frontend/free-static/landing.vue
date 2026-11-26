<script setup lang="ts">
import { tools } from './tools'

useSeoMeta({
  title: 'Private image tools — IMGENGINE Free',
  description: 'Resize, crop, compress, rotate and convert JPEG or PNG images directly on your device. No image upload or account.',
})
</script>

<template>
  <main class="home">
    <h1>Free image tools that run in your browser.</h1>
    <p class="summary">Choose a tool to edit a JPEG or PNG on your device. Your images stay in your browser until you download the result.</p>
    <nav aria-label="Free image tools" class="tool-grid">
      <NuxtLink v-for="tool in tools" :key="tool.path" :to="tool.path" class="tool-card">
        <strong>{{ tool.name }}</strong><span>{{ tool.description }}</span><span class="arrow" aria-hidden="true">↗</span>
      </NuxtLink>
    </nav>
    <p class="help">Each tool lists its supported formats and size limits. Exported images omit the original metadata.</p>
  </main>
</template>

<style scoped>
.home { max-width: 66rem; margin: 0 auto; padding: clamp(2rem, 6vw, 5rem) 1.5rem 2rem; }
.home h1 { max-width: 16ch; font-size: clamp(2.5rem, 6vw, 4.75rem); line-height: 1.02; letter-spacing: -.045em; }
.summary { color: #bdcae0; max-width: 54ch; line-height: 1.6; font-size: 1.1rem; }
.tool-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(min(100%, 17rem), 1fr)); gap: .85rem; margin: 2rem 0 1rem; }
.tool-card { position: relative; padding: 1.4rem 2.5rem 1.4rem 1.4rem; background: #122139; border: 1px solid #354d6e; text-decoration: none; color: #eef4ff; }
.tool-card:hover, .tool-card:focus-visible { border-color: #89b6ff; background: #182c48; }
.tool-card strong { display: block; font-size: 1.45rem; }
.tool-card span { display: block; margin-top: .5rem; color: #c0cfe2; font-weight: 400; line-height: 1.5; }
.tool-card .arrow { position: absolute; top: .9rem; right: 1rem; color: #89b6ff; }
</style>