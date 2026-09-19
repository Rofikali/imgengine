export default defineEventHandler(async (event): Promise<unknown> => {
  const config = useRuntimeConfig(event)
  return await $fetch('/api/presets', { baseURL: config.apiBase })
})
