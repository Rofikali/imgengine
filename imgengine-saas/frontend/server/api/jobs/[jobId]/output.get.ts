export default defineEventHandler((event) => {
  const config = useRuntimeConfig(event)
  const jobId = getRouterParam(event, 'jobId')
  return proxyRequest(event, `${config.apiBase}/api/output/${encodeURIComponent(jobId ?? '')}`, {
    headers: { 'X-API-Key': config.apiKey },
  })
})
