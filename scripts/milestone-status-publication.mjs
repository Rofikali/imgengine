import { pathToFileURL } from 'node:url'

export const BOT_EMAIL = '41898282+github-actions[bot]@users.noreply.github.com'

// These are observations from trusted GitHub/Git commands, never PR body text.
// Keep branch publication separate from opening the review PR: a failed PR
// creation must be attempted again even if the bot branch already matches.
export function planStatusPublication({ remoteSha, remoteAuthor, sameTree, openPrNumber }) {
  if (typeof remoteSha !== 'string' || typeof remoteAuthor !== 'string' ||
      typeof openPrNumber !== 'string' || typeof sameTree !== 'boolean') {
    throw new Error('Malformed status publication observation')
  }
  if (remoteSha && !/^[a-f0-9]{40}$/.test(remoteSha)) {
    throw new Error('Invalid remote status-branch SHA')
  }
  if (remoteSha && remoteAuthor !== BOT_EMAIL) {
    throw new Error('Status branch is not owned by GitHub Actions bot')
  }
  if (!remoteSha && (remoteAuthor || sameTree || openPrNumber)) {
    throw new Error('Inconsistent missing remote branch state')
  }
  if (openPrNumber && !/^[1-9][0-9]*$/.test(openPrNumber)) {
    throw new Error('Invalid status PR number')
  }
  return (sameTree ? 'keep' : 'push') + '-' +
    (openPrNumber ? 'reuse' : 'open')
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const args = process.argv.slice(2)
    if (args.length !== 4 || !['true', 'false'].includes(args[2])) {
      throw new Error('Expected remote SHA, author, same-tree flag and open PR number')
    }
    console.log(planStatusPublication({
      remoteSha: args[0], remoteAuthor: args[1], sameTree: args[2] === 'true',
      openPrNumber: args[3]
    }))
  } catch (error) {
    console.error(error.message)
    process.exitCode = 1
  }
}
