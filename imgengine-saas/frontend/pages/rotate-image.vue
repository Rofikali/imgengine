<script setup lang="ts">
import { computed, ref } from 'vue'
import { useLocalResize } from '~/composables/useLocalResize'
import { RIGHT_ANGLE_ROTATIONS, rotatedSize, type RightAngleRotation } from '~/utils/local-rotate'

const rotation = ref<RightAngleRotation>(90), input = ref<HTMLInputElement | null>(null)
const { source, result, busy, error, message, select, resize, cancel, reset, invalidate } = useLocalResize(undefined, () => rotation.value, 'rotate')
const output = computed(() => source.value ? rotatedSize(source.value, rotation.value) : null)
function choose(event: Event) { const target = event.target as HTMLInputElement; void select(target.files?.[0]); target.value = '' }
function chooseRotation(value: RightAngleRotation) { rotation.value = value; invalidate() }
function remove() { reset(); input.value?.focus() }
useHead({ title: 'Rotate an image locally — IMGENGINE' })
</script>

<template>
  <main class="page-shell"><section class="render-card local-rotate" aria-labelledby="rotate-title">
    <p class="eyebrow">IMGENGINE · LOCAL TOOL</p><h1 id="rotate-title">Rotate an image</h1>
    <p class="intro">Rotate one JPEG or PNG by a right angle and download a PNG. Your image stays on this device. No upload or account.</p>
    <p class="help">Up to 4 MiB, 4 megapixels and 8192 pixels per side. Stored pixels are used without EXIF auto-rotation; original metadata is not preserved. Export is re-encoded as PNG and colors may vary by browser.</p>
    <form class="render-form" @submit.prevent="resize">
      <label for="rotate-file">Choose a JPEG or PNG</label><input id="rotate-file" ref="input" type="file" accept="image/jpeg,image/png" @change="choose">
      <fieldset v-if="source" class="layout-fields" :disabled="busy"><legend>Right-angle rotation</legend>
        <div class="rotation-options" role="group" aria-label="Rotation angle">
          <button v-for="angle in RIGHT_ANGLE_ROTATIONS" :key="angle" type="button" class="secondary" :aria-pressed="rotation === angle" @click="chooseRotation(angle)">{{ angle === 0 ? 'Keep orientation' : `Rotate ${angle}° clockwise` }}</button>
        </div>
        <p class="help layout-note">Source: {{ source.width }} × {{ source.height }} pixels. Output: {{ output!.width }} × {{ output!.height }} pixels. Outputs over 4096 pixels per side or 4 megapixels are rejected.</p>
      </fieldset>
      <div class="selection-actions"><button v-if="source" type="submit" class="primary" :disabled="busy">Rotate locally</button><button v-if="busy" type="button" class="secondary" @click="cancel">Cancel</button><button v-if="source || busy || error" type="button" class="secondary" @click="remove">Remove image</button></div>
    </form>
    <p class="status" role="status">{{ message }}</p><p v-if="error" class="error" role="alert">{{ error }}</p>
    <div v-if="source" class="result"><img :src="source.url" alt="Local source image"></div>
    <section v-if="result" class="result" aria-label="Rotated image"><img :src="result" alt="Rotated PNG preview"><a class="primary" :href="result" download="imgengine-rotated.png">Download PNG</a></section>
    <p><NuxtLink to="/resize-image">Resize locally</NuxtLink> · <NuxtLink to="/crop-image">Crop locally</NuxtLink> · <NuxtLink to="/">Server print layouts (uploads on Render)</NuxtLink></p>
  </section></main>
</template>

<style scoped>
.local-rotate > .help { margin-top: 1rem; } input[type=file] { max-width: 100%; }
.rotation-options { display: flex; flex-wrap: wrap; gap: .5rem; } .rotation-options [aria-pressed=true] { outline: 3px solid #b7d0ff; outline-offset: 2px; }
button:focus-visible, input:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; } a:not(.primary) { color: #b7d0ff; }
</style>
