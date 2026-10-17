# Phase 2 individual confirmation and regression-resolution screen

This fixed protocol confirms individual mechanisms and resolves prior regression signals. It is not final performance or merge qualification. One workflow contains 14 timing shards; all candidate-specific correctness prerequisites must pass first.

The authoritative case, schedule, source and compiler definitions are in plan.json. The complete package is recorded in MANIFEST.sha256.

## Fixed scope

Comparison baseline and temporary branch parent: b584f748d67d38faf22229cb2c971ccd98ba7d6e. Prior screening evidence remains pinned to829efa17b452f904eef5558930d91dd3792feffa. The target headers, nonstandard include closure, three relevant unit sources, AGENTS.md and verify/Mac workflows are byte-identical between those commits; baseline-refresh.json records the checks. Concurrent unrelated changes are preserved.

Exactly seven individuals: neighbors, uniformneighbors, selectprefix, rangeprefix, pdep, avxbuild, leaf. No combinations, selectkernels, width-performance or Titan references in the first wave. Their later experiments use separately frozen protocols.

56 baseline/candidate pair-cases in 14 shards of four. Every individual includes access/mixed/updates plus independent access-only and ordinary-mixed caller controls. Relevant query individuals also have short-query controls. Intended targets are neighbors full/duplicate (plus equal for uniformneighbors), select full/exact-valid-duplicate, range full/narrow, build full/duplicate/equal, and leaf small/boundary/local updates. Build/rebuild measurement cases have q=0. All case parameters and source-header hashes are explicit in JSON; do not infer cases from the later-stage plan.

Fixed held-out timing/preflight seed: **20261017**, not screening seed 713. Extra correctness seed: **1709**. Do not tune or replace a seed after seeing results.

## Caller mapping

- `caller=broad_dispatch`, `caller_shape=0`: bench.cpp.
- `caller=independent_access_only`, `caller_shape=1`: caller.cpp with CALLER_SHAPE=1; kind=access.
- `caller=independent_mixed_ordinary`, `caller_shape=2`: caller.cpp with CALLER_SHAPE=2; kind=simplemixed. Each operation independently uses rng()%4 to choose equally among access, set, kth_smallest and three-argument range_freq.

The ordinary mixed caller is **not** a fixed 4/2/1/1 cycle and contains no rank operation. It is **not the same trace** as broad mixed. These two caller controls supplement the broad harness. Do not force noinline/always_inline or artificial layout changes to flatten differences.

## Exact timing/calibration policy

Compile both native and explicit scalar with actual GCC 15.2.0 and run them sequentially on the **same public hosted runner and CPU affinity per shard**. Track order is explicitly fixed per shard. Each two-shard candidate group has opposite orders, and the first order alternates across candidate groups, balancing target/adverse and independent caller roles. Scalar includes -march=x86-64, explicit AVX/AVX2/AVX512F/BMI/BMI2 disables and disabled loop/SLP auto-vectorization. Log preprocessor ISA macros and audit generated scalar instructions. Old compiler-default results are not scalar evidence.

One full-size three-replay equality preflight, one separate small three-replay independent-vector-oracle preflight (n<=512, q<=512, seed 1709), and one one-replay pilot per distinct variant/case/flag. For an exact case/flag in the shard, use one common replay count:

R = min(4096, max(1, ceil(1.5 / minimum pilot CPU seconds across its variants))).

Freeze R before measurements and reuse it identically for AA, BB and all cross-variant legs. Both calibration and candidates therefore use the same duration policy, unlike the initial screen's 1.5-second AA versus 0.4-second candidates. Record actual durations, slow-leg inflation and any sample below 1.0 CPU second; do not retune/repeat until favorable.

Four fixed cycles, with the cycle outermost over the shard's cases and deterministic cyclic/reversed case order:

- even cycle: AA, ABBA, BB, BAAB
- odd cycle: BB, BAAB, AA, ABBA

This is 48 measured processes per pair/case/flag, eight cross blocks plus four AA and four BB pairs, interleaved throughout target/control families. For every complete ABBA/BAAB block log D=(log(B1)+log(B2)-log(A1)-log(A2))/2 and exp(D), the B/A latency ratio. Use the eight complete block contrasts as the analysis units, not 16 independent A/B observations. For AA/BB retain signed log(second/first) drift and the absolute calibration envelope separately. Small 1–3% regressions remain unresolved unless matched interleaved evidence and a predeclared margin resolve them; overlap with noise is not equivalence.

## Approved validation boundary

- Full-size preflight: every one of three fresh-object replays checks exact full output arrays and full final state; complete input/output/final bytes are compared against baseline and the corresponding baseline flag run. A separate independent C++ vector oracle validates the complete measured input, every operation output and final state once per exact canonical input/kind/value_type/bits key. Only after success may later candidate/flag preflights reuse that verified expected record, requiring exact input and full output/final byte equality. Source/input/output hashes establish provenance but do not replace byte equality.
- Independent oracle preflight: a separate small deterministic case (n<=512, q<=512, seed 1709) checks every output and full final vector against the Python vector oracle, with three exact fresh-object replays. For restored select-churn use q divisible by ten (e.g. 500 rather than 512) and n>=2. The small Python oracle supplements, never substitutes for, the full-size C++ oracle and cross-variant/replay equality.
- Timed operation replays: output hash checked on each replay; full final-state readback only on the first timed replay.
- Timed build-only: actual object escaped every replay; full state checked in preflight and only the first timed replay. Do not claim each individual timed build's final state was read back.
- Warm-cache nonempty fresh objects receive 1024 access calls before timing; build-from-empty has no prebuilt object, so no object-access warmup. No full-workload prewarm.
- Input generation, setup except explicit build rows, warmup, destruction, digest and readback stay outside timers. Per-operation output stores and approved constant-cost escapes stay inside.

The runner must NOT globally enable DWM_VALIDATE_REPLAYS for timed samples; use it for preflights. Preserve the existing approved harness boundary. Benchmark CLI is kind N Q SEED DIST LOC REPS CACHE DUMP; dump plus .input sidecar can be verified ephemerally, but never printed/uploaded on success.

The current select-churn draft has a ten-operation cycle: select absent, set p, select singleton, set p+1, select later duplicate, restore p+1, select singleton, select k1 miss, restore p, select absent. This is four sets and six selects. Claim restored state only when q is divisible by ten; the correctness gate must enforce that condition for restoration tests.

Own-library correctness remains a gate before timing, including the reviewed exact names select-lastbit-miss, select-high-k, select-valid-dup, select-churn and Bits64 narrow20. The dupselect timing row is exactly kind=select-valid-dup, so valid later duplicate occurrences ARE timed in this first wave. Last-bit missing (select-lastbit-miss), excessive k (select-high-k), churn (select-churn), Bits64 narrow20 and wider-domain variants are correctness-only here or deferred to later timing packages. Later select factorial and wide performance packages remain separately gated. Actual candidate Mac tests and latest-head required CI remain merge conditions.

## Counts and bounds

Across both flags: 56 pair-cases, 14 timing jobs, 5,376 measurement processes, 224 full-size preflight processes, 224 small oracle-preflight processes, 224 pilot processes = **6,048 benchmark processes**, plus 224 small Python oracle invocations and at most 56 full-input C++ oracle calls. **114 benchmark binaries** require 114 source-object compiles and 114 separate links. Add 28 per-flag escape-object compiles and 14 full-oracle compile/links: **270 g++ invocations, 156 compile-bearing calls**; compiler identity/ISA probes are additional metadata commands. Nominal measured CPU: **8,064 seconds = 2.24 hours**, excluding slow-leg inflation, setup, oracles, compilation and pilots. This is not wall-time or billing prediction.

Each shard: four comparisons, 432 benchmark processes, 16 small Python oracle verifiers plus at most 4 full-input C++ oracle calls, one oracle compile/link and two per-flag escape-object compiles, 576 nominal timed CPU seconds; 39-minute internal deadline with a 60-second shutdown reserve under the 40-minute execution step/45-minute job; 120-second per-command bound. A failure or exhausted bound makes the job incomplete, not permission to drop rows or rerun until favorable. At most two timing shards concurrently. Linux/Mac/sanitizer gates and normal CI may run concurrently; the limit is not a global workflow-job cap. Own-library individual correctness gates the first wave. First-wave sanitizers cover all eight sources, baseline plus all seven individuals, split into two four-variant jobs with 32 source-suite compile/run pairs per job; no combination-only shortcut. Detailed Linux/Mac/sanitizer counts and bounds belong to the separate reviewed correctness-gate manifest, not the timing totals. Combination correctness and direct ablations are separate later stages.

Standard public ubuntu-26.04 only; read-only contents; checkout credentials not persisted. No artifact uploads, persistent cache/ccache/sccache writes, paid/larger/self-hosted runners, custom images or billing changes. Outputs only normal logs and GITHUB_STEP_SUMMARY. Temporary compiler/validation files in the disposable job workspace are not a persistent cache.

Timing normal-log cap: <=8 MiB and summary <=32 KiB. The separate correctness-gate normal-log cap is 24 MiB per gate, with a 39-minute internal deadline under its 40-minute step/45-minute job limits. These are distinct budgets, not a shared cap. Timing logs use bounded per-record output. Success records retain source/compiler/binary/input/output/final-state SHA256 and byte lengths, deterministic case/seed/generator/replay/order parameters, full-size independent-oracle or verified exact-byte reuse, replay equality and separate small-oracle outcomes and all timed/calibration results, never full arrays. On failure retain exact reproduction parameters, first mismatched operation/value/tag, bounded surrounding operations/final-state window and diagnostics (<=64 KiB). Missing/truncated required evidence makes the result incomplete. No artifact fallback.

## Fixed shard assignment

Every shard mixes intended targets with adverse or independently shaped caller controls. No control-only shard remains. The uniformneighbors equal target shares shard 10 with PDEP because those families contain nine and seven rows respectively. All 56 rows are retained exactly once.

- first-wave-01 (native then scalar): neighbors/neighbors, neighbors/access, neighbors/short, neighbors/access_only.
- first-wave-02 (scalar then native): neighbors/dupneighbor, neighbors/mixed, neighbors/updates, neighbors/mixed_ordinary.
- first-wave-03 (scalar then native): uniformneighbors/neighbors, uniformneighbors/access, uniformneighbors/short, uniformneighbors/access_only.
- first-wave-04 (native then scalar): uniformneighbors/dupneighbor, uniformneighbors/mixed, uniformneighbors/updates, uniformneighbors/mixed_ordinary.
- first-wave-05 (native then scalar): selectprefix/select, selectprefix/access, selectprefix/short, selectprefix/access_only.
- first-wave-06 (scalar then native): selectprefix/dupselect, selectprefix/mixed, selectprefix/updates, selectprefix/mixed_ordinary.
- first-wave-07 (scalar then native): rangeprefix/range, rangeprefix/access, rangeprefix/short, rangeprefix/access_only.
- first-wave-08 (native then scalar): rangeprefix/narrow, rangeprefix/mixed, rangeprefix/updates, rangeprefix/mixed_ordinary.
- first-wave-09 (native then scalar): pdep/select, pdep/access, pdep/updates, pdep/access_only.
- first-wave-10 (scalar then native): pdep/dupselect, pdep/mixed, pdep/mixed_ordinary, uniformneighbors/equalneighbor.
- first-wave-11 (scalar then native): avxbuild/build, avxbuild/buildequal, avxbuild/access, avxbuild/access_only.
- first-wave-12 (native then scalar): avxbuild/builddup, avxbuild/mixed, avxbuild/updates, avxbuild/mixed_ordinary.
- first-wave-13 (native then scalar): leaf/smallupdate, leaf/boundaryupdate, leaf/access, leaf/access_only.
- first-wave-14 (scalar then native): leaf/localupdate, leaf/mixed, leaf/updates, leaf/mixed_ordinary.

The redistributed layout changes benchmark binaries from 112 to 114, but retains exactly 224 small Python oracle invocations and 8,064 nominal timed CPU seconds. The full-input C++ oracle adds 14 compile/link invocations and at most 56 verifier calls, not timing rows/jobs. The separately stated small-oracle benchmark preflight increases the original 5,824 benchmark-process estimate to 6,048. Flag orders are explicit per shard, opposite within each two-shard candidate group and alternating by group. Targets are split 8 native-first/9 scalar-first; access-only caller rows 4/3, ordinary-mixed caller rows 3/4, and short-query controls 2/2. The uniformneighbors equal target is the disclosed third-shard exception caused by its nine rows.

## Full-input oracle policy

Compile vector_oracle.cpp once per shard with the recorded GCC 15.2.0 identity and fixed -O2 -std=gnu++23 -march=x86-64 flags. Record source, compiler command and binary hashes. Cross-check its semantics with the existing independent Python oracle on small cases before the source freeze.

Run the C++ oracle once for each distinct canonical full input, kind and value-type/bits key, on a baseline-produced dump. This means at most 4 calls per four-case shard, or 56 across the wave; exact duplicate input keys may reduce this. After success, retain a disposable job-local verified input/output/final record, and require complete byte equality before reusing it for candidates or the other flag. Failure and timeout keys remain failures without retries; a 120-second timeout is incomplete validation, never a baseline-only fallback. These ephemeral verification records are not persistent compiler/result caches and are not uploaded.

## Package contract

The 0.5% AA/BB multiplicative envelope and 1% adverse-control margin are explicit descriptive flags only, never automatic acceptance or merge criteria. Case order is rotate-left-by-cycle, then reverse on odd cycles.

Each benchmark source and escape source compiles separately with its own -MMD/-MF dependency file, followed by a separate link. Dependency records must prove both product headers come from the selected variant with frozen bytes. The escape and independent oracle must have no product-header dependencies. Counts distinguish benchmark binaries, source compiles, escape compiles and links.

plan.json pins the baseline, ordered component patches, resulting header hashes, exact cases and schedule, compiler flags and source hashes. run.py and gate.py verify the materialized sources before execution. The normal verify/Mac workflow files are included here for byte-identical temporary-branch installation; only their persistent-cache steps are removed, with test commands and main-only deployment guards unchanged.

## Correctness jobs

Three normal jobs cover native GCC15.2, explicit x86-64 scalar GCC15.2 and native ARM64 Apple Clang. Each uses 105 build invocations, 64 source-suite runs, 880 exact-three-replay processes and 880 independent Python oracle checks. Two native Linux Clang ASan/UBSan jobs each build/run32 suites, together covering baseline and all seven measured candidates. Actual compiler dependency files prove the selected candidate headers were compiled. These gate counts are separate from timing totals.

Correctness logs are bounded at24MiB per job; timing logs at8MiB. Both use ordinary workflow logs only, with bounded summaries and no artifact/cache uploads. Every job uses a39-minute internal deadline,40-minute step and45-minute job ceiling. These are abort bounds, not promised completion times. Standard public Linux/macOS runners only; the two-shard parallelism cap is not a global cap on prerequisite or normal CI jobs.
