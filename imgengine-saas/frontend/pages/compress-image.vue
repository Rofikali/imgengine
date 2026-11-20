<script setup lang="ts">
import { ref } from 'vue'
import { useLocalCompress } from '~/composables/useLocalCompress'

const { source, result, quality, busy, error, message, select, changeQuality, compress, cancel, reset } = useLocalCompress()
const input = ref<HTMLInputElement | null>(null)
function choose(event: Event) { const target = event.target as HTMLInputElement; void select(target.files?.[0]); target.value = '' }
function remove() { reset(); input.value?.focus() }
useHead({ title: 'Compress an image locally — IMGENGINE' })
</script>

<template>
  <main class="page-shell"><section class="render-card local-compress" aria-labelledby="compress-title">
    <p class="eyebrow">IMGENGINE · LOCAL TOOL</p><h1 id="compress-title">Compress an image</h1>
    <p class="intro">Compress one JPEG or PNG to a JPEG on this device. No upload or account.</p>
    <p class="help">Up to 4 MiB, 4 megapixels and 8192 pixels per side. Output is JPEG, so PNG transparency is composited onto white; EXIF and other metadata are removed. JPEG quality controls file size and visible detail.</p>
    <form class="render-form" @submit.prevent="compress">
      <label for="compress-file">Choose a JPEG or PNG</label><input id="compress-file" ref="input" type="file" accept="image/jpeg,image/png" @change="choose">
      <fieldset v-if="source" class="layout-fields" :disabled="busy"><legend>JPEG quality</legend>
        <label>Quality <input :value="quality" inputmode="numeric" aria-describedby="quality-help" @input="changeQuality(($event.target as HTMLInputElement).value)"></label>
        <p id="quality-help" class="help layout-note">40–95. Lower quality usually makes a smaller file; 80 is the default.</p>
      </fieldset>
      <div class="selection-actions"><button v-if="source" type="submit" class="primary" :disabled="busy">Compress locally</button><button v-if="busy" type="button" class="secondary" @click="cancel">Cancel</button><button v-if="source || busy || error" type="button" class="secondary" @click="remove">Remove image</button></div>
    </form>
    <p class="status" role="status">{{ message }}</p><p v-if="error" class="error" role="alert">{{ error }}</p>
    <div v-if="source" class="result"><p class="help">Source: {{ source.width }} × {{ source.height }} pixels.</p><img :src="source.url" alt="Local source image"></div>
    <section v-if="result" class="result" aria-label="Compressed image"><img :src="result" alt="Compressed JPEG preview"><a class="primary" :href="result" download="imgengine-compressed.jpg">Download JPEG</a></section>
    <p><NuxtLink to="/resize-image">Resize locally</NuxtLink> · <NuxtLink to="/crop-image">Crop locally</NuxtLink> · <NuxtLink to="/rotate-image">Rotate locally</NuxtLink> · <NuxtLink to="/convert-image">Convert locally</NuxtLink> · <NuxtLink to="/">All Free tools</NuxtLink></p>
  </section></main>
</template>

<style scoped>
.local-compress > .help { margin-top: 1rem; } input[type=file] { max-width: 100%; }
input:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; } a:not(.primary) { color: #b7d0ff; }
</style>
