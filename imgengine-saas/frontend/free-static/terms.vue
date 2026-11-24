<script setup lang="ts">
import FreeNoticeContact from './FreeNoticeContact.vue'

useSeoMeta({
  title: 'Terms — IMGENGINE Free (draft)',
  description: 'Terms for using the five browser-local IMGENGINE Free tools.',
})
</script>

<template>
  <main class="page-shell">
    <article class="render-card free-notice" aria-labelledby="terms-title">
      <p class="eyebrow">IMGENGINE · FREE · PRE-LAUNCH DRAFT</p>
      <h1 id="terms-title">Terms of use</h1>
      <p><strong>Proposed text only.</strong> Effective date: pending actual publication approval. Independent legal approval has not been obtained.</p>
      <p>IMGENGINE Free provides Resize, Crop, Compress, Rotate and Convert in your browser. Use images you have permission to edit. You retain your rights in your images and choose where to save and share the results.</p>
      <h2>Responsible use</h2>
      <p>Respect applicable law, confidentiality and the rights of others. Do not use the tools to create fraudulent identity documents.</p>
      <h2>Checking your results</h2>
      <p>Processing depends on the supported format, image size, browser and available memory. Review the output before using it for printing or an official application. The receiving authority decides whether an image meets its requirements.</p>
      <h2>Availability and changes</h2>
      <p>The proposed Free service is browser-local and may be changed, suspended or withdrawn to address security, legal or operational concerns. Nothing in these proposed terms promises uninterrupted availability, a particular output quality or acceptance by any authority. Mandatory rights that apply to you are not excluded by this notice.</p>
      <p><a href="/_nuxt/third-party-licenses.txt">Third-party software licenses</a></p>
      <FreeNoticeContact />
      <p><NuxtLink to="/privacy">Privacy notice</NuxtLink> · <NuxtLink to="/support">Support and contact</NuxtLink></p>
    </article>
  </main>
</template>
