<script setup lang="ts">
import { nextTick, ref } from 'vue'
import { useRenderWorkflow } from '~/composables/useRenderWorkflow'

const { selectedFile, status, failure, result, previewUrl, isSubmitting, select, submit, reset } = useRenderWorkflow()
const fileInput = ref<HTMLInputElement | null>(null)
const dropZone = ref<HTMLLabelElement | null>(null)
const dragActive = ref(false)
const formatter = new Intl.NumberFormat(undefined, { maximumFractionDigits: 1 })
const selectedFileSize = computed(() => selectedFile.value
  ? `${formatter.format(selectedFile.value.size / 1024)} KiB`
  : '')

function chooseFile() {
  fileInput.value?.click()
}

function selectFromInput(event: Event) {
  select((event.target as HTMLInputElement).files?.[0] ?? null)
}

function selectDroppedFile(event: DragEvent) {
  dragActive.value = false
  select(event.dataTransfer?.files?.[0] ?? null)
}

async function renderAnother() {
  reset()
  await nextTick()
  dropZone.value?.focus()
}
</script>

<template>
  <main class="page-shell">
    <section class="render-card" aria-labelledby="page-title">
      <p class="eyebrow">IMGENGINE</p>
      <h1 id="page-title">Render a JPEG or PNG image.</h1>
      <p class="intro">Choose one image. IMGENGINE returns a completed JPEG without creating a stored job.</p>

      <form v-if="status !== 'success'" class="render-form" @submit.prevent="submit">
        <label
          ref="dropZone"
          class="drop-zone"
          :class="{ 'drop-zone--active': dragActive, 'drop-zone--selected': selectedFile }"
          tabindex="0"
          aria-describedby="file-help"
          @keydown.enter.prevent="chooseFile"
          @keydown.space.prevent="chooseFile"
          @dragenter.prevent="dragActive = true"
          @dragover.prevent="dragActive = true"
          @dragleave.prevent="dragActive = false"
          @drop.prevent="selectDroppedFile"
        >
          <input
            ref="fileInput"
            class="visually-hidden"
            type="file"
            accept="image/jpeg,image/png"
            aria-label="Choose a JPEG or PNG image"
            @change="selectFromInput"
          >
          <strong>{{ selectedFile ? selectedFile.name : 'Drop an image here or choose a file' }}</strong>
          <span v-if="selectedFile">{{ selectedFileSize }} selected</span>
          <span v-else>JPEG or PNG, up to about 1 MB for the current service.</span>
        </label>
        <p id="file-help" class="help">File type and size checks help before upload; the server validates every image.</p>

        <div v-if="selectedFile" class="selection-actions">
          <button class="secondary" type="button" :disabled="isSubmitting" @click="renderAnother">Remove file</button>
          <button class="primary" type="submit" :disabled="isSubmitting">
            {{ isSubmitting ? 'Working…' : 'Render JPEG' }}
          </button>
        </div>
        <button v-else class="primary" type="button" @click="chooseFile">Choose JPEG or PNG</button>
      </form>

      <p v-if="status === 'uploading'" class="status" role="status" aria-live="polite">Uploading image…</p>
      <p v-else-if="status === 'processing'" class="status" role="status" aria-live="polite">Processing image…</p>
      <p v-if="failure" class="error" role="alert">{{ failure.message }}</p>

      <section v-if="status === 'success' && result && previewUrl" class="result" aria-labelledby="result-title" role="status" aria-live="polite">
        <p class="eyebrow">COMPLETE</p>
        <h2 id="result-title">Rendering complete</h2>
        <img :src="previewUrl" alt="Rendered JPEG preview">
        <div class="selection-actions">
          <a class="primary" :href="previewUrl" download="imgengine-render.jpg">Download JPEG</a>
          <button class="secondary" type="button" @click="renderAnother">Render another image</button>
        </div>
      </section>
    </section>
  </main>
</template>

<style>
:root { color: #eef4ff; background: #08111f; font-family: Inter, ui-sans-serif, system-ui, sans-serif; }
body { margin: 0; }
button, input, a { font: inherit; }
button, a { border-radius: .65rem; font-weight: 700; }
button { cursor: pointer; }
button:focus-visible, a:focus-visible, .drop-zone:focus-visible { outline: 3px solid #b7d0ff; outline-offset: 3px; }
.page-shell { box-sizing: border-box; width: 100%; min-width: 0; min-height: 100vh; display: grid; place-items: center; padding: clamp(1rem, 4vw, 3rem); }
.render-card { box-sizing: border-box; width: min(calc(100vw - 2rem), 46rem); min-width: 0; padding: clamp(1.5rem, 5vw, 3.5rem); border: 1px solid #2f4568; border-radius: 1.25rem; background: #101d31; box-shadow: 0 2rem 6rem #0008; }
.eyebrow { margin: 0 0 .6rem; color: #89b6ff; font-size: .75rem; font-weight: 800; letter-spacing: .14em; }
h1, h2 { margin: 0; line-height: 1.1; } h1 { font-size: clamp(2rem, 6vw, 3.8rem); } h2 { font-size: clamp(1.6rem, 4vw, 2.4rem); }
.intro, .help, .drop-zone span { color: #bdcae0; line-height: 1.55; }.intro { max-width: 40rem; margin: 1rem 0 0; }
.render-form { display: grid; gap: 1rem; margin-top: 2rem; }
.drop-zone { display: grid; gap: .5rem; min-height: 10rem; place-content: center; box-sizing: border-box; padding: 1.5rem; border: 1px dashed #6987b8; border-radius: .85rem; text-align: center; cursor: pointer; background: #0a1526; }
.drop-zone--active, .drop-zone--selected { border-color: #99c0ff; background: #102442; }.drop-zone strong, .drop-zone span { overflow-wrap: anywhere; }
.visually-hidden { position: absolute; width: 1px; height: 1px; margin: -1px; overflow: hidden; clip: rect(0, 0, 0, 0); white-space: nowrap; }
.help { margin: 0; font-size: .9rem; }.selection-actions { display: flex; flex-wrap: wrap; gap: .75rem; margin-top: .5rem; }
.primary, .secondary { display: inline-flex; align-items: center; justify-content: center; min-height: 2.75rem; padding: .65rem 1rem; border: 1px solid transparent; text-decoration: none; }
.primary { background: #8bb8ff; color: #071323; }.secondary { border-color: #617da7; background: transparent; color: #edf4ff; }.primary:disabled, .secondary:disabled { opacity: .55; cursor: not-allowed; }
.status, .error { margin: 1.25rem 0 0; padding: .9rem 1rem; border-radius: .65rem; }.status { background: #142944; color: #c7dcff; }.error { background: #3b1720; color: #ffd1d7; }
.result { display: grid; gap: 1rem; margin-top: 2rem; }.result img { width: 100%; max-height: 30rem; object-fit: contain; border-radius: .85rem; background: #07101d; }
@media (max-width: 32rem) { .selection-actions { display: grid; }.primary, .secondary { width: 100%; } }
</style>
