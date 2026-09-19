export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  return await $fetch('/api/presets', { baseURL: config.apiBase })
})
