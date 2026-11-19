<script setup lang="ts">
import { ref } from 'vue'
import { useLocalConvert } from '~/composables/useLocalConvert'
const { source, result, format, busy, error, message, select, changeFormat, convert, cancel, reset, extension } = useLocalConvert()
const input = ref<HTMLInputElement | null>(null)
function choose(event: Event) { const target = event.target as HTMLInputElement; void select(target.files?.[0]); target.value = '' }
function remove() { reset(); input.value?.focus() }
useHead({ title: 'Convert an image locally — IMGENGINE' })
</script>
<template>
  <main class="page-shell"><section class="render-card local-convert" aria-labelledby="convert-title">
    <p class="eyebrow">IMGENGINE · LOCAL TOOL</p><h1 id="convert-title">Convert an image</h1>
    <p class="intro">Convert one JPEG or PNG on this device. No upload or account.</p>
    <p class="help">Up to 4 MiB, 4 megapixels and 8192 pixels per side. Output keeps stored pixel dimensions, removes metadata, and can vary slightly by browser. JPEG composites transparency onto white; PNG preserves transparency.</p>
    <form class="render-form" @submit.prevent="convert"><label for="convert-file">Choose a JPEG or PNG</label><input id="convert-file" ref="input" type="file" accept="image/jpeg,image/png" @change="choose">
      <fieldset v-if="source" class="layout-fields" :disabled="busy"><legend>Output format</legend><label>Format <select :value="format" aria-describedby="convert-help" @change="changeFormat(($event.target as HTMLSelectElement).value)"><option value="image/png">PNG</option><option value="image/jpeg">JPEG</option></select></label><p id="convert-help" class="help layout-note">Conversion re-encodes the image locally; EXIF and other source metadata are not preserved.</p></fieldset>
      <div class="selection-actions"><button v-if="source" type="submit" class="primary" :disabled="busy">Convert locally</button><button v-if="busy" type="button" class="secondary" @click="cancel">Cancel</button><button v-if="source || busy || error" type="button" class="secondary" @click="remove">Remove image</button></div>
    </form>
    <p class="status" role="status">{{ message }}</p><p v-if="error" class="error" role="alert">{{ error }}</p>
    <div v-if="source" class="result"><p class="help">Source: {{ source.width }} × {{ source.height }} pixels.</p><img :src="source.url" alt="Local source image"></div>
    <section v-if="result" class="result" aria-label="Converted image"><img :src="result" :alt="`Converted ${format === 'image/jpeg' ? 'JPEG' : 'PNG'} preview`"><a class="primary" :href="result" :download="`imgengine-converted.${extension()}`">Download {{ format === 'image/jpeg' ? 'JPEG' : 'PNG' }}</a></section>
    <p><NuxtLink to="/resize-image">Resize locally</NuxtLink> · <NuxtLink to="/crop-image">Crop locally</NuxtLink> · <NuxtLink to="/rotate-image">Rotate locally</NuxtLink> · <NuxtLink to="/compress-image">Compress locally</NuxtLink> · <NuxtLink to="/">Server print layouts (uploads on Render)</NuxtLink></p>
  </section></main>
</template>
<style scoped>.local-convert > .help { margin-top: 1rem; } input[type=file] { max-width: 100%; } input:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; } a:not(.primary) { color: #b7d0ff; }</style>
