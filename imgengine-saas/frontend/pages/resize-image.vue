<script setup lang="ts">
import { ref } from 'vue'
import { useLocalResize } from '~/composables/useLocalResize'
const { source, result, width, height, locked, busy, error, message, dimensionsError, select, change, toggleLock, resize, cancel, reset } = useLocalResize()
const input = ref<HTMLInputElement | null>(null)
function choose(event: Event) { const target = event.target as HTMLInputElement; void select(target.files?.[0]); target.value = '' }
function remove() { reset(); input.value?.focus() }
useHead({ title: 'Resize an image locally — IMGENGINE' })
</script>

<template>
  <main class="page-shell">
    <section class="render-card local-resize" aria-labelledby="resize-title">
      <p class="eyebrow">IMGENGINE · LOCAL TOOL</p>
      <h1 id="resize-title">Resize an image</h1>
      <p class="intro">Your image stays on this device. Resize one JPEG or PNG and download a PNG. No upload or account.</p>
      <p class="help">Up to 4 MiB, 4 megapixels and 8192 pixels per side. Stored pixels are shown without EXIF rotation; metadata is not preserved. Colors may vary by browser.</p>
      <form class="render-form" @submit.prevent="resize">
        <label for="local-file">Choose a JPEG or PNG</label>
        <input id="local-file" ref="input" type="file" accept="image/jpeg,image/png" @change="choose">
        <fieldset v-if="source" class="layout-fields" :disabled="busy">
          <legend>Output size in pixels</legend>
          <label>Width <input :value="width" inputmode="numeric" aria-describedby="size-help" @input="change('width', ($event.target as HTMLInputElement).value)"></label>
          <label>Height <input :value="height" inputmode="numeric" aria-describedby="size-help" @input="change('height', ($event.target as HTMLInputElement).value)"></label>
          <label class="layout-note"><span><input type="checkbox" :checked="locked" @change="toggleLock(($event.target as HTMLInputElement).checked)"> Maintain aspect ratio</span></label>
          <p id="size-help" class="help layout-note">1–4096 pixels per side, up to 4 megapixels. Aspect lock rounds to the nearest pixel.</p>
        </fieldset>
        <p v-if="dimensionsError" class="error" role="alert">{{ dimensionsError }}</p>
        <div class="selection-actions">
          <button v-if="source" class="primary" type="submit" :disabled="busy || !!dimensionsError">Resize locally</button>
          <button v-if="busy" class="secondary" type="button" @click="cancel">Cancel</button>
          <button v-if="source || busy || error" class="secondary" type="button" @click="remove">Remove image</button>
        </div>
      </form>
      <p class="status" role="status">{{ message }}</p>
      <p v-if="error" class="error" role="alert">{{ error }}</p>
      <div v-if="source" class="result">
        <p class="help">Source: {{ source.width }} × {{ source.height }} pixels.</p>
        <img :src="source.url" alt="Local source image">
      </div>
      <section v-if="result" class="result" aria-label="Resized image">
        <img :src="result" alt="Resized PNG preview">
        <a class="primary" :href="result" download="imgengine-resized.png">Download PNG</a>
      </section>
      <p><NuxtLink to="/crop-image">Crop locally</NuxtLink> · <NuxtLink to="/rotate-image">Rotate locally</NuxtLink> · <NuxtLink to="/compress-image">Compress locally</NuxtLink> · <NuxtLink to="/">Server print layouts (uploads on Render)</NuxtLink></p>
    </section>
  </main>
</template>

<style scoped>
.local-resize > .help { margin-top: 1rem; }
input[type=file] { max-width: 100%; }
a:not(.primary) { color: #b7d0ff; }
input:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; }
</style>
