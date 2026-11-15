<script setup lang="ts">
import { computed, ref } from 'vue'
import type { SheetCrop } from '~/types/api'
import { clamp, cropPixels, type CropPosition, type ImageSize } from '~/utils/crop-editor'

const props = defineProps<{
  source: ImageSize & { url: string }, target: ImageSize, crop: SheetCrop,
  position: CropPosition, selected: boolean, disabled: boolean,
}>()
const emit = defineEmits<{ change: [position: CropPosition], reset: [] }>()
const frame = ref<HTMLDivElement | null>(null)
const pixels = computed(() => cropPixels(props.source, props.crop))
const imageStyle = computed(() => ({
  width: `${props.source.width / pixels.value.width * 100}%`,
  height: `${props.source.height / pixels.value.height * 100}%`,
  left: `${-pixels.value.x / pixels.value.width * 100}%`,
  top: `${-pixels.value.y / pixels.value.height * 100}%`,
}))
let drag: { id: number, x: number, y: number, position: CropPosition } | null = null
function update(key: keyof CropPosition, event: Event) {
  emit('change', { ...props.position, [key]: Number((event.target as HTMLInputElement).value) })
}
function start(event: PointerEvent) {
  if (props.disabled || drag || event.button !== 0) return
  frame.value?.focus({ preventScroll: true })
  frame.value?.setPointerCapture(event.pointerId)
  drag = { id: event.pointerId, x: event.clientX, y: event.clientY, position: { ...props.position } }
}
function move(event: PointerEvent) {
  if (props.disabled || !drag || event.pointerId !== drag.id || !frame.value) return
  const bounds = frame.value.getBoundingClientRect()
  const travelX = props.source.width - pixels.value.width
  const travelY = props.source.height - pixels.value.height
  emit('change', { ...drag.position,
    x: travelX ? clamp(drag.position.x - (event.clientX - drag.x) / bounds.width * pixels.value.width / travelX * 100, 0, 100) : 50,
    y: travelY ? clamp(drag.position.y - (event.clientY - drag.y) / bounds.height * pixels.value.height / travelY * 100, 0, 100) : 50,
  })
}
function stop(event: PointerEvent) {
  if (drag?.id !== event.pointerId) return
  drag = null
  if (frame.value?.hasPointerCapture(event.pointerId)) frame.value.releasePointerCapture(event.pointerId)
}
function keydown(event: KeyboardEvent) {
  if (props.disabled || !['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'].includes(event.key)) return
  event.preventDefault()
  const step = event.shiftKey ? 10 : 1
  const key = ['ArrowLeft', 'ArrowRight'].includes(event.key) ? 'x' : 'y'
  const direction = ['ArrowLeft', 'ArrowUp'].includes(event.key) ? -1 : 1
  emit('change', { ...props.position, [key]: clamp(props.position[key] + direction * step, 0, 100) })
}
</script>

<template>
  <fieldset class="crop-editor" :disabled="disabled">
    <legend>Adjust photo crop</legend>
    <p id="crop-help" class="help">Drag the photo, or use the position and zoom sliders. Arrow keys move the crop; Shift + arrow moves further. Reset returns to centered fill.</p>
    <div ref="frame" class="crop-frame" role="group" aria-label="Photo crop preview" aria-describedby="crop-help"
      :tabindex="disabled ? -1 : 0" :aria-disabled="disabled" :style="{ aspectRatio: `${target.width} / ${target.height}`, width: `min(100%, 22rem, ${28 * target.width / target.height}rem)` }"
      @pointerdown="start" @pointermove="move" @pointerup="stop" @pointercancel="stop" @lostpointercapture="stop" @keydown="keydown">
      <img :src="source.url" alt="Selected photo within the crop frame" draggable="false" :style="imageStyle">
    </div>
    <label for="crop-x">Horizontal position: {{ Math.round(position.x) }}%</label>
    <input id="crop-x" type="range" min="0" max="100" step="1" :value="position.x" @input="update('x', $event)">
    <label for="crop-y">Vertical position: {{ Math.round(position.y) }}%</label>
    <input id="crop-y" type="range" min="0" max="100" step="1" :value="position.y" @input="update('y', $event)">
    <label for="crop-zoom">Zoom: {{ position.zoom.toFixed(2) }}×</label>
    <input id="crop-zoom" type="range" min="1" max="4" step="0.05" :value="position.zoom" @input="update('zoom', $event)">
    <button class="secondary" type="button" @click="emit('reset')">Reset crop to centered fill</button>
    <p role="status" aria-live="polite" class="help">{{ selected ? 'Your selected crop will be used.' : 'Default centered fill will be used.' }} Preview is approximate; the completed server JPEG is authoritative.</p>
  </fieldset>
</template>

<style scoped>
.crop-editor { display: grid; gap: .75rem; min-width: 0; padding: 1rem; border: 1px solid #8897a8; border-radius: 1rem; }
.crop-frame { position: relative; overflow: hidden; background: #162334; touch-action: none; cursor: grab; }
.crop-frame:active { cursor: grabbing; }
.crop-frame:focus-visible { outline: 3px solid #147b68; outline-offset: 4px; }
.crop-frame img { position: absolute; max-width: none; pointer-events: none; user-select: none; }
input[type=range] { width: 100%; min-height: 2.75rem; padding: 0; }
button { min-height: 2.75rem; }
</style>
