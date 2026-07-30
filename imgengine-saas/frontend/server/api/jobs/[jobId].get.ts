export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  const jobId = getRouterParam(event, 'jobId')
  try {
    return await $fetch(`/api/status/${encodeURIComponent(jobId ?? '')}`, {
      baseURL: config.apiBase,
      headers: { 'X-API-Key': config.apiKey },
    })
  } catch (error: any) {
    throw createError({
      statusCode: error.response?.status ?? 502,
      statusMessage: error.data?.detail ?? 'Unable to retrieve job status',
    })
  }
})
