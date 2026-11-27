<script setup lang="ts">
import { tools } from './tools'

const menu = ref<HTMLDetailsElement>()
const toggle = ref<HTMLElement>()
function closeMenu() {
  if (!menu.value?.open) return
  menu.value.open = false
  toggle.value?.focus()
}
</script>

<template>
  <header class="free-header">
    <a class="skip-content" href="#free-content">Skip to content</a>
    <nav aria-label="Site navigation" class="free-bar">
      <NuxtLink class="wordmark" to="/" aria-label="IMGENGINE home"><span aria-hidden="true" class="mark"></span>IMGENGINE</NuxtLink>
      <NuxtLink to="/">Home</NuxtLink>
      <NuxtLink v-for="tool in tools" :key="tool.path" class="desktop-tool" :to="tool.path">{{ tool.name }}</NuxtLink>
      <details ref="menu" class="mobile-tools" @keydown.esc="closeMenu">
        <summary ref="toggle">Tools</summary>
        <div class="tool-menu">
          <a v-for="tool in tools" :key="tool.path" :href="tool.path" :aria-current="$route.path === tool.path ? 'page' : undefined">{{ tool.name }}</a>
        </div>
      </details>
    </nav>
  </header>
</template>

<style scoped>
.free-header { background: var(--img-color-surface); border-bottom: 1px solid #334764; }
.free-bar { max-width: 68rem; margin: auto; padding: .55rem 1rem; display: flex; align-items: center; gap: .25rem; }
.free-bar a, summary { color: #d8e4f4; padding: .75rem; text-decoration: none; }
.free-bar a:hover, summary:hover, a[aria-current="page"]:not(.wordmark) { color: white; background: #233955; }
.wordmark { margin-right: auto; display: inline-flex; align-items: center; gap: .6rem; letter-spacing: .06em; }
.mark { width: 1rem; height: 1rem; border: 3px solid #88b9ff; border-radius: .28rem; transform: rotate(-10deg); }
.mobile-tools { display: none; position: relative; }
summary { cursor: pointer; font-weight: 700; }
summary:focus-visible { outline: 3px solid var(--img-color-link-focus); }
.tool-menu { position: absolute; z-index: 5; right: 0; min-width: 10rem; padding: .35rem; border: 1px solid #456180; border-radius: .75rem; background: var(--img-color-surface); }
.tool-menu a { display: block; }
.skip-content { position: absolute; top: .5rem; left: .5rem; padding: .75rem; z-index: 10; background: #fff; color: #152941; transform: translateY(-200%); }
.skip-content:focus { transform: none; }
@media (max-width: 44rem) { .desktop-tool { display: none; } .mobile-tools { display: block; } }
</style>