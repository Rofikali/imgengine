# Temporary frontend-only security risk acceptance — Picomatch GHSA-fc5q-39xc-vjpf

**Decision date:** 2026-11-18. **Status:** Owner-requested temporary *non-production* exception **MERGED** in [PR #65](https://github.com/Rofikali/imgengine/pull/65), merge SHA `a9690d88ea7423b4fdd4ce8e54ba210c571d4fbf`. Exact PR head `ff9078025d4e265d561ecbb6b87bfacdcfa1468d` passed **7/7 required CI checks**. It remains an unpatched vulnerability, not production authorization. **Expires automatically:** **2026-12-02 00:00 UTC**, or sooner if the advisory changes, the upstream audit lists a patched version, severity/count changes, or a new high/critical finding appears. **Scope:** only the `Nuxt frontend verification` job of `imgengine-native-ci` for IMGENGINE development PR/main tests. **Not approved for production deployment, customer traffic, a commercial release or any other CI job.** This is a documented *risk acceptance*, not a repaired vulnerability and not proof of no exploitability.

## Observed evidence and why there is no quick upgrade

- 2026-11-18 GitHub Actions `pnpm audit --prod --json` reported **critical 0, high 2, moderate 2, low 0**. The two high counts are consistent with two affected dependency paths under one high advisory; the JSON diagnostics show exactly one distinct high advisory: `picomatch` / `GHSA-fc5q-39xc-vjpf`, with `patched_versions: <0.0.0`. Current reported paths: `nuxt > picomatch` and `nuxt > @nuxt/devtools > unstorage > anymatch > picomatch`.
- The advisory is [GHSA-fc5q-39xc-vjpf](https://github.com/advisories/GHSA-fc5q-39xc-vjpf) / CVE-2026-112366: picomatch through 4.0.7 may cause Node.js event-loop denial of service if an attacker can supply a specially crafted extglob pattern. **Do not claim attacker-controlled paths cannot reach the package without tracing the actual Nuxt runtime.**
- As checked on 2026-11-18, `picomatch 4.0.7` is the current registry version; advisory reports **no patched version**. Forcing a fabricated version or blind transitive-major override is not an acceptable fix. Two moderate `glob`/`minimatch` findings remain visible and should receive a separate compatible patch review.

## Exact technical exception contract (merged and time-boxed)

- The main audit still runs `pnpm audit --prod --json`, still logs severity counts, and still fails by default on **any high or critical** finding. No `pnpm audit --ignore`, `--ignore-unfixable`, global advisory suppression, changing audit level, or disabling CI.
- Only `imgengine-saas/frontend/scripts/check-production-audit.mjs` accepts the explicit `--allow-temporary-picomatch-in-frontend-ci` flag. `.github/workflows/ci.yml` supplies it **only to the Nuxt frontend verification job**. The checker validates `GITHUB_ACTIONS=true`, workflow `imgengine-native-ci`, job `frontend-verification` and `push`/`pull_request` event.
- Waiver eligibility requires **critical=0**, **high=2**, exactly **one distinct high advisory**, package **picomatch**, URL exactly **GHSA-fc5q-39xc-vjpf**, and **patched_versions='<0.0.0'**. It fails closed on unexpected/missing details, different package, extra high/critical advisory, changed counts, patched version now listed, expired clock, unknown flags, or use outside the expected GitHub frontend job.
- Every accepted run prints **TEMPORARY NON-PRODUCTION SECURITY EXCEPTION**, stating the dependency remains vulnerable and no production release is authorized. The five original checker regression tests and added waiver/expiry/adversarial tests must pass in exact-head GitHub CI.
- The exception is coded and documented **only for frontend verification**. Native fuzz/sanitizer, Rust API, Docker and SaaS integration jobs remain mandatory and unmodified by this decision. A green frontend job is **not** certification for production safety or full product release.

## Seven engineering levels, CTO, security and business sign-off questions

| Lens | Decision and required evidence |
| --- | --- |
| Junior / mid | Confirm actual pnpm 10.34.9 audit/lockfile output, distinct GHSA and two paths. Preserve normal frontend unit and browser behavior. |
| Senior | Regression tests must show only the named advisory passes; unknown new high or critical advisories fail. Do not alter project dependencies or lockfile just to green CI. |
| Lead | Record owner approval, expiry, GHSA, reviewer and exact PR/check SHA. Keep PR scope bounded and reversible. |
| Staff | Exception belongs to the frontend audit checker only; do not change global pnpm configuration, cross-workflow severity policy or deployment paths. |
| Principal | Treat untrusted extglobs as a potential event-loop DoS input until runtime reachability is proven. Reject production use and assess Nuxt code paths prior to launch. |
| CTO | Preserve momentum on browser-local Free tool development without inferring product maturity. Block production launch until mitigation/patch and explicit full gate review. |
| Security/SRE | No production deployment. Revisit advisory upstream and affected path after any Nuxt/picomatch release; remove exception immediately when fix is obtainable and tested; expire automatically on 2026-12-02. No downgrading of unrelated CI jobs. |
| CA/MBA/management | A short dev-only exception avoids repeated failed PR tests but accepts residual development risk. Do not quantify unmeasured savings. A public outage, reputational harm, support burden and liability are potentially much costlier; no production revenue/launch is permitted by this exception. |
| Logic/evidence | The finding and blocked CI are verified; inability to exploit in production is **not** verified. Successful CI under exception is explicitly `WAIVED`, not `RESOLVED`. |

## Remove immediately after upstream remediation

1. Check [upstream advisory](https://github.com/advisories/GHSA-fc5q-39xc-vjpf), [picomatch releases](https://github.com/micromatch/picomatch/releases), and the Nuxt dependency tree. A published patch is a **trigger to act**, not proof the lockfile updated automatically.
2. On a **separate security PR**, update compatible Nuxt/transitive packages and the pnpm lockfile with the project's pinned pnpm **inside Docker**. Do not manually edit lockfile entries.
3. Remove the explicit CLI waiver flag, exception evaluation/import/module, associated waiver-specific tests and this time-boxed override. Retain normal fail-closed audit diagnostics and new security tests. Run `pnpm install --frozen-lockfile`, `pnpm audit --prod`, checker, unit/typecheck/build, affected browser tests and seven exact-head CI jobs.
4. Verify **zero high/critical** advisories and independently review remaining moderate advisories and runtime exposure. Reconcile this decision as **CLOSED — PATCHED** in a new doc PR/commit.
5. If no patch is available before **2026-12-02 UTC**, frontend CI **fails closed again**. Any extension requires a NEW owner-approved security decision with explicit dates, evidence and PR review. No silent renewal.

**Monitoring is not automatic:** documentation and this checker cannot watch new releases without a separate scheduled monitor/automation. The owner should review this at least daily during the exception, or opt into an alerting automation. **Owner approval to temporarily pass this exact development-only finding does not authorize merge, deploy, secret changes, cloud spending or public release.**

**Post-merge follow-up:** Runner labels from [draft PR #64](https://github.com/Rofikali/imgengine/pull/64) are already merged via #65; do not duplicate them. The security waiver is still subject to automatic expiry and prompt patch removal. [Live security task](../next.md) owns ongoing state.
