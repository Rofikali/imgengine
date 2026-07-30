export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  const payload = await readFormData(event)
  try {
    return await $fetch('/api/generate', {
      baseURL: config.apiBase,
      method: 'POST',
      body: payload,
      headers: { 'X-API-Key': config.apiKey },
    })
  } catch (error: any) {
    throw createError({
      statusCode: error.response?.status ?? 502,
      statusMessage: error.data?.detail ?? 'Unable to submit image job',
    })
  }
})
