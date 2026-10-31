<script setup lang="ts">
import { nextTick, ref } from 'vue'
import { useSheetRenderWorkflow } from '~/composables/useSheetRenderWorkflow'

const { selectedFile, status, failure, result, previewUrl, isSubmitting, sheet, select, submit, reset } = useSheetRenderWorkflow()
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
  <section class="sheet-panel" aria-labelledby="sheet-title">
    <p class="eyebrow">P12 · PHYSICAL SHEET</p>
    <h2 id="sheet-title">Print exact-size photos on a sheet.</h2>
    <p class="intro">Enter paper and photo sizes in millimetres. The server converts dimensions using DPI, plans the exact copy count, and crops each photo to fill its cell.</p>

    <form v-if="status !== 'success'" class="render-form" @submit.prevent="submit">
      <label
        ref="dropZone"
        class="drop-zone"
        :class="{ 'drop-zone--active': dragActive, 'drop-zone--selected': selectedFile }"
        tabindex="0"
        aria-describedby="sheet-file-help"
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
          aria-label="Choose a JPEG or PNG photo"
          @change="selectFromInput"
        >
        <strong>{{ selectedFile ? selectedFile.name : 'Drop a photo here or choose a file' }}</strong>
        <span v-if="selectedFile">{{ selectedFileSize }} selected</span>
        <span v-else>JPEG or PNG, up to about 1 MB for the current service.</span>
      </label>
      <p id="sheet-file-help" class="help">The server validates the image and owns all memory/output limits.</p>

      <fieldset class="layout-fields" :disabled="isSubmitting">
        <legend>Paper size</legend>
        <label>Sheet width (mm) <input v-model="sheet.sheetWidthMm" inputmode="decimal" required aria-label="Sheet width in millimetres"></label>
        <label>Sheet height (mm) <input v-model="sheet.sheetHeightMm" inputmode="decimal" required aria-label="Sheet height in millimetres"></label>
      </fieldset>

      <fieldset class="layout-fields" :disabled="isSubmitting">
        <legend>Photo size</legend>
        <label>Photo width (mm) <input v-model="sheet.photoWidthMm" inputmode="decimal" required aria-label="Photo width in millimetres"></label>
        <label>Photo height (mm) <input v-model="sheet.photoHeightMm" inputmode="decimal" required aria-label="Photo height in millimetres"></label>
      </fieldset>

      <fieldset class="layout-fields" :disabled="isSubmitting">
        <legend>Print settings</legend>
        <label>DPI <input v-model="sheet.dpi" inputmode="numeric" required aria-label="Print DPI"></label>
        <label>Exact copies <input v-model="sheet.copies" inputmode="numeric" required aria-label="Exact number of copies"></label>
        <label>Margin (mm) <input v-model="sheet.marginMm" inputmode="decimal" required aria-label="Sheet margin in millimetres"></label>
        <label>Horizontal gap (mm) <input v-model="sheet.gapXmm" inputmode="decimal" required aria-label="Horizontal gap in millimetres"></label>
        <label>Vertical gap (mm) <input v-model="sheet.gapYmm" inputmode="decimal" required aria-label="Vertical gap in millimetres"></label>
        <p class="help layout-note">Default is A4 at 300 DPI. JPEG output only. If every requested copy cannot fit, the server rejects the request instead of silently reducing the count.</p>
      </fieldset>

      <div v-if="selectedFile" class="selection-actions">
        <button class="secondary" type="button" :disabled="isSubmitting" @click="renderAnother">Remove photo</button>
        <button class="primary" type="submit" :disabled="isSubmitting">
          {{ isSubmitting ? 'Working…' : 'Create physical sheet' }}
        </button>
      </div>
      <button v-else class="primary" type="button" @click="chooseFile">Choose JPEG or PNG</button>
    </form>

    <p v-if="status === 'uploading'" class="status" role="status" aria-live="polite">Uploading photo…</p>
    <p v-else-if="status === 'processing'" class="status" role="status" aria-live="polite">Planning and rendering the sheet…</p>
    <p v-if="failure" class="error" role="alert">{{ failure.message }}</p>

    <section v-if="status === 'success' && result && previewUrl" class="result" aria-labelledby="sheet-result-title" role="status" aria-live="polite">
      <p class="eyebrow">COMPLETE</p>
      <h2 id="sheet-result-title">Physical sheet complete</h2>
      <p class="help">Server-confirmed canvas: {{ result.canvasWidth }} × {{ result.canvasHeight }} px. Requested {{ result.expectedCount }} copies; rendered {{ result.executedCount }}.</p>
      <img :src="previewUrl" alt="Rendered physical photo sheet preview">
      <div class="selection-actions">
        <a class="primary" :href="previewUrl" download="imgengine-sheet.jpg">Download JPEG</a>
        <button class="secondary" type="button" @click="renderAnother">Create another sheet</button>
      </div>
    </section>
  </section>
</template>
