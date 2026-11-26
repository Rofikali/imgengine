<script setup lang="ts">
import FreeNoticeContact from './FreeNoticeContact.vue'

useSeoMeta({
  title: 'Support — IMGENGINE Free',
  description: 'Contact IMGENGINE about a tool, accessibility, security or privacy.',
})
</script>

<template>
  <main class="page-shell">
    <article class="render-card free-notice" aria-labelledby="support-title">
      <p class="eyebrow">IMGENGINE · FREE</p>
      <h1 id="support-title">Support and contact</h1>
      <p>For help, a privacy request, or a private security or accessibility report, email the operator below. Include the tool name, browser version and what happened. For a privacy request, describe the correspondence you want the operator to find.</p>
      <p>Please leave out photographs, passwords, payment details and identity documents. Support uses Gmail. The privacy notice explains how email is handled. Rofik Ali keeps ordinary correspondence for 12 months after the last support interaction.</p>
      <h2>Troubleshooting</h2>
      <p>Check the file type, size and resolution limits shown in the tool. Larger images may exceed the memory available to your browser. Keep your original file while checking the result.</p>
      <FreeNoticeContact />
      <p><NuxtLink to="/privacy">Privacy notice</NuxtLink> · <NuxtLink to="/terms">Terms of use</NuxtLink></p>
    </article>
  </main>
</template>
