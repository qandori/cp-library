# Direct-tree baseline qualification

This change adds an explicit alternative to waiting for successful exact-main receipts. It does not skip postmerge CI, replace current-head tests with cached results, or transfer documentation/deployment success from a branch to main. No product headers, unit/judge sources, intrinsic code, compiler flags, permissions, caches, or artifacts are added or changed.

## Unchanged v1

Version 1 retains the original schema, exact-baseline run requirements, reviewed control list, and two coverage disclaimers. Its production route does not import the new control-only helper. The existing source branch restriction, final marker, base/head/binding/source digest checks, ordered merge parents, clean checkout, and workflow checks remain required. New controls are bound only by the version-2 control list.

## Version 2

Version 2 uses policy `direct-tree-source-push-or-exact-baseline-v2`. The existing base, source/control hashes, and exact-main `runs` fields remain. One additional `qualification` object explicitly selects either:

- `kind: source-push`, with source workflow `runs` (exact ID and attempt) and `reviewed_source_control_sha256`.
- `kind: control-full`, with `reviewed_source_control_sha256`, exclusively for the bounded bootstrap below. Root `runs` are actual-main control-full receipts, not premerge receipts.

The canonical JSON file and exact field sets are validated. A source-push alternative is opt-in, not discovered from arbitrary green runs. Its exact-main fallback IDs remain explicit even while those main runs are pending. An unsupported, incomplete, unavailable, or mismatching source alternative falls back to those exact-main receipts. A known actual-main failure is a hard blocker, including failures in older attempts, unfinished jobs, or visible parts of incomplete API lists; it must never be caught as ordinary fallback.

### Source-push requirements

1. The baseline must be an ordered direct two-parent merge `[previous baseline, source]`, with a tree identical to its source parent. Both commits carry the original no-cache marker. The source is descended from the previous baseline and retains the exact reviewed historical branch, binding, and source digest.
2. Reconstruct source and merge inventories from immutable Git objects using the existing planner. Compare target lists, selected fingerprints and scope reasons. Commit-sensitive `plan_sha256` values are not compared across commits; source log hashes must match the reconstruction for that same source commit.
3. Require the current authoritative successful source `push` run and its exact successful attempt, repository, branch, SHA, workflow, complete unique jobs and executed-success required steps. The intrinsic job is independently mandatory, not inferred from the verify job's success.
4. Read logs belonging to those job IDs. Require the source plan, exact completed groups, source route, and runtime evidence. Intrinsic proof includes actual GCC 15.2.0, source controls, family-specific flags/modes, macros, command completions, binaries/dependencies and exact-source completion. Edit-distance/FPS generic mode is not labeled scalar.
5. Enumerate actual-main runs and bounded attempt histories before and after source log checks. Missing or incomplete history rejects source qualification. Any observed actual-main failure remains a blocker even when an older supplied exact-main receipt is green.

A successful alternative is reported as historical source-push execution on an equivalent main tree. It never says those tests executed on main, and preserves `full_baseline_target_coverage=not_independently_established` and `unaffected_current_head_checks=not_run`.

### Runtime evidence and limitations

Linux requires observed Linux x86_64, GCC and Clang versions, and Linux x86_64 compiler targets. Mac requires observed Darwin arm64 and Apple Clang with an arm64 Apple target. The existing intrinsic gate independently asserts exact GCC 15.2.0 and its required ISA behavior. These are observed historical environments within the workflow's required platform/compiler-family bounds; ordinary compiler versions are not claimed to equal versions on an unfinished main run.

The Linux unit step now prints uname and compiler version/target output before its unchanged test command. Existing older receipts lacking that output do not qualify. Mutable runner images, action tags, package/dependency versions, remote judge data and event/ref-specific behavior are not fixed by tree equality. Main CI and its failure monitoring remain necessary.

### Read-only log transport

Only existing Actions read permission is used. Token-bearing `api.github.com` requests never follow redirects. A signed-log redirect is accepted only for HTTPS `productionresultssa<digits>.blob.core.windows.net/actions-results/`, without userinfo, explicit port or fragment. It is followed using a new credential-free request that also rejects redirects. Reads are bounded to 16 MiB and 30 seconds per request. Unsupported hosts, redirect chains, unavailable logs and oversized responses do not establish evidence. Raw logs and signed URLs are not fixtures or committed artifacts.

## One bounded control-only bootstrap

The dedicated branch is `codex/ci-tree-reuse-20261018`, based exactly on `82fe5f034fca2805aeb59aee1be56216493ea5de`. Its final commit-message line is `Verify-Mode: control-full`. PR synthetic merges require the exact base/source ordering and identical tree. The actual-main merge must also be direct, identical-tree, onto that exact base, and carry the control-full final marker. Unmarked main introduction/change of the control route cannot fall into normal cache/deployment behavior.

Only the explicit CI-control/documentation path allowlist may change. Product, header, unit, judge, intrinsic, binding and test configuration files cannot change. Entire workflows must equal the original sources plus the exact approved selector/condition/compiler-identity additions; extra steps, environment changes, changed flags, altered checkout, cache/upload/deploy conditions and permission expansion are rejected.

The distinct `control-full` route executes complete ordinary Linux and Mac verification. It forces all exact-Git targets, validates selection/source again before recording completed groups, and does not read or write successful-result cache state. Existing normal-only cache actions, artifact upload and Pages deployment stay skipped. No new dispatch input, artifacts or permission changes are needed.

Intrinsic remains no-cache-production-only, so this control bootstrap explicitly reports `intrinsic_checks=not_run_control_only`. It never promotes skipped intrinsic into success. Only this unchanged-product control transition can use the actual-main full-bootstrap basis. Every later source-push alternative still requires intrinsic execution. Full ordinary execution does not upgrade the broader baseline/unaffected coverage claims.

## Local validation and historical evidence

The regression suites cover legacy isolation, exact source/merge provenance, control-only full selection without state writes, missing/skipped/duplicate execution, stale attempts, runtime gaps, incomplete-list failure veto, changed workflow execution, and safe log transport.

Retained real PR23 source-push logs at `247fc2a7cfffaad0b8c47caa98d056979e861507` were parsed offline without alteration or new remote requests. Linux and Mac selected-plan/completion validation passed. Mac runtime validation passed. Intrinsic command count, family modes/flags, source controls, dependency bytes and completion validation passed. Source and main `82fe5f0` selected target fingerprints match while their plan hashes differ. Linux runtime validation correctly rejected the absent actual platform/compiler identity output. The four new v2 control files are also absent historically, so those old receipts are not eligible for the new alternative. Synthetic fixtures exercise future complete identity evidence; they are not represented as real CI results.

Publication and remote execution require a separate review of the exact final diff. The dedicated PR and its actual-main merge must finish the control-full Linux/Mac workflows before the bootstrap can be used. This local change does not activate v2 for any existing optimization PR or alter its evidence JSON/binding.
