# Frozen stacked tree-only qualification after PR13

Status: source preparation only; no stacked compilation, timing, publication, or merge has been performed. Baseline is pinned to merged PR13 commit 31d238c62d0b297b1ffaf8b32a9b08461c2cadd9. This protocol specifies a single remote validation run. Historical pre-SIMD evidence remains intact and does not qualify this incremental candidate.

## Frozen proposed source identities

- Baseline header: ac33415ec7aaead7adba8e1114f1ffc303a666c10549158be1ff1612fc839a94.
- Shared direct helper: 73f3c335566cad50271ed5a0c8336ec10c57c6ec2c69a193c463939d15136380.
- Proposed candidate header: 4cb93dd9d9d2b0bbbece729c7de93217a19cd4915cf9ace9aa797c0eefcb049e.
- Narrow two-hunk patch: e6f20e1e42238d3b8b04ea2dfe884c8776baccbccb94cd37112e2a8472871d4e.
- Renamed baseline: 6f99a23ad5154ce6c91365cac199d58d2bc18d8057207e8efe0cbf7817e8e416.
- Benchmark source: a84c5588a54f1a5fc871258fd6be26296b5db78f2d25a01872b95e9a8e3c3fc2.
- Shapes: 186177732627bf375ceec481d8854457aa117c2f21ad3899434b9691bcbbb3f9.

The SIMD direct route and its gates are byte-identical between baseline and proposed candidate. Only the two tree-specific hunks change: size starts at 1; unchanged degree-sized root division is followed by suffix retention. Never overlay the historical pre-SIMD full header. Apply only tree-only.patch after verifying the exact baseline hash, then verify the proposed candidate hash.

source-manifest.json lists the complete actual seven-header recursive closure. Six dependencies, including the direct helper, are shared byte-identically. The five pre-existing dependencies also match the historical b15be548 experiment. The closure was verified against a git archive of that exact merge commit during source-only preparation. The remote runner must verify it again; do not scan every unrelated repository header or silently accept mismatches.

The renamed baseline is generated from the exact original PR13 header by renaming its function and header guard, then normalizing each quoted include relative to the original fps directory. There are now three direct includes, including components/fps_direct_eval.hpp. This is a mechanical harness change, not a production or benchmark algorithm change. The historical reconstruction wrapper is NOT directly reusable because its frozen baseline/candidate hashes and two-include conversion belong to the pre-SIMD experiment.

## One fixed remote comparison

Run on one isolated Linux x86_64 GitHub-hosted VM, pinned CPU 0 if available. If CPU 0 is unavailable, pin and record an allowed CPU before the run; do not silently switch. Record GCC identity, flags, /proc/cpuinfo, affinity, exact baseline commit, candidate source hash, and recursive closure hashes. Compile serially outside timing. Use the unchanged benchmark.cpp and shapes.inc with the newly renamed SIMD baseline and stacked candidate. Build generic -std=c++20 -O2 and -std=c++20 -O2 -march=native. Perform the full 44-shape, 8-balanced-pair protocol for each build, including all unchanged direct controls. No selective retry, changed pair count, reduced grid, benchmark tuning, or cherry-picking. Keep all observations and checksums as workflow artifacts, not committed raw logs.

Before timing, run the retained independent ordinary modular Horner tests, exact baseline/candidate equality, generic PlainMint, default and forced-scalar checks, existing affected units, and independent adversarial tests as permitted by the remote job. ASan+UBSan uses detect_leaks=0 if the runner's leak detector is unsupported. Required latest-head CI and Apple Clang/macOS tests still gate merge separately. No source change is justified by a failed benchmark without a new independently declared experiment.

After timing, optionally reproduce the same four-shape separate-process allocation and structural harnesses against the stacked pair. Identical explicit baseline-then-candidate warmup, no allocation hooks during timing, preserve the degree-sized quotient-capacity caveat.

The numeric acceptance gates below are unchanged. Report paired CPU/wall medians, ratio of independent medians, absolute costs, every adverse row, and extreme pairs. Passing the numeric gate does not automatically resolve historical control instability: an independent review must assess the fresh complete control evidence. If instability or regressions remain unexplained, keep merge HOLD rather than rerun until green. Old scalar-direct control anomalies and the old 1.26–1.27x target benefit remain historical evidence, not fresh measurements on this stacked source.

## Unchanged shape, measurement, and acceptance protocol

Baseline is the exact PR13 SIMD header and helper identified above. One variant only: point-sized product tree, unchanged degree-sized root division, quotient suffix erase. No SIMD change.
Same process baseline renamed from exact original header; shared unchanged dependencies. CPU 0. GCC C++20 O2 and O2 -march=native. 8 balanced alternating pairs, reverse shape traversal in odd pairs. Each variant batch >=0.15 actual process CPU seconds, clocks recorded per batch and per operation. Full output equality before timing, full checksums on every timed result. Seed 491991+shape index. No exclusions or selective reruns. Report all rows, median paired speedup and ratio of median times, absolute CPU/wall costs. Memory allocation accounting separately, no instrumented timing.
Shapes (44): [(1024, 33), (1024, 63), (1024, 65), (1024, 129), (1024, 128), (1024, 512), (1024, 1024), (4096, 33), (4096, 63), (4096, 65), (4096, 129), (4096, 512), (4096, 2048), (4096, 4096), (32768, 33), (32768, 63), (32768, 65), (32768, 129), (32768, 4096), (32768, 16384), (32768, 32768), (131072, 33), (131072, 63), (131072, 65), (131072, 129), (131072, 16384), (131072, 65536), (131072, 131072), (1025, 33), (1025, 129), (1025, 1025), (4097, 33), (4097, 129), (4097, 1025), (4097, 4097), (32769, 33), (32769, 129), (32769, 1025), (32769, 32769), (33, 1024), (129, 32768), (32, 4096), (4096, 32), (32768, 16385)]
Independent ordinary uint64 modular Horner oracle, PlainMint, boundary/empty/zero/duplicate controls, root equality, current FPS tests, ASan+UBSan detect_leaks=0. Native Mac CI pending external publication.
Reference read-only AC409516 https://judge.yosupo.jp/submission/409516 lines7631-7646: point-sized tree plus quotient suffix window, but different root division, no code copied or executed.

Predeclared acceptance before first timing: target group is n>=1024 and 33<=m<=129. Each build must show >=1.10 geometric mean of per-shape paired-median CPU speedups across this target group, with at least 75% of target shapes >=1.05. No shape may show a >5% paired-median CPU regression (speedup<1/1.05); any such adverse row blocks publication pending independent explanation/qualification, not silent removal or a rerun-until-green. Unchanged controls (point-sized and degree-sized tree already equal, plus direct path) must meet this no-material-regression rule separately. Report wall metrics alongside CPU, every adverse row, ratio-of-medians and absolute costs. A failed gate is a hold, not permission to select a different subset or tune thresholds after observing results.
Memory accounting: root.erase preserves degree-sized quotient capacity. Root div_series transforms remain degree-sized. Report separate warm peak-live-requested and total-requested bytes, plus metadata reduction; these are not allocator overhead or RSS. Both variants are warmed first so persistent NTT plans are excluded from per-operation accounting; no cold-memory claim.

Memory instrumentation correction before final runs: fresh subprocess per variant and shape, identical sequential baseline-then-candidate warmup. Track all allocations continuously, including aligned allocations. Report starting/peak/ending live requested bytes and operation total allocated bytes/count. Warm NTT plan replacement may retain or free allocations, so report its observed end-start delta; do not assume zero retention. Initial instrument pilot asserted zero retained bytes and failed; no pilot memory number is used.

## Final source packaging

Baseline commit: 31d238c62d0b297b1ffaf8b32a9b08461c2cadd9 (original PR13 tested tree 0cbd3745, parent 488451af, as reported by root). The seven-header archive matches every previously proposed source hash. benchmark.cpp, shapes.inc, memory.cpp, structural.cpp, summarize.py, and correctness.cpp are copied byte-for-byte from the frozen historical harness. adversarial.cpp changes only its include from an absolute temporary path to the adjacent correctness.cpp. No C++ measurement logic or acceptance gate changes. run.py mechanically reconstructs and hash-checks the SIMD baseline, normalizes all three includes, verifies exact recursive closure, and runs correctness before the single complete timing sequence. All raw data stays outside the repository. The report of the old pre-SIMD experiment is not replaced or relabeled as stacked results.
