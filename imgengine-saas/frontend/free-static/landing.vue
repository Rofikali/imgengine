<script setup lang="ts">
useSeoMeta({
  title: 'Private image tools — IMGENGINE Free',
  description: 'Resize, crop, compress, rotate and convert JPEG or PNG images directly on your device. No image upload or account.',
})
</script>

<template>
  <main class="page-shell">
    <section class="render-card" aria-labelledby="free-title">
      <p class="eyebrow">IMGENGINE · FREE</p>
      <h1 id="free-title">Edit images privately.</h1>
      <p class="intro">Five free image tools, processed in your browser. Choose one image, make your change, and download it without uploading your image to IMGENGINE.</p>
      <p class="help">Supports JPEG and PNG input, subject to each tool's documented size and browser limits. Image metadata is not retained in outputs.</p>
      <nav class="local-tools" aria-label="Free image tools">
        <NuxtLink to="/resize-image">Resize locally — no upload</NuxtLink>
        <NuxtLink to="/crop-image">Crop locally — no upload</NuxtLink>
        <NuxtLink to="/compress-image">Compress locally — no upload</NuxtLink>
        <NuxtLink to="/rotate-image">Rotate locally — no upload</NuxtLink>
        <NuxtLink to="/convert-image">Convert locally — no upload</NuxtLink>
      </nav>
      <p class="help">The website host can receive normal asset-request information, but your images are handled on your device. Do not use the tools with files that exceed their stated limits.</p>
    </section>
  </main>
</template>
