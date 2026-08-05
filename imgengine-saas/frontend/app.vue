<script setup lang="ts">
type Job = { job_id: string; status: string; output_url?: string | null; error?: string | null }
type JobEvent = { timestamp: string; level: string; component: string; message: string }
type JobLogResponse = { logs: string; events: JobEvent[] }

const selectedFile = ref<File | null>(null)
const job = ref<Job | null>(null)
const error = ref('')
const logs = ref('')
const submitting = ref(false)
const layout = reactive({
  width: 4.5, height: 3.5, dpi: 300, cols: 6, rows: 6,
  gap: 15, padding: 20, border: 2, bleed: 0,
  crop_mark: 15, crop_thickness: 2, crop_offset: 8,
})

const fields = [
  ['width', 'Width (cm)', 0.1, 50, 0.1], ['height', 'Height (cm)', 0.1, 50, 0.1],
  ['dpi', 'DPI', 72, 1200, 1], ['cols', 'Columns', 1, 20, 1], ['rows', 'Rows', 1, 20, 1],
  ['gap', 'Gap (px)', 0, 500, 1], ['padding', 'Padding (px)', 0, 1000, 1], ['border', 'Border (px)', 0, 100, 1],
  ['bleed', 'Bleed (px)', 0, 500, 1], ['crop_mark', 'Crop mark (px)', 0, 500, 1],
  ['crop_thickness', 'Crop thickness (px)', 1, 100, 1], ['crop_offset', 'Crop offset (px)', 0, 500, 1],
] as const

async function submit() {
  if (!selectedFile.value) return
  submitting.value = true
  error.value = ''
  logs.value = ''
  const form = new FormData()
  form.append('file', selectedFile.value)
  for (const [key] of fields) form.append(key, String(layout[key]))
  try {
    job.value = await $fetch<Job>('/api/jobs', { method: 'POST', body: form })
    void pollStatus()
  } catch (cause) {
    error.value = cause instanceof Error ? cause.message : 'Unable to submit job'
  } finally {
    submitting.value = false
  }
}

async function pollStatus() {
  while (job.value && !['completed', 'failed'].includes(job.value.status)) {
    await new Promise(resolve => setTimeout(resolve, 1500))
    try {
      job.value = await $fetch<Job>(`/api/jobs/${job.value.job_id}`)
    } catch (cause) {
      error.value = cause instanceof Error ? cause.message : 'Unable to retrieve job status'
      return
    }
  }
}

async function loadLogs() {
  if (!job.value) return
  const response = await $fetch<JobLogResponse>(`/api/jobs/${job.value.job_id}/logs`)
  const timeline = response.events.map(event =>
    `[${event.timestamp}] ${event.level.toUpperCase()} ${event.component}: ${event.message}`,
  )
  logs.value = [...timeline, response.logs].filter(Boolean).join('\n')
}
</script>

<template>
  <main>
    <section>
      <p class="eyebrow">IMGENGINE</p>
      <h1>Print-ready layouts, on demand.</h1>
      <p class="intro">Configure the physical photo size and print-safe layout, then submit it as an asynchronous job.</p>
      <form @submit.prevent="submit">
        <label for="file">Source image</label>
        <input id="file" accept="image/jpeg,image/png" type="file" required @change="selectedFile = ($event.target as HTMLInputElement).files?.[0] ?? null">
        <div class="fields">
          <label v-for="[key, label, min, max, step] in fields" :key="key">
            {{ label }}
            <input v-model.number="layout[key]" type="number" :min="min" :max="max" :step="step">
          </label>
        </div>
        <button :disabled="!selectedFile || submitting">{{ submitting ? 'Submitting…' : 'Generate layout' }}</button>
      </form>
      <p v-if="error" class="error">{{ error }}</p>
      <div v-if="job" class="job">
        <strong>Job {{ job.job_id }}</strong>
        <span>{{ job.status }}</span>
        <p v-if="job.error" class="error">{{ job.error }}</p>
        <a v-if="job.output_url" :href="`/api/jobs/${job.job_id}/output`">Download output</a>
        <button v-if="['completed', 'failed'].includes(job.status)" type="button" @click="loadLogs">Show job logs</button>
        <pre v-if="logs">{{ logs }}</pre>
      </div>
    </section>
  </main>
</template>

<style>
:root { color: #eaf0ff; background: #09111f; font-family: Inter, ui-sans-serif, system-ui, sans-serif; } body { margin: 0; }
main { min-height: 100vh; display: grid; place-items: center; padding: 2rem; } section { width: min(100%, 56rem); padding: 3rem; border: 1px solid #273752; border-radius: 1rem; background: #101c30; box-shadow: 0 2rem 6rem #0005; }
.eyebrow { color: #76a7ff; font-weight: 700; letter-spacing: .14em; font-size: .75rem; } h1 { font-size: clamp(2rem, 5vw, 3.5rem); margin: .3rem 0; line-height: 1.05; }.intro { color: #b5c3dc; }
form { display: grid; gap: .8rem; margin-top: 2rem; }.fields { display: grid; grid-template-columns: repeat(auto-fit, minmax(11rem, 1fr)); gap: .8rem; } label { display: grid; gap: .35rem; color: #c8d4ea; }
input, button { box-sizing: border-box; font: inherit; padding: .75rem; border-radius: .5rem; } input { width: 100%; border: 1px solid #415577; background: #0a1425; color: inherit; }button { border: 0; background: #76a7ff; color: #07101e; cursor: pointer; font-weight: 700; }button:disabled { opacity: .5; cursor: not-allowed; }.job { display: grid; gap: .5rem; margin-top: 1.5rem; padding: 1rem; background: #0a1425; border-radius: .5rem; }.error { color: #ff9ba4; }.job a { color: #76a7ff; }
</style>
