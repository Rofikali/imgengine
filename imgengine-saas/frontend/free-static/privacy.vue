<script setup lang="ts">
useSeoMeta({
  title: 'Privacy — IMGENGINE Free (draft)',
  description: 'How browser-local IMGENGINE Free handles image files, hosting requests and support information. Pre-launch draft pending review.',
})
</script>

<template>
  <main class="page-shell">
    <article class="render-card free-notice" aria-labelledby="privacy-title">
      <p class="eyebrow">IMGENGINE · FREE · PRE-LAUNCH DRAFT</p>
      <h1 id="privacy-title">Privacy notice</h1>
      <p class="intro">This draft describes the intended Free v1.0.0 experience. It has not been approved for public launch.</p>
      <h2>Images and local processing</h2>
      <p>Resize, Crop, Compress, Rotate and Convert run in your browser. IMGENGINE Free is designed not to send your selected image files, their content or their filenames to an IMGENGINE processing server. Results are created on your device for you to download. Your browser and device manage temporary memory, previews and downloaded files.</p>
      <h2>Website delivery</h2>
      <p>Loading this website sends ordinary requests for HTML, JavaScript, CSS and other static assets to the hosting provider. The provider may process technical information such as IP address, request time, user-agent, network and security metadata. This is separate from uploading image content.</p>
      <h2>Accounts, analytics and storage</h2>
      <p>Free v1.0.0 is planned without IMGENGINE user accounts, payment processing, server image storage or application-controlled advertising trackers. Hosting-provider security features, logs or analytics must be reviewed and disclosed before launch; do not assume the absence of all third-party cookies or hosting logs.</p>
      <h2>Support information and your choices</h2>
      <p>If you contact the future support channel, the information you choose to provide may need to be processed to reply. Do not send private images, identity documents, passwords or other sensitive content in a support request. A verified private contact and applicable rights-request procedure must be published before the service launches.</p>
      <h2>Owner and contact</h2>
      <p><strong>Launch pending:</strong> the service operator's approved identity, private privacy-contact channel, applicable retention details and final effective date have not yet been supplied. This draft is not a substitute for a reviewed privacy notice.</p>
      <p><NuxtLink to="/support">Support and contact readiness</NuxtLink> · <NuxtLink to="/terms">Draft terms</NuxtLink></p>
    </article>
  </main>
</template>
