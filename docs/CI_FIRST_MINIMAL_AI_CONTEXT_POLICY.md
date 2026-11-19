# IMGENGINE — CI-first verification and minimal-AI-context log policy

**CI-FUZZ-APT-01 diagnosis and remediating policy (2026-11-19):** On F2c [PR #72](https://github.com/Rofikali/imgengine/pull/72), [native-fuzz run #44456975520](https://github.com/Rofikali/imgengine/actions/runs/44456975520) **FAILED BEFORE FUZZ EXECUTION** because Ubuntu 24.04 hosted runner APT downloads of dependencies (including ImageMagick's optional Ghostscript/font chain) exceeded three hard 180-second installation cutoffs. The workflow proposal switches to minimal necessary Clang/libFuzzer/libturbojpeg/etc packages, APT transport retries and a bounded step timeout, and a tiny synthetic valid PNG seed via base64 (no ImageMagick). A pinned Node 24 checkout v6 fixes the deprecated Node 20 action runtime warning for this job. Neither original native fuzz target, sanitizer flag, bounded 30s/60s fuzz duration, existing seven required jobs, nor the additive browser/P8 jobs is weakened. **Installation failure ≠ native bug, but it also ≠ fuzz PASS.** Verify exact-SHA installed dependencies and actual fuzzer execution before merging. On “go ahead” inspect [next.md](../next.md) and repair the first unfinished/failed job; preserve owner-only merge/deploy.\n\n**Standing instruction for Astra, Codex and all human/AI engineering contributors.** Proposed 2026-11-15; docs-only. This policy **does not change** GitHub Actions, executable tests, release requirements, permissions or branch rules. The source-backed prior guidance is [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md) and [three test lanes](TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md). Follow root and scoped AGENTS, the milestone template, and latest source/contract evidence.

## 1. Non-negotiable principle

**Run all tests required for the change, but do not load full successful test output into an AI conversation.** Compute outside the model is not free, but GitHub Actions runner minutes are separate from AI inference/context consumption. Pushing or letting GitHub Actions execute does **not** by itself make the full logs enter Astra/Codex context. Tokens are used when the agent asks for results, reads output, reasons about failures or implements fixes. **Never promise absolute zero tokens.**

- **Full logs:** CI logs/artifacts or local files under an ignored temporary directory, never huge pasted messages or tracked source.
- **Ordinary agent view:** test name, exact tested revision, outcome, relevant duration only if actually known, URL/evidence location and next action. No 5,000-line success transcript, progress-bar spam or entire Docker build output.
- **Failure agent view:** retrieve just failed job and step; start with its diagnostic summary and **at most 40–60 relevant lines**. Expand **only that failure's** log if needed to diagnose. Record commands and actual exit codes.
- **Security:** redact secrets, API keys, private customer filenames, image bytes and sensitive paths before quoting or storing logs in public CI artifacts. Do not use command echoing of credentials to gain insight.
- **Preserve evidence:** do not delete failed logs or suppress assertions to save tokens. Attach/retain logs only under existing repository/CI artifact policy; do not create unmanaged permanent storage.

## 2. Development → PR CI → review → release

| Moment | Who should run what | What the AI agent should read |
| --- | --- | --- |
| **Local development, unpushed changes** | Focused tests and relevant cross-boundary regression checks as defined by changed paths. GitHub **cannot test unpushed code**. | One compact command/exit-status summary; only failing excerpt |
| **Feature branch pushed / PR opened** | The existing GitHub Actions `.github/workflows/ci.yml` currently runs all seven checks on PRs automatically. Do **not manually duplicate the seven jobs** just to give an agent more output. | At most seven compact job status records, run URL and tested SHA; no success logs |
| **PR updated after a fix** | Re-run required CI for the **new** head/merge revision; keep prior evidence labeled historical. | Only new check status and changed failed step |
| **Merge decision** | Human reviewer verifies all *required* checks for the intended revision and reviews actual code/security acceptance; no AI-driven automatic merge. | Short go/no-go summary: green, red, pending, cancelled, skipped/unknown, reviewed limitations |
| **Native C/ABI/unsafe FFI changes** | Full relevant native builds, C11/C17, ABI/consumer, sanitizers, fuzz, Rust integration and Ubuntu verification when required by the current gates. These may run in CI; local focused native tests first. | Concise results; failed native step only when red |
| **Release candidate / real VPS** | **Exact-release-candidate** required verification + P14 live-host, capacity, TLS, secrets, alerts, recovery and explicit human approval. | Gate matrix, explicit evidence revision/host; no blanket success based on an earlier green PR |

**No weakening:** the minimal-context rule is about **where test logs go and how results are read**, not reducing test count or coverage. Never bypass CI, sanitizers, fuzz, security gates, reviews or production release criteria. A CI run on a GitHub-generated merge commit may differ from the PR head: report **actual tested SHA/ref** and explain any difference.

## 3. Current seven required-workflow jobs (verify present state before relying on this snapshot)

Source inspected: `.github/workflows/ci.yml` at main `d73e321566c1cd58854a03b4ea051a6a9f3cb322`. This is a *dated snapshot*, not a promise the CI graph never changes.

| GitHub job ID | Short status label |
| --- | --- |
| `native-build-test` | Native |
| `native-sanitizers` | Sanitizers |
| `native-fuzz` | Fuzz |
| `saas-integration` | Legacy SaaS integration |
| `frontend-verification` | Nuxt |
| `rust-api-container` | Rust API container |
| `docker-linux-verification` | Ubuntu 24.04 verifier |

**A green single job is not the whole green PR.** Treat `failure`, `cancelled`, `skipped`, `timed_out`, `neutral`, `pending`, `in_progress`, `action_required` and absent/missing expected checks as distinct states; do not render any as `PASS`. In particular, conditional skips can appear green in GitHub's UI, so the future selective-job classifier must have its own fail-closed mandatory aggregate gate.

Existing CI runs even for docs-only PRs; reducing **GitHub runner minutes** is separately proposed under [CI-SCOPE-01](ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md). **Do not implement path-filtering, disable jobs, edit branch protection or add a new CI framework in a normal feature/docs milestone.**

## 4. Local commands without transcript flooding

Capture all output **outside chat**, keep exit codes, and emit a single success line. Bash example (execute from the correct package directory on an installed, supported toolchain):

~~~bash
mkdir -p /tmp/imgengine-test-logs
log=/tmp/imgengine-test-logs/frontend-unit.log
if pnpm test >"$log" 2>&1; then
  printf 'PASS | pnpm test | full log: %s\n' "$log"
else
  status=$?
  printf 'FAIL (exit=%s) | pnpm test | relevant tail:\n' "$status"
  tail -n 50 "$log"
  exit "$status"
fi
~~~

**Important:** Do not claim that command output suppression equals skipped test execution; capture the process exit status, including `pipefail` when piping through `tee`. If a command fails, do not continue to publication/merge. On Windows/PowerShell, apply the same rule using `$LASTEXITCODE`, a log file and bounded failure tail; do not paste the complete console transcript into the agent. A failing test that lacks diagnostics warrants targeted log inspection, not guessed success.

**Use existing CI run/check APIs or GitHub UI:** fetch the workflow run and the job/check list once after completion (or report `PENDING` with URL if not finished). Query only `name`, `status`, `conclusion`, `head_sha`/actual merge revision and `html_url`. If failed, retrieve only the failed step or relevant small excerpt. **Do not poll in a tight loop, download all successful logs, or rerun expensive tests merely to fill the conversation.**

Avoid reading the same unchanged source file, entire test folder, or all repository docs in every model turn. Read only scope-specific sources and trust exact linked records rather than reprinting earlier evidence.

## 5. How change scope affects local work

Apply [existing risk-based test matrix](CHANGE_SCOPED_VERIFICATION.md): UI-only -> frontend/interaction/a11y tests; UI form/proxy or Rust HTTP contract -> frontend **and** targeted Rust/proxy integration; future isolated Worker/Wasm -> crate/browser/no-upload/memory/codec tests; C17, public ABI and unsafe FFI -> full relevant native/sanitizer/fuzz/compatibility gates; infra or unknown/mixed change -> conservative broader verification. Current GitHub CI remains fully triggered independent of these local choices.

For the user's previously reported M1.2 local SHA `f2d42ebd7f5816a8773331d6edf90060f98e2d36` in a separate Windows worktree: locally reported PASS is **not automatically a GitHub CI PASS**. Do not access or alter that local worktree, invent remote availability, or mark it published without a source-backed remote SHA/PR.

## 6. Standard engineering acceptance report (max 12 lines when green)

~~~text
Task / branch / source: <ID, branch, local or GitHub>
Base / tested SHA: <exact SHA; merge SHA if distinct>
Changed scope: <Nuxt UI + HTTP form | Wasm | C ABI | ...>
Focused local: <tests + PASS/FAIL/NOT RUN; log path>
CI URL / tested SHA: <link, SHA or NOT RUN (unpublished)>
Native: <status> | Sanitizers: <status> | Fuzz: <status>
Legacy SaaS: <status> | Nuxt: <status> | Rust API: <status> | Ubuntu: <status>
Security/privacy/a11y: <scoped result or NOT RUN>
Failed steps: <none | 1 line each + small excerpt reference>
Unresolved acceptance/release risks: <truthful list>
Approval needed: <review / merge / deploy / other>
Next action: <one bounded task>
~~~

Use explicit `PASS`, `FAIL`, `NOT RUN`, `PENDING`, `SKIPPED` and `CANCELLED`. Do not infer test results from a commit existing, a clean git worktree, a docs PR, or a successful build alone. When all tests pass, **do not write a separate engineer-by-engineer narrative**; apply seven-level/security/SRE/CA-MBA review proportionate to risk and send the consolidated result.

## 7. Enforcement and guardrails

- **Policy discovery:** root `AGENTS.md` is the repository-wide rule; nested AGENTS refine scope; `skills.md` contains the repeatable recipe; `next.md` points to policy and milestone status. This is **not guaranteed to auto-load in every AI tool**: instruct new agents to read root AGENTS once. Do not duplicate the full document into each nested AGENTS.
- **No invented infrastructure:** CI-SCOPE-01 is a *separate* code proposal, not this documentation milestone. No new Python, paid observability service, database, queue, CI action with broad write permissions or experimental dependency just to hide logs.
- **Review human decisions:** pushing/publishing, merging, branch settings, deployment, secrets, purchasing services and legacy retirement follow existing permission gates. This policy never grants permission to merge/deploy.
- **When full logs really are needed:** failing tests, nondeterminism, memory corruption, security incidents, performance regression or release investigation justify escalating log examination. Protect private information and communicate why escalation is needed.
- **Performance and CA/MBA measure:** track AI tokens/context from unnecessary log reads separately from runner minutes, elapsed test time and defect-escape rate. The goal is trustworthy verification at lower effort, **not a zero-test illusion**.

**Permanent one-line instruction:** *Run the required tests where they belong; show AI only the smallest truthful evidence necessary to make the next engineering decision.*
