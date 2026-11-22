<script setup lang="ts">
import FreeNoticeContact from './FreeNoticeContact.vue'

useSeoMeta({
  title: 'Privacy — IMGENGINE Free (draft)',
  description: 'How IMGENGINE Free processes images, delivers the website and handles email.',
})
</script>

<template>
  <main class="page-shell">
    <article class="render-card free-notice" aria-labelledby="privacy-title">
      <p class="eyebrow">IMGENGINE · FREE · PRE-LAUNCH DRAFT</p>
      <h1 id="privacy-title">Privacy notice</h1>
      <h2>Images on your device</h2>
      <p>Resize, Crop, Compress, Rotate and Convert process selected images in your browser. The tools do not upload the images or filenames. Closing the tab ends the editing session. Originals and downloaded results stay on your device until you remove them.</p>
      <h2>Website delivery</h2>
      <p>Your browser requests site files from the host. Cloudflare Pages is the intended host. Serving and protecting those requests can involve your IP address, requested URL, request time and browser information. The Free app has no accounts, payment processing or application analytics. Your browser may cache site files.</p>
      <h2>Email and retention</h2>
      <p>If you email the operator, Google’s Gmail service handles your address, message and any attachments. The operator uses that information to respond to support and privacy requests. You can ask about or request deletion of your correspondence using the address below. The proposed policy keeps ordinary correspondence for 12 months after the last support interaction. Attachments share that maximum and unnecessary sensitive attachments should be removed sooner. You may request earlier deletion. A documented legal or security requirement may justify keeping specific records longer.</p>
      <p>A monthly manual review of inbox, sent and archived messages is proposed. Automatic cleanup is not active. Gmail Trash normally retains deleted messages for up to 30 additional days. Google controls the deletion schedule for its backups. Cloudflare logging, retention and analytics settings still need verification before launch.</p>
      <FreeNoticeContact />
      <p><NuxtLink to="/support">Support and contact</NuxtLink> · <NuxtLink to="/terms">Terms of use</NuxtLink></p>
    </article>
  </main>
</template>
