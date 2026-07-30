<script setup lang="ts">
const selectedFile = ref<File | null>(null)
const job = ref<{ job_id: string; status: string; output?: string; error?: string } | null>(null)
const error = ref('')
const submitting = ref(false)

async function submit() {
  if (!selectedFile.value) return
  submitting.value = true
  error.value = ''
  const form = new FormData()
  form.append('file', selectedFile.value)
  try {
    job.value = await $fetch('/api/jobs', { method: 'POST', body: form })
    pollStatus()
  } catch (cause) {
    error.value = cause instanceof Error ? cause.message : 'Unable to submit job'
  } finally {
    submitting.value = false
  }
}

async function pollStatus() {
  if (!job.value || ['completed', 'failed'].includes(job.value.status)) return
  await new Promise(resolve => setTimeout(resolve, 1500))
  try {
    job.value = await $fetch(`/api/jobs/${job.value.job_id}`)
  } catch (cause) {
    error.value = cause instanceof Error ? cause.message : 'Unable to retrieve job status'
    return
  }
  pollStatus()
}
</script>

<template>
  <main>
    <section>
      <p class="eyebrow">IMGENGINE</p>
      <h1>Print-ready layouts, on demand.</h1>
      <p class="intro">Upload a JPEG or PNG to generate a professional photo sheet.</p>
      <form @submit.prevent="submit">
        <label for="file">Source image</label>
        <input id="file" accept="image/jpeg,image/png" type="file" required @change="selectedFile = ($event.target as HTMLInputElement).files?.[0] ?? null">
        <button :disabled="!selectedFile || submitting">{{ submitting ? 'Submitting…' : 'Generate layout' }}</button>
      </form>
      <p v-if="error" class="error">{{ error }}</p>
      <div v-if="job" class="job">
        <strong>Job {{ job.job_id }}</strong>
        <span>{{ job.status }}</span>
        <code v-if="job.output">{{ job.output }}</code>
      </div>
    </section>
  </main>
</template>

<style>
:root { color: #eaf0ff; background: #09111f; font-family: Inter, ui-sans-serif, system-ui, sans-serif; }
body { margin: 0; }
main { min-height: 100vh; display: grid; place-items: center; padding: 2rem; }
section { width: min(100%, 42rem); padding: 3rem; border: 1px solid #273752; border-radius: 1rem; background: #101c30; box-shadow: 0 2rem 6rem #0005; }
.eyebrow { color: #76a7ff; font-weight: 700; letter-spacing: .14em; font-size: .75rem; }
h1 { font-size: clamp(2rem, 5vw, 3.5rem); margin: .3rem 0; line-height: 1.05; }
.intro { color: #b5c3dc; } form { display: grid; gap: .8rem; margin-top: 2rem; }
input, button { font: inherit; padding: .8rem; border-radius: .5rem; }
input { border: 1px solid #415577; background: #0a1425; color: inherit; }
button { border: 0; background: #76a7ff; color: #07101e; cursor: pointer; font-weight: 700; }
button:disabled { opacity: .5; cursor: not-allowed; }.job { display: grid; gap: .5rem; margin-top: 1.5rem; padding: 1rem; background: #0a1425; border-radius: .5rem; }.error { color: #ff9ba4; }
</style>
