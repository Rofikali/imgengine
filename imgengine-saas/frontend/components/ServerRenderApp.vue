<script setup lang="ts">
import { computed, nextTick, ref } from 'vue'
import { useLayoutRenderWorkflow } from '~/composables/useLayoutRenderWorkflow'

const { selectedFile, status, failure, result, previewUrl, isSubmitting, layout, select, submit, reset } = useLayoutRenderWorkflow()
const fileInput = ref<HTMLInputElement | null>(null)
const dropZone = ref<HTMLLabelElement | null>(null)
const dragActive = ref(false)
const activeMode = ref<'layout' | 'sheet'>('layout')
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
  if (fileInput.value) fileInput.value.value = ''
  await nextTick()
  dropZone.value?.focus()
}
</script>

<template>
  <main class="page-shell">
    <section class="render-card" aria-labelledby="page-title">
      <p class="eyebrow">IMGENGINE</p>
      <h1 id="page-title">Create printable photo layouts.</h1>
      <p class="intro">Choose a FIT grid or a physical print sheet. Processing is synchronous, server-validated, and ephemeral.</p>
      <p><NuxtLink to="/resize-image" style="color: #b7d0ff">Resize locally — no upload</NuxtLink> · <NuxtLink to="/compress-image" style="color: #b7d0ff">Compress locally — no upload</NuxtLink> · <NuxtLink to="/convert-image" style="color: #b7d0ff">Convert locally — no upload</NuxtLink></p>
      <nav class="mode-switch" aria-label="Choose a rendering mode">
        <button class="mode-tab" :aria-pressed="activeMode === 'layout'" type="button" @click="activeMode = 'layout'">FIT layout</button>
        <button class="mode-tab" :aria-pressed="activeMode === 'sheet'" type="button" @click="activeMode = 'sheet'">Physical sheet</button>
      </nav>
      <div v-if="activeMode === 'layout'">

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

        <fieldset class="layout-fields" :disabled="isSubmitting">
          <legend>Layout</legend>
          <label>Width <input v-model="layout.width" inputmode="decimal" required aria-label="Photo width"></label>
          <label>Height <input v-model="layout.height" inputmode="decimal" required aria-label="Photo height"></label>
          <label>Unit <select v-model="layout.unit" aria-label="Layout unit"><option value="px">Pixels</option><option value="mm">Millimetres</option></select></label>
          <label v-if="layout.unit === 'mm'">DPI <input v-model="layout.dpi" inputmode="numeric" required aria-label="DPI"></label>
          <label>Copies <input v-model="layout.copies" inputmode="numeric" required aria-label="Copies"></label>
          <label>Columns <input v-model="layout.columns" inputmode="numeric" aria-label="Columns"></label>
          <label>Gap <input v-model="layout.gap" inputmode="decimal" required aria-label="Gap"></label>
          <label>Margin <input v-model="layout.margin" inputmode="decimal" required aria-label="Margin"></label>
          <p class="help layout-note">The server performs checked FIT geometry. JPEG is the only currently supported output.</p>
        </fieldset>

        <div v-if="selectedFile" class="selection-actions">
          <button class="secondary" type="button" :disabled="isSubmitting" @click="renderAnother">Remove file</button>
          <button class="primary" type="submit" :disabled="isSubmitting">
            {{ isSubmitting ? 'Working…' : 'Create layout JPEG' }}
          </button>
        </div>
        <button v-else class="primary" type="button" @click="chooseFile">Choose JPEG or PNG</button>
      </form>

      <p v-if="status === 'uploading'" class="status" role="status" aria-live="polite">Uploading image…</p>
      <p v-else-if="status === 'processing'" class="status" role="status" aria-live="polite">Processing image…</p>
      <p v-if="failure" class="error" role="alert">{{ failure.message }}</p>

      <section v-if="status === 'success' && result && previewUrl" class="result" aria-labelledby="result-title" role="status" aria-live="polite">
        <p class="eyebrow">COMPLETE</p>
        <h2 id="result-title">Layout complete</h2>
        <p class="help">Server-confirmed canvas: {{ result.canvasWidth }} × {{ result.canvasHeight }} px; placements: {{ result.executedCount }}.</p>
        <img :src="previewUrl" alt="Rendered JPEG preview">
        <div class="selection-actions">
          <a class="primary" :href="previewUrl" download="imgengine-layout.jpg">Download JPEG</a>
          <button class="secondary" type="button" @click="renderAnother">Render another image</button>
        </div>
      </section>
      </div>
      <SheetRenderPanel v-else />
    </section>
  </main>
</template>
