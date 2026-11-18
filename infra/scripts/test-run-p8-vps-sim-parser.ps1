[CmdletBinding()]
param(
  [string]$ScriptPath
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($ScriptPath)) {
  $ScriptPath = Join-Path $PSScriptRoot 'run-p8-vps-sim.ps1'
}
$resolved = (Resolve-Path -LiteralPath $ScriptPath).Path
$tokens = $null
$errors = $null
[void][System.Management.Automation.Language.Parser]::ParseFile($resolved, [ref]$tokens, [ref]$errors)
if ($errors.Count -ne 0) {
  $details = $errors | ForEach-Object { "$($_.Extent.StartLineNumber):$($_.Message)" }
  throw "P8 simulation parser errors: $($details -join '; ')"
}

$source = Get-Content -LiteralPath $resolved -Raw
if (([regex]::Matches($source, '(?m)^finally\s*\{')).Count -ne 1) {
  throw 'P8 simulation must have exactly one try/finally cleanup block.'
}
if ($source -notmatch [regex]::Escape("`$oomEventPattern = '(?m)^oom(?:_kill)?\s+[1-9][0-9]*\s*`$'")) {
  throw 'P8 simulation OOM pattern is missing or changed without its parser regression test.'
}
$pattern = '(?m)^oom(?:_kill)?\s+[1-9][0-9]*\s*$'
if ('oom 1' -notmatch $pattern -or 'oom_kill 2' -notmatch $pattern -or 'oom 0' -match $pattern -or 'unavailable' -match $pattern) {
  throw 'P8 simulation OOM pattern does not reject exactly nonzero oom/oom_kill counters.'
}
Write-Output "PASS | P8 simulation parser and OOM predicate | $resolved"
