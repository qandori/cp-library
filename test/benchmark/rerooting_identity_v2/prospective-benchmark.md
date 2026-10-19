# Rerooting identity-elision: separately bound protocol v2

Status: local correction implementation is separately authorized; independent review remains required. This document does not authorize remote publication, timing, launch, installation, or a retry. Do not repeat local timings: the prior local collector showed same-binary ratios of 0.59–1.09 and a previously 170–207 ms baseline batch later taking 4.33 s. Its environment is not suitable for performance qualification.

## 0. Explicit pre-comparison amendment and preserved failures

This separately reviewed protocol supersedes only the listed resource/admission/transport details of v1, whose SHA-256 is 5df8eea912d81db9ca7c79665c1ccbac487524f55c1b8f154fb9c333e9b1010f. It is declared before any v2 data. Production baseline/candidate headers, CSR, permanent unit, workloads, ten cells, seeds, compiler flags, timer boundaries, complete outputs, 100 ms floor, A/A and A/B schedules, gain/regression gates, and all inner deadlines remain unchanged.

The two distinct v1 push runs 39293044477 and 39293127017 must remain separate preserved failures. Each stopped in baseline-only cell-2 calibration at the 64-repetition cap before reaching the 100 ms floor. Neither reached beginning A/A, A/B, ending A/A, or any candidate real-timed batch. The second run was not an authorized retry; its distinct run number was discovered during final audit. The retained evidence cannot establish the duplicate trigger's cause. No duration/count from either run may seed v2 calibration, replace a v2 cell, support a speedup claim, or be pooled into a favorable result.

The explicit correction is maximum 128 repetitions, 512 MiB retained charge, 1 GiB per-worker RSS, and 2,304 MiB combined historical worker peaks plus collector. Collector RSS remains 256 MiB. Pair/matrix retained charges at 128 are respectively 469,784,576 and 419,428,352 bytes; these are accounting estimates, not RSS guarantees. Persistent input/reference caches, snapshot and canonical-byte temporaries, allocator behavior, and both workers' historical peaks still count. Cheap cells can still miss the floor, and 2 s batch, 5 s request, 360 s campaign, and 660 s full-experiment ceilings may still stop the run. No adequacy guarantee or adaptive threshold/cap relaxation is permitted.

One-shot admission uses a new unique workflow filename/ID with zero prior runs verified after staging and immediately before a separately authorized marker update. Both the workflow job and prepare adapter require run_number == 1 and run_attempt == 1, in addition to the bound branch/head/stage/marker predicates. Record both numbers. A skipped stage run consuming number 1 invalidates admission; never relax the guard to number 2, rename/recreate a workflow during the same experiment, or use a second trigger as a retry. A later materially revised experiment requires separate authorization and identity.

Log-only transport preserves every exact-byte file via deterministic stdlib gzip per file, then base64 chunks. Bind both original and compressed byte counts and SHA-256 hashes, all chunk indices/offsets, and the complete inventory/footer. Reconstruction must use bounded streaming decompression and reject truncation, corruption, trailing compressed data, oversize expansion, missing/duplicate chunks/files, or any hash/count mismatch. Known fixture, object/binary, source and bytecode records remain hash-only; no raw timing/provenance/control/failure record is omitted. The 60 s cap is unchanged. Both v1 streams reconstructed completely despite GitHub step timeouts; queued log draining is a possible mechanism, not a proven cause. Lossless compression reduces transport volume without changing benchmark samples or outcomes.

## 1. Scope, source, and architecture

One prospective public standard GitHub-hosted `ubuntu-26.04` job, one installed GCC 15.2 compiler, portable x86-64 code, no package or compiler installations, no paid/larger runner, no matrix of jobs. Use read-only repository permissions. Missing prerequisites or a changed scoped source hash stop the experiment; do not substitute another compiler, runner, or implementation.

- Exact baseline main: `9f05f01500656c65e394a8aa11c3f5afa4d9495a`.
- Baseline header SHA-256: `f7b2af0d128eb794a1342acc328becb82d1fd49075daa6db273d09dda308b828`.
- Candidate header SHA-256: `63ff812240127afd3313ac0001652cf01173c66d212063813f8d47fc3a27f5d4`.
- Shared CSR SHA-256: `2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7`.
- Final permanent unit SHA-256: `122d62eb83c84ee2f0771a52ebe6ffb3b4a7f7406051df7b5326128dcbd861fc`.

Reuse the reviewed Boolean collector architecture from `boolean-active-planes-local/benchmark`: separate baseline/candidate kernel translation units, common worker object, no LTO, observable external wrapper calls, persistent workers, disabled-by-default real clock, preallocated result slots, complete post-timer verification, baseline-only calibration, fixed A/A guards, injectable synthetic clocks, append-only raw logs, external watchdogs, and pre/post sealed manifests. Adapt only the workload/output representation, limits, and frozen cell/gate tables. Do not import Boolean historical performance or its source identity as qualification for rerooting.

Before any future launch, review the implementation and run only untimed/synthetic local tests of the adapted collector: complete schedule, every limit, missing/duplicate/corrupt responses, mismatches, early/late A/A failures, clock regression, hash changes, bounded cleanup, and the real-clock authorization gate. No real local timing probe.

## 2. Fixed ten cells

Fixed seeds: 857123, 857124, 857125. For each seed, generate each topology deterministically; shuffle vertex labels and edge insertion order, independently reverse edge directions, and save canonical inputs and hashes. Root at the permuted structural vertex 0, so paths are endpoint-rooted and stars center-rooted. Every cell uses initial repetition count 1.

| IDs | Monoid | Vertices | Shape order |
| --- | --- | ---: | --- |
| 1–5 | Distance sum / vertex count pair | 32,768 | path, centered star, balanced binary, broom, random |
| 6–10 | Dense 4-by-4 modular matrices | 8,192 | path, centered star, balanced binary, broom, random |

Topology generation is fixed as follows. Let shape index be 0..4 in listed order and initialize std::mt19937 with seed + 104729*shape_index. Generate parents in increasing v before permuting: path p(v)=v-1; star p(v)=0; balanced p(v)=(v-1)/2; broom p(v)=v-1 for v<n/2 and n/2-1 otherwise; random p(v)=rng()%v (only random topology consumes RNG during this parent stage). Shuffle labels with descending Fisher-Yates, using rng()%(i+1). Form mapped edges in increasing original v, consuming one rng()&1 to choose each direction, then shuffle edges by the same descending Fisher-Yates rule. Root is labels[0]. For matrices, use a separate std::mt19937 seeded by seed XOR 0x9e3779b9; fill matrices in final vertex-ID, row, column order with 1+rng()%(p-1). No hidden standard-distribution/shuffle state is used. Freeze all generated input hashes before launching measurement. Both binaries consume identical pre-generated bytes; neither implementation chooses inputs.

Pair DP is the existing legitimate distance-sum/count recurrence: merge adds the two components, vertex adds one to count, lift adds count to sum. It is the cheap-merge regression control.

Matrix entries are `uint32_t`, modulus p=1,000,000,007, with fixed-seed dense vertex matrices M_v. Unit is I, merge is ordinary matrix multiplication, add_vertex(x,v)=M_v*x, and lift returns x. Explicitly widen both operands to `uint64_t` before each multiplication, accumulate the four widened products in `uint64_t`, and then reduce (a wide accumulator alone does not prevent a narrow product overflow): `4*(p-1)^2` fits. The monoid is associative, two-sided-unital, and generally noncommutative; no inverse, identity-dependent shortcut, sparse specialization, volatile dummy work, or artificial callback delay is permitted. Vertex transforms are real ordered transfer-matrix DP work, not padding.

Path saves n-2 merges. Centered star saves zero. Record the independently computed expected reduction for every generated tree. A leaf-rooted star and forest/root/order/identity cases belong in untimed validation, not extra measured cells.

## 3. Measurement and complete output verification

Measure full fresh-tree construction, edge insertion, and one public `build`. One external wrapper call returns an owning opaque result handle retaining the built tree; preallocated empty handle slots exist before timing. Include wrapper object allocation, all graph/DP allocations and internal destruction, and result-handle assignment in the interval. Do not copy result vectors into a DTO within the interval merely to expose them to the worker. Input generation, slot allocation, full result traversal/comparison/hashing, tree/result destruction, and logging are outside the timer. Describe results as warm-call throughput with retained trees, not cold latency or caller-destruction-inclusive time.

Retain every complete result until its batch timer stops. Afterwards, compare every vertex value and both directional edge messages of every repetition against the saved exact reference, not just an aggregate checksum. Serialize canonical inputs and outputs from logical scalar fields only, using fixed-width little-endian integers: vertex/edge IDs, counts, and matrix entries are uint32, pair distance sums are int64 represented as two's-complement uint64 bits, and header fields are fixed-width/versioned. Never serialize raw std::pair, tree, container, padded, or native-endian object bytes. Compare logical values first. Save one canonical complete output per cell/seed/build as local binary evidence, cross-compare them byte-for-byte, and record hashes plus verified repetition/element counts for every batch. Do not serialize a duplicate full output for every identical repetition; raw duration, warmup, calibration, count, order, verification, and failure records must all be retained.

Before timing, validate all 10×3×2 full-size outputs and independent small-tree naive all-roots oracles. The matrix oracle must implement scalar dot-product reduction independently, verify I*A=A*I=A and associativity on fixed cases, and verify ordered DP on small trees. Untimed output reference preparation must not read or expose elapsed wrapper timings.

## 4. Compiler and environment provenance

Use the installed driver resolved from `g++-15`, or installed `g++` only if it independently identifies version 15.2.0. Freeze flags `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -fno-lto -march=x86-64 -mtune=generic`; no `-march=native`, profile-guided optimization, or added source attributes.

Record resolved driver path/hash/version, dumpmachine, dumpfullversion, search directories/specs, complete commands, objects, libraries, binary hashes, symbols, and disassembly verifying actual wrapper calls. Resolve the actual C++ backend with `-print-prog-name=cc1plus` and confirm it in a verbose compile trace. Accept the verified existing `/usr/lib/gcc/...` and `/usr/libexec/gcc/...` layouts after canonicalization; never assume only one hardcoded path. If the driver returns a relative program name, resolve it against the driver's reported search paths and confirm the same existing executable in the trace. Missing, ambiguous, or inconsistent backend identity stops the run. Hash that executable, and similarly record the resolved assembler/linker/collect2 paths when used.

Record OS/image identity, kernel, architecture/CPU model, microcode if exposed, logical CPUs, allowed affinity, memory, load, and available cgroup quota/throttling/pressure counters before/after. Missing optional counters are explicitly unknown, not inferred. Before samples, pin both workers and collector to the lowest allowed CPU and record it. No compiles, parallel tests, or additional workloads during calibration/measurement. Source and build/collector/protocol hashes are checked before and after, including failure cleanup.

## 5. Baseline-only calibration and fixed schedule

Reuse the collector's baseline-only doubling schedule and nine-pair design.

- Before every calibration, A/A, or A/B measured batch, execute exactly two same-build/cell/seed/count warmup batches with identical retention and verification. Warmup times are recorded for limits, excluded from statistics.
- For each cell in ID order, begin at 1 call. Run one measured baseline calibration batch per seed, in listed seed order, with those warmups. When all three durations reach 100 ms and none exceeds 2 s, freeze this cell's count. Otherwise double once and repeat the complete three-seed step, retaining all earlier observations. Do not reduce counts, retry a count, drop a seed, or use candidate durations to calibrate.
- Freeze all ten repetition counts and their hash before paired phases. Candidate and baseline use identical counts for every seed; no candidate recalibration.
- Beginning A/A: unchanged baseline binary as labels A and B on fixed cells 1,2,6,7, in that order.
- A/B: all ten cells in ID order, baseline A and candidate B.
- Ending A/A: repeat the exact beginning guard as a mandatory drift check, not a retry.
- Each cell/phase uses exactly nine pairs p=0..8; seed is seeds[p%3]; order AB for even p, BA for odd p. Each constituent batch receives its own two warmups. Use median of the nine paired B/A ratios, retaining all nine, not a ratio of independently selected medians.

This is 90 A/B pairs and 72 A/A pairs, 324 measured batches and 648 warmups after calibration. It is a single fixed experiment, not exploratory tuning.

## 6. Predeclared acceptance gates

Let r be candidate/baseline time per call (same count); smaller is better.

- Both beginning and ending A/A, every guard cell: median B/A in [1/1.03,1.03], and at least 7/9 individual ratios in [1/1.05,1.05]. Beginning failure stops before A/B. Ending failure invalidates the whole performance screen.
- Primary modest gain: matrix path (6) AND matrix balanced (8) each require median r <= 1/1.05, with at least 7/9 ratios <1. These test long chains and branching, both predicted to remove meaningful genuine matrix merges; neither is selected after looking at results.
- No material regression: ALL ten cells must have median r <=1.03 and at least 7/9 ratios <=1.05. This explicitly includes all five cheap-pair cells, both zero-savings stars, and matrix broom/random. No favorable aggregate erases a bad cell. Preserve and discuss every result.
- A complete valid screen failing a performance gate is blocked, not inconclusive noise to retry. Timeout, cap, output/provenance mismatch, failed A/A, missing cell, or logging failure is inconclusive. Do not claim success from an incomplete screen or repeat until it passes.

Passing means evidence for this bounded generic-rerooting improvement on this runner/compiler only. It establishes neither Library Checker fastest status nor Mac/Clang or exact-head CI qualification. Those remain separately required before any later merge.

## 7. Resource and wall limits

- At most 128 calls per batch; calibration counts are powers of two.
- Conservative retained-tree charge per call: `256 + 2*(8*m + (n+2*m)*sizeof(E))` bytes. The factor two covers vector/allocation overhead for the fixed no-growth construction and is a preallocation estimate, not a claim to inspect inaccessible vector capacities. Reject a count before allocation if total charge exceeds 512 MiB. Enforce real memory separately with the process RSS limits below; never silently lower the count if the 100 ms floor cannot fit.
- At most 1,024 MiB peak RSS per worker, 256 MiB collector RSS, and 2,304 MiB combined accounted peak RSS. Check outside timing; bounded input/reference caches count toward RSS. Store canonical inputs/outputs and raw evidence within 256 MiB total local artifacts; no GitHub artifact/cache/site upload is permitted. A storage/RSS violation stops the experiment.
- Any warmup or measured batch over 2 s fails. Each worker request has a 5 s external hard watchdog, including post-timer full comparison/destruction; terminate the process group and preserve diagnostics on expiry. Other subprocesses have deadlines derived from the remaining full budget.
- One 360 s collector campaign budget starts before full-size output preparation and includes all validation in that process, calibration, warmups, A/A, A/B, verification, destruction, logging, and final hash checks.
- One 660 s full experiment subprocess deadline starts before compiler/environment probes and includes compilation, synthetic/preflight validation, the complete campaign, and final metadata/hash collection. The workflow job has a 14-minute ceiling, with checkout at most 60 s, prepare at most 30 s, and lossless log transport at most 60 s. The sub-bounds sum to 810 s before scheduler overhead; these are ceilings, not a guarantee of completion. No process may outlive the full deadline. Budget exhaustion is terminal/inconclusive, not permission to omit ending A/A or shorten cells.
- No installs, automatic reruns, alternate flags, extra shapes, adjusted thresholds, increased caps, or source edits within the experiment. Preserve all failed/incomplete evidence. A later materially revised experiment requires separate review.

Rationale: ten cells keep scope small while separating eliminated expensive merges from cheap/no-elimination controls. The two sizes keep a complete result near 1.5 MiB, permitting baseline-only duration calibration without unbounded retained outputs. The 5% primary gate is modest but larger than the 3% median noise tolerance; the 3%/5% regression and A/A gates prevent appealing single timings from overriding stability. Reused collector scheduling and fail-closed guards reduce new infrastructure risk. No cost is added by choosing a public standard runner and existing toolchain, subject to verifying that setup before any future launch.
