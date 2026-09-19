import { describe, expect, it } from 'vitest'
import { classifyRenderFailure, classifyRenderFailureData, parseRenderProblem } from './api-errors'

const requestId = 'request-test-123'

describe('render API error mapping', () => {
  it.each([
    [400, 'invalid_request', 'invalid_request'],
    [401, 'authentication_failed', 'authentication_failed'],
    [413, 'resource_limit', 'request_too_large'],
    [415, 'unsupported_media_type', 'unsupported_media_type'],
    [422, 'invalid_image', 'invalid_image'],
    [429, 'overloaded', 'overloaded'],
    [500, 'internal', 'internal'],
    [503, 'unavailable', 'unavailable'],
    [504, 'deadline_exceeded', 'deadline_exceeded'],
  ] as const)('maps %s/%s safely', (status, code, kind) => {
    expect(classifyRenderFailure(status, {
      type: `https://imgengine.example/problems/${code}`,
      title: 'Safe title',
      status,
      code,
      request_id: requestId,
    })).toMatchObject({ kind, status, requestId })
  })

  it('classifies a transport failure without accepting arbitrary details', () => {
    expect(classifyRenderFailure(null, { diagnostic: '/private/path' }))
      .toMatchObject({ kind: 'network', status: null, requestId: null })
  })

  it('rejects malformed problem bodies', () => {
    expect(parseRenderProblem({ status: 500, code: 'native_failure' })).toBeNull()
  })

  it('decodes a blob problem response before classifying it', async () => {
    const error = await classifyRenderFailureData(429, new Blob([JSON.stringify({
      type: 'https://imgengine.example/problems/overloaded',
      title: 'Service overloaded',
      status: 429,
      code: 'overloaded',
      request_id: requestId,
    })], { type: 'application/problem+json' }))

    expect(error).toMatchObject({ kind: 'overloaded', status: 429, requestId })
  })
})
