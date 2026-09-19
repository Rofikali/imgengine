export default defineEventHandler(async (event) => {
  const config = useRuntimeConfig(event)
  const jobId = getRouterParam(event, 'jobId')
  return await $fetch(`/api/jobs/${encodeURIComponent(jobId ?? '')}/logs`, {
    baseURL: config.apiBase,
    headers: { 'X-API-Key': config.apiKey },
  })
})
