# One-shot stacked product-tree validation

Baseline: 31d238c62d0b297b1ffaf8b32a9b08461c2cadd9 (merged PR13). This directory prepares a new incremental comparison; historical pre-SIMD timings are not results for this source.

1. Check out the exact baseline commit with its Git object available. Apply only the supplied tree-only.patch, then add this benchmark directory and test/unit/multipoint_tree.cpp. Do not overlay the old pre-SIMD header.
2. On an otherwise idle Linux x86_64 runner, verify preparation without compilation:

```sh
python3 test/benchmark/multipoint_tree_stacked/run.py --output /tmp/tree-stacked-prepare --prepare-only
```

3. After root authorizes the single remote run, run the complete protocol once:

```sh
python3 test/benchmark/multipoint_tree_stacked/run.py --output /tmp/tree-stacked-results --cpu 0 --cxx g++
```

The runner first checks every frozen source and the exact seven-header recursive closure. It reconstructs the SIMD baseline with function/header-guard renaming and three include-path normalizations. It then runs baseline/Horner correctness and independent adversarial checks in default, forced-scalar, and ASan/UBSan configurations, followed by the affected fps, fps_algorithms, standalone, and multipoint_tree units. Checks and compilation are sequential, never overlapping timing. Both complete 44-shape, 8-pair generic/native measurements follow. Preserve every raw row and failure as workflow artifacts, not committed files. No selective retry or rerun-until-green.

CPU 0 is the declared core. If unavailable, select and record an allowed CPU before starting. GCC identity, CPU information, exact source/dependency hashes, raw CSV, summaries, and correctness logs are recorded outside the repository. Existing CSV output directories are rejected. --checks-only runs correctness without timing; --memory-only runs the separate fresh-process allocation and structural harnesses in a fresh output directory.

All original numerical acceptance gates and adverse-result reporting requirements remain in protocol.md. A passing median gate is not blanket no-regression evidence: independently review ratio-of-medians, absolute costs, controls, and extreme pairs. Required latest-head CI and Apple Clang/macOS tests still gate merge.

The measured benchmark, shapes, memory, structural, summary, and author-correctness C++/Python sources are unchanged. The adversarial test changes only its old absolute include to the adjacent correctness.cpp. run.py is new orchestration and was validated source-only with --prepare-only; no stacked compilation or timings have been performed during preparation.
