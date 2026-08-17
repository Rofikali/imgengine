export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  const payload = await readFormData(event)
  const idempotencyKey = getHeader(event, 'idempotency-key')
  try {
    return await $fetch('/api/generate', {
      baseURL: config.apiBase,
      method: 'POST',
      body: payload,
      headers: {
        'X-API-Key': config.apiKey,
        ...(idempotencyKey ? { 'Idempotency-Key': idempotencyKey } : {}),
      },
    })
  } catch (error: any) {
    throw createError({
      statusCode: error.response?.status ?? 502,
      statusMessage: error.data?.detail ?? 'Unable to submit image job',
    })
  }
})
