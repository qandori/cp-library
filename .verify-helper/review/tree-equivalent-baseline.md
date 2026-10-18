# Direct-tree baseline qualification

The current PR24 rollout is the immutable-anchor/direct-delta protocol below.
The earlier exact-base control-full route and its receipts are retained as historical behavior, not the current deployment plan.

This change adds an explicit alternative to waiting for successful exact-main receipts. It does not skip postmerge CI, replace current-head tests with cached results, or transfer documentation/deployment success from a branch to main. No product headers, unit/judge sources, intrinsic code, compiler flags, permissions, caches, or artifacts are added or changed.

## Unchanged v1

Version 1 retains the original schema, exact-baseline run requirements, reviewed control list, and two coverage disclaimers. Its production route does not import the new control-only helper. The existing source branch restriction, final marker, base/head/binding/source digest checks, ordered merge parents, clean checkout, and workflow checks remain required. New controls are bound only by the version-2 control list.

## Version 2

Version 2 uses policy `direct-tree-source-push-or-exact-baseline-v2`. The existing base, source/control hashes, and exact-main `runs` fields remain. One additional `qualification` object explicitly selects either:

- `kind: source-push`, with source workflow `runs` (exact ID and attempt) and `reviewed_source_control_sha256`.
- `kind: control-full`, with `reviewed_source_control_sha256`, for the retained exact-base bootstrap. Root `runs` are actual-main control-full receipts, not premerge receipts.
- `kind: control-delta`, with `reviewed_source_control_sha256`, for the current moving-main bootstrap below. Root `runs` are actual-main delta receipts; their complete ordinary coverage is a direct-success union, not fresh full execution.

The canonical JSON file and exact field sets are validated. A source-push alternative is opt-in, not discovered from arbitrary green runs. Its exact-main fallback IDs remain explicit even while those main runs are pending. An unsupported, incomplete, unavailable, or mismatching source alternative falls back to those exact-main receipts. A known actual-main failure is a hard blocker, including failures in older attempts, unfinished jobs, or visible parts of incomplete API lists; it must never be caught as ordinary fallback.

The historical source hash uses that immutable source's own declared control inventory, parsed as AST data without executing historical code. All fourteen core/v2 controls are mandatory; paths must be unique, valid and present regular files. Later benchmark-control renames therefore do not change the historical proof. The current candidate control hash is checked independently against its current inventory.

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

GitHub's real multi-chunk logs may begin a later timestamped line with a UTF-8 BOM. The parser permits one BOM only immediately before an otherwise-strict timestamp, never untimestamped text or malformed lines.

## Retained exact-base control-only bootstrap

The dedicated branch is `codex/ci-tree-reuse-20261018`, based exactly on `7fb5f2897349aa0a11c18e3ab56f3e4815319017`. Its final commit-message line is `Verify-Mode: control-full`. PR synthetic merges require the exact base/source ordering and identical tree. The actual-main merge must also be direct, identical-tree, onto that exact base, and carry the control-full final marker. Unmarked main introduction/change of the control route cannot fall into normal cache/deployment behavior.

Only the explicit CI-control/documentation path allowlist may change. Product, header, unit, judge, intrinsic, binding and test configuration files cannot change. Entire workflows must equal the original sources plus the exact approved selector/condition/compiler-identity additions; extra steps, environment changes, changed flags, altered checkout, cache/upload/deploy conditions and permission expansion are rejected.

The distinct `control-full` route executes complete ordinary Linux and Mac verification. It forces all exact-Git targets, validates selection/source again before recording completed groups, and does not read or write successful-result cache state. Existing normal-only cache actions, artifact upload and Pages deployment stay skipped. No new dispatch input, artifacts or permission changes are needed.

This one bootstrap has a fixed 120-minute Linux job ceiling on the dedicated source push/PR or exact-base, marked main-push context; every ordinary mode retains 60 minutes. The full route still validates all provenance before tests. Earlier 60-minute cold runs were cancelled during judges after 76–77 of 120 targets, dominated by external test-data generation; these cancelled runs are not qualifying evidence. After main advanced, its product changes are preserved unchanged and the full inventory is now 126 headers/bundles, 104 units/sanitizers, and 122 judges. The larger bound does not change runners, tests, flags or permissions, and is a maximum rather than a requested duration.

Intrinsic remains no-cache-production-only, so this control bootstrap explicitly reports `intrinsic_checks=not_run_control_only`. It never promotes skipped intrinsic into success. Only this unchanged-product control transition can use the actual-main full-bootstrap basis. Every later source-push alternative still requires intrinsic execution. Full ordinary execution does not upgrade the broader baseline/unaffected coverage claims.

## Local validation and historical evidence

The regression suites cover legacy isolation, exact source/merge provenance, control-only full selection without state writes, missing/skipped/duplicate execution, stale attempts, runtime gaps, incomplete-list failure veto, changed workflow execution, and safe log transport.

Retained real PR23 source-push logs at `247fc2a7cfffaad0b8c47caa98d056979e861507` were parsed offline without alteration or new remote requests. Linux and Mac selected-plan/completion validation passed. Mac runtime validation passed. Intrinsic command count, family modes/flags, source controls, dependency bytes and completion validation passed. Source and main `82fe5f0` selected target fingerprints match while their plan hashes differ. Linux runtime validation correctly rejected the absent actual platform/compiler identity output. The four new v2 control files are also absent historically, so those old receipts are not eligible for the new alternative. Synthetic fixtures exercise future complete identity evidence; they are not represented as real CI results.

Publication and remote execution require a separate review of the exact final diff. The historical exact-base route required control-full Linux/Mac completion; the current rollout instead requires the anchor and exact-main delta proof below. This local change does not activate v2 for any existing optimization PR or alter its evidence JSON/binding.

## Moving-main rollout: immutable anchor and direct deltas

The exact-base `control-full` route above and its completed b498 receipts remain
historical, not relabeled as current-main or complete-runtime evidence. The
rollout now uses one immutable source-only anchor on
`codex/ci-tree-anchor-20261018`, with final marker `Verify-Mode: control-anchor`.
There is no anchor PR or manual dispatch. Its reviewed product base is
`3dd3db7c749ba3c00d432fc82b085146f86c4b0e`; independent main advances do not alter
that commit or restart its full ordinary Linux/Mac checks.

The integration branch remains `codex/ci-tree-reuse-20261018`, now using final
marker `Verify-Mode: control-delta`. Each source integration has ordered parents
`[previous reviewed source/history, current main]` and changes only the explicit
control/documentation allowlist relative to that main parent. Any upstream CI/helper/workflow/test-driver input change since
the reviewed anchor base stops integration for renewed review, rather than being
overwritten automatically. The anchor is
merged into that source's ancestry without force, then remains reachable from
main after the final direct merge. Anchor-branch cleanup is separate. PR and
actual-main merges require ordered `[base, source]` parents and the identical
source tree. A moved-base PR routing rejection is not a product-test pass; useful
source push evidence may still finish on its original integration base.

`.verify-helper/control-receipts.json` is a canonical, explicit list of at most
eight exact push run/attempt identities, including one Linux and one Mac anchor
at the same ancestor commit. It is not cached success state. Every donor is an
ancestor, has authoritative successful run/attempt/job and required executed
steps, and supplies only the target groups it actually executed. An inherited
target in an earlier delta cannot be promoted into direct execution in a later
receipt. No PR result, omitted group, failed/skipped check, missing log or unknown
context establishes coverage. Receipt inputs for an active anchor/delta are not
repushed merely because main moves.

After a source delta D1 succeeds, a separately reviewed metadata-only integration
D2 may add D1's direct Linux/Mac push receipt IDs to the bounded input list. D1
must already be an ancestor; a commit cannot include its own future run IDs. D2
retains the exact current-main integration parent/marker and fresh control/docs
checks, while unchanged product targets may inherit anchor plus D1 execution.
Only D1's executed targets are exported as direct donors, never its inherited
ones. This can avoid repeating the same changed targets on the latest PR and
actual-main run. It is an explicit reviewed publication, not automatic metadata
mutation or an extra cache layer.

For each current platform/group/target, reuse requires identical complete
repository source closure, reviewed check/helper/workflow/configuration bytes,
and the defined observed runtime profile. The profile hashes ci.py's existing
per-group environment key: OS/architecture/image identity, full compiler version
output, six relevant ambient variables, and Python/package versions for judge
and bundle groups. Only presence and SHA256 of ambient values are emitted,
never their raw contents or credentials. Linux also binds installed
`libboost-dev` and its exact versioned Boost header-package dependency. Missing
or ambiguous dependency identity fails closed. Changed context/checks invalidate
that group; changed, new or uncertain source closures execute freshly. Unused
Python distributions do not invalidate C++ header/unit/sanitizer groups.

This is a bounded historical check/runtime profile, not a hermetic environment
proof or existing-cache equivalence. Inherited judge results prove past execution
under the recorded profile, not freshly downloaded current remote judge data or
floating-action identity. Remote judge/checker changes that are actually known
must invalidate the relevant evidence; the optional sorted `fresh_groups` input
forces complete named groups fresh, and cannot make a failed/unknown donor qualify.
Known relevant failures remain blockers.
The implementation does not freeze all external validation inputs. Ref-dependent
documentation/site generation and control tests run freshly. Inherited checks
are explicitly NOT EXECUTED NOW, including on actual main.

Logs contain bounded context records, 24-target chunks, and coverage/completion
digests; inventories and fingerprints are reconstructed from immutable Git.
Every JSON record is bounded to 8 KiB, and the existing 16 MiB whole-log limit
remains. Producer run ID, attempt and workflow job key come from actual Actions
context and must match the authoritative API-associated job log. Numeric job IDs
are taken only from API receipts, not invented from runner environment.

All platform groups must be disjointly complete as `executed now ∪ inherited`.
Linux covers headers, units, sanitizers, judges and bundles; Mac covers headers
and units only. Source push, PR and actual-main delta workflows remain enabled. Marked
control-delta main pushes append their exact commit to the concurrency key, so
a later ordinary main update cannot cancel the required actual-main bootstrap.
Every ordinary concurrency key and cancellation behavior stays unchanged. The
three control markers are reserved: quoting one in a message without the matching
valid final nonempty marker rejects the route before normal cache/product work.
The workflow prefilter intentionally tolerates trailing newlines; the source
router enforces the exact final-marker/ref contract.
Ordinary modes retain their existing flags, runners and limits. Dedicated routes
have a fixed 120-minute Linux ceiling (ordinary Linux 60, Mac 30), with exact
routing checked before product execution. No cache reads/writes, artifact uploads
or Pages deployment are allowed. Intrinsic remains explicitly NOT RUN in this
control-only transition, and later production source-push proof still requires
its original intrinsic gate.

Version 2 additionally accepts explicit `qualification.kind: control-delta` for
this actual-main control transition. Its exact-main Linux/Mac receipts must
reconstruct the direct successful donor union, and main failure history is
checked before and after. It does not upgrade the unchanged broad baseline and
unaffected-coverage disclaimers. Product optimization API, Mac, benchmark and
latest-head merge gates are unchanged.

Worst case: a changed compiler/image/profile or reviewed check implementation
invalidates all corresponding groups, which must execute freshly. Missing or
expired receipts fail closed; dropping a no-longer-useful bounded input causes
its uncovered targets to execute again, rather than inventing evidence. An
anchor is not restarted solely because main advances. No general cache service,
new permissions or persistent artifact store is introduced.
