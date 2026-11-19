<script setup lang="ts">
import { computed, reactive, ref, watch } from 'vue'
import { useLocalResize } from '~/composables/useLocalResize'
import { initialCrop, moveCrop, parsePixelCrop, type PixelCrop } from '~/utils/local-crop'
const fields = reactive({ x: '0', y: '0', width: '1', height: '1' })
const square = ref(false), input = ref<HTMLInputElement | null>(null)
const { source, result, busy, error, message, select, resize, cancel, reset, invalidate } = useLocalResize((): PixelCrop => parsePixelCrop(fields, source.value!))
const selection = computed(() => { try { return source.value ? parsePixelCrop(fields, source.value) : null } catch { return null } })
const invalid = computed(() => {
  if (!source.value) return ''
  try { parsePixelCrop(fields, source.value); return '' } catch (e) { return (e as Error).message }
})
function setSelection(crop: PixelCrop) { invalidate(); for (const key of ['x', 'y', 'width', 'height'] as const) fields[key] = String(crop[key]) }
let drag: { id: number, x: number, y: number, scaleX: number, scaleY: number, crop: PixelCrop } | null = null
function resetSelection() { drag = null; square.value = false; if (source.value) setSelection(initialCrop(source.value)) }
watch(source, resetSelection)
function edit(key: keyof PixelCrop, value: string) {
  drag = null; invalidate(); fields[key] = value
  if (square.value && (key === 'width' || key === 'height')) fields[key === 'width' ? 'height' : 'width'] = value
}
function aspect(event: Event) { square.value = (event.target as HTMLSelectElement).value === 'square'; if (source.value) setSelection(initialCrop(source.value, square.value)) }
function down(event: PointerEvent) {
  if (busy.value || !selection.value || !source.value || event.button !== 0) return
  const target = event.currentTarget as HTMLElement, box = target.getBoundingClientRect()
  event.preventDefault()
  target.focus(); target.setPointerCapture(event.pointerId)
  drag = { id: event.pointerId, x: event.clientX, y: event.clientY, scaleX: source.value.width / box.width, scaleY: source.value.height / box.height, crop: selection.value }
}
function move(event: PointerEvent) {
  if (!drag || drag.id !== event.pointerId || busy.value || !source.value) return
  event.preventDefault()
  setSelection(moveCrop(drag.crop, source.value, (event.clientX - drag.x) * drag.scaleX, (event.clientY - drag.y) * drag.scaleY))
}
function key(event: KeyboardEvent) {
  if (!selection.value || !source.value || busy.value) return
  const steps: Record<string, [number, number]> = { ArrowLeft: [-1, 0], ArrowRight: [1, 0], ArrowUp: [0, -1], ArrowDown: [0, 1] }
  const step = steps[event.key]; if (!step) return
  event.preventDefault(); const distance = event.shiftKey ? 10 : 1
  setSelection(moveCrop(selection.value, source.value, step[0] * distance, step[1] * distance))
}
function choose(event: Event) { const target = event.target as HTMLInputElement; void select(target.files?.[0]); target.value = '' }
function remove() { drag = null; reset(); input.value?.focus() }
useHead({ title: 'Crop an image locally — IMGENGINE' })
</script>
<template>
  <main class="page-shell"><section class="render-card local-crop" aria-labelledby="crop-title">
    <p class="eyebrow">IMGENGINE · LOCAL TOOL</p><h1 id="crop-title">Crop an image</h1>
    <p class="intro">Select an exact pixel region and download a PNG. Your image stays on this device. No upload or account.</p>
    <p class="help">JPEG/PNG up to 4 MiB, 4 megapixels and 8192 pixels per side. Stored pixels are shown without EXIF rotation; original metadata is not preserved. Colors may vary by browser.</p>
    <form class="render-form" @submit.prevent="resize">
      <label for="crop-file">Choose a JPEG or PNG</label><input id="crop-file" ref="input" type="file" accept="image/jpeg,image/png" @change="choose">
      <fieldset v-if="source" class="layout-fields" :disabled="busy"><legend>Crop selection in pixels</legend>
        <label v-for="axis in (['x', 'y', 'width', 'height'] as const)" :key="axis">{{ { x: 'Left', y: 'Top', width: 'Width', height: 'Height' }[axis] }}<input :value="fields[axis]" inputmode="numeric" aria-describedby="crop-help" @input="edit(axis, ($event.target as HTMLInputElement).value)"></label>
        <label>Aspect ratio<select :value="square ? 'square' : 'free'" @change="aspect"><option value="free">Free</option><option value="square">Square</option></select></label>
        <button class="secondary" type="button" @click="resetSelection">Reset selection</button>
        <p id="crop-help" class="help layout-note">Left/top start at zero. Output up to 4096 pixels per side and 4 megapixels. Reset centers the largest allowed selection.</p>
      </fieldset>
      <p v-if="invalid" class="error" role="alert">{{ invalid }}</p>
      <div v-if="source" class="crop-stage" :style="{ width: `min(100%, ${24 * source.width / source.height}rem)`, aspectRatio: `${source.width} / ${source.height}` }" role="group" aria-label="Crop selection preview" tabindex="0" aria-describedby="crop-controls" @pointerdown="down" @pointermove="move" @pointerup="drag = null" @pointercancel="drag = null" @lostpointercapture="drag = null" @keydown="key">
        <img :src="source.url" alt="Local source image" draggable="false">
        <div v-if="selection" class="selection-box" :style="{ left: `${selection.x / source.width * 100}%`, top: `${selection.y / source.height * 100}%`, width: `${selection.width / source.width * 100}%`, height: `${selection.height / source.height * 100}%` }" />
      </div>
      <p v-if="source" id="crop-controls" class="help">Drag to move. Arrow keys move one pixel; Shift + arrow moves ten. Use the fields to resize. Selected: {{ selection ? `${selection.width} × ${selection.height} at ${selection.x}, ${selection.y}` : 'invalid' }}.</p>
      <div class="selection-actions"><button v-if="source" type="submit" class="primary" :disabled="busy || !!invalid">Crop locally</button><button v-if="busy" type="button" class="secondary" @click="cancel">Cancel</button><button v-if="source || busy || error" type="button" class="secondary" @click="remove">Remove image</button></div>
    </form>
    <p class="status" role="status">{{ message }}</p><p v-if="error" class="error" role="alert">{{ error }}</p>
    <section v-if="result" class="result" aria-label="Cropped image"><img :src="result" alt="Cropped PNG preview"><a class="primary" :href="result" download="imgengine-cropped.png">Download PNG</a></section>
    <p><NuxtLink to="/resize-image">Resize locally</NuxtLink> · <NuxtLink to="/rotate-image">Rotate locally</NuxtLink> · <NuxtLink to="/compress-image">Compress locally</NuxtLink> · <NuxtLink to="/convert-image">Convert locally</NuxtLink> · <NuxtLink to="/">Server print layouts (uploads on Render)</NuxtLink></p>
  </section></main>
</template>
<style scoped>
.local-crop > .help { margin-top: 1rem; }
.crop-stage { position: relative; overflow: hidden; touch-action: none; user-select: none; margin: auto; cursor: move; }
.crop-stage img { width: 100%; height: 100%; display: block; pointer-events: none; }
.selection-box { position: absolute; box-sizing: border-box; border: 2px solid white; outline: 1px solid black; box-shadow: 0 0 0 100vmax #0008; pointer-events: none; }
.crop-stage:focus-visible, input:focus-visible, select:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; }
input[type=file] { max-width: 100%; } a:not(.primary) { color: #b7d0ff; }
</style>
