<template><NuxtPage /></template>

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
.mode-switch { display: flex; flex-wrap: wrap; gap: .5rem; margin-top: 1.5rem; }
.local-tools { display: flex; flex-wrap: wrap; gap: .5rem 1rem; margin: 1.5rem 0 0; }.local-tools a { color: #b7d0ff; }
.mode-tab { min-height: 2.5rem; padding: .5rem .9rem; border: 1px solid #617da7; background: transparent; color: #edf4ff; }
.mode-tab[aria-pressed="true"] { border-color: #8bb8ff; background: #18345a; }
.render-form { display: grid; gap: 1rem; margin-top: 2rem; }
.layout-fields { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: .75rem; margin: 0; padding: 1rem; border: 1px solid #2f4568; border-radius: .85rem; }
.layout-fields legend { padding: 0 .35rem; font-weight: 700; }.layout-fields label { display: grid; gap: .35rem; color: #bdcae0; font-size: .9rem; }.layout-fields input, .layout-fields select { min-width: 0; padding: .55rem; border: 1px solid #617da7; border-radius: .45rem; background: #07101d; color: #eef4ff; }.layout-note { grid-column: 1 / -1; margin: 0; }
.drop-zone { display: grid; gap: .5rem; min-height: 10rem; place-content: center; box-sizing: border-box; padding: 1.5rem; border: 1px dashed #6987b8; border-radius: .85rem; text-align: center; cursor: pointer; background: #0a1526; }
.drop-zone--active, .drop-zone--selected { border-color: #99c0ff; background: #102442; }.drop-zone strong, .drop-zone span { overflow-wrap: anywhere; }
.visually-hidden { position: absolute; width: 1px; height: 1px; margin: -1px; overflow: hidden; clip: rect(0, 0, 0, 0); white-space: nowrap; }
.help { margin: 0; font-size: .9rem; }.selection-actions { display: flex; flex-wrap: wrap; gap: .75rem; margin-top: .5rem; }
.primary, .secondary { display: inline-flex; align-items: center; justify-content: center; min-height: 2.75rem; padding: .65rem 1rem; border: 1px solid transparent; text-decoration: none; }
.primary { background: #8bb8ff; color: #071323; }.secondary { border-color: #617da7; background: transparent; color: #edf4ff; }.primary:disabled, .secondary:disabled { opacity: .55; cursor: not-allowed; }
.status, .error { margin: 1.25rem 0 0; padding: .9rem 1rem; border-radius: .65rem; }.status { background: #142944; color: #c7dcff; }.error { background: #3b1720; color: #ffd1d7; }
.result { display: grid; gap: 1rem; margin-top: 2rem; }.result img { width: 100%; max-height: 30rem; object-fit: contain; border-radius: .85rem; background: #07101d; }
@media (max-width: 32rem) { .selection-actions, .layout-fields { display: grid; grid-template-columns: 1fr; }.primary, .secondary { width: 100%; } }
</style>
