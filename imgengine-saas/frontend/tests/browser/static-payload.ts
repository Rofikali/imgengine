// Only Nuxt's predictable, same-origin prerender payload requests are allowed
// in the browser-local privacy assertions. This does not allow arbitrary GETs.
const freeRoutes = new Set(['/', '/resize-image', '/crop-image', '/compress-image', '/rotate-image', '/convert-image'])
export function isApprovedStaticNuxtPayload(url: URL): boolean {
  if (process.env.IMGENGINE_STATIC_E2E !== '1') return false
  const suffix = '/_payload.json'
  if (!url.pathname.endsWith(suffix)) return false
  const route = url.pathname.slice(0, -suffix.length) || '/'
  if (!freeRoutes.has(route)) return false
  const keys = [...url.searchParams.keys()]
  if (keys.length !== 1 || keys[0] !== '_b') return false
  return /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i.test(url.searchParams.get('_b') || '')
}
