# Point-sized multipoint product tree

## Qualified result

The incremental tree-only optimization passes the fixed performance gates against merged SIMD PR13. On the measured Linux runner, the target asymmetric shapes improve by **1.29969× generic** and **1.32810× native** (geometric mean of paired-median CPU speedups). The public API, SIMD helper, runtime guards, and direct-evaluation selection gates are unchanged. Required CI, including Apple Clang/macOS portability, must still pass on the exact final merge head.

- Baseline commit: `31d238c62d0b297b1ffaf8b32a9b08461c2cadd9`.
- Measured validation commit: `ba4bd17c1faab556f62653782b3930a17692cd91`.
- Candidate header SHA-256: `4cb93dd9d9d2b0bbbece729c7de93217a19cd4915cf9ace9aa797c0eefcb049e`.
- Unchanged SIMD helper SHA-256: `73f3c335566cad50271ed5a0c8336ec10c57c6ec2c69a193c463939d15136380`.
- [Remote validation run](https://github.com/qandori/cp-library/actions/runs/39031675354) and [complete evidence artifact](https://github.com/qandori/cp-library/actions/runs/39031675354/artifacts/12100458725).

The validation workflow is temporary infrastructure, not part of this production change. Raw benchmark outputs are preserved in the evidence artifact, not committed here.

## Change and equivalence

Only two production edits are needed: start the product-tree size at 1, letting construction size it from the point count; after the unchanged degree-sized reverse/div_series operation, retain the quotient's last `size * 8` coefficients.

Each omitted upper ancestor had an empty right subtree with polynomial `{1}`, so its root product equals its left child's. The previous evaluator descended these ancestors by retaining the upper half at each step, exactly equivalent to retaining the final suffix once. All lower child frequencies remain unchanged. An omitted survivor-root spectrum was unused because evaluation reads child spectra. Generic modint conversion and the direct route are untouched.

Reference: [official accepted submission 409516](https://judge.yosupo.jp/submission/409516), lines 7631–7646, inspected read-only. It similarly uses a point-sized tree and a quotient suffix, but its division implementation differs. No reference source was copied or executed.

## Exact remote protocol and checks

The single run used an AMD EPYC 7763 virtual machine, GCC 13.3.0, Linux x86_64, CPU 0. Builds were `-std=c++20 -O2` and the same flags plus `-march=native`. The compiler wrapper recorded commands and binary hashes without changing compilation flags.

The frozen grid contains 44 shapes and 8 balanced alternating pairs per shape, with shape traversal reversed every other pair. Each variant receives at least 0.15 actual process CPU seconds per batch. Baseline and candidate execute in the same process, with full output equality before timing and full-result checksums during timing. All 704 paired rows were retained; no shape, adverse observation, or extreme pair was removed. Exact source hashes and the actual seven-header recursive dependency closure were verified.

The predeclared target set is `n >= 1024` and `33 <= m <= 129` (23 shapes). Both builds required target geometric mean at least 1.10, at least 75% of target shape medians at least 1.05, and no shape with paired-median speedup below `1 / 1.05`. Both builds pass, with all 23 target shapes at least 1.05. Target paired-median ranges are 1.18177–1.37340 generic and 1.19562–1.39340 native.

Remote correctness passed:

- 399 shape combinations for each of internal Mint and PlainMint, compared with the exact baseline and independent ordinary modular Horner.
- 118 additional adversarial cases for each type: sparse, leading-zero, monomial, repeated points, roots of unity, and point-product vanishing polynomials.
- Both suites in default, forced-scalar, and ASan/UBSan configurations (`detect_leaks=0`).
- Existing fps, fps_algorithms, and standalone units, plus the retained multipoint_tree unit.

These local Linux checks are not a claim that latest-head CI or macOS testing has already completed.

## Controls and adverse observations

The formerly problematic direct control `32 × 4096` is near parity in this stacked run:

| Build | Paired CPU speedup | CPU ratio of medians | Baseline median us | Candidate median us |
| --- | ---: | ---: | ---: | ---: |
| Generic | 0.99959 | 0.99920 | 67.6230 | 67.6773 |
| Native | 1.00123 | 1.00160 | 67.7143 | 67.6063 |

This does not mean every pair improved. The worst per-shape paired medians are 0.99341 (generic, `129 × 32768`) and 0.99067 (native, `131072 × 131072`). The lowest individual pair is 0.90590 for generic `4097 × 4097`, pair 0, baseline-first; native reaches 0.89812 for `32 × 4096`, pair 4, candidate-first. These observations remain in the evidence. Every per-shape result, including subparity rows and ratio-of-medians discrepancies, appears below.

The older pre-SIMD experiment's direct control had paired median 0.98235, ratio of medians 0.85098, and an individual 0.108× pair. Those results are not erased or relabeled: [historical-pre-simd.md](historical-pre-simd.md) preserves that separate machine/compiler/baseline experiment and all its rows. The present qualification relies on the complete stacked remote comparison, not on transferring the earlier 1.26–1.27× result.

## Memory scope

No fresh allocation or RSS measurement was run in the stacked remote validation. The included memory and structural harnesses are reproducible tools, not proof of an unperformed remote measurement.

Historical pre-SIMD allocation accounting, reported separately in [historical-pre-simd.md](historical-pre-simd.md), measured peak live requested bytes of 6,474,840 → 4,768,564 at 131072 coefficients and 129 points. Those figures belong to the earlier GCC 14.2/Xeon run and must not be presented as measurements of this stacked remote source.

The structural caveat is unchanged: the quotient's size shrinks, but its capacity remains degree-sized; root division and transform storage remain degree-sized. Savings concern tree/frequency metadata, empty-node payloads, and omitted ancestor work. Requested allocation bytes are not RSS or allocator overhead.

## Protocol erratum

The measured `protocol.md` contained a transcription error in its proposed candidate hash. The published prose corrects it to `4cb93dd9d9d2b0bbbece729c7de93217a19cd4915cf9ace9aa797c0eefcb049e`, which was already enforced by the measured runner and manifest. Its source-preparation status is a pre-run record; this report records the subsequent remote result.

For publication, descriptive manifest status and protocol role wording were normalized, and only their expected document hashes in `run.py` were updated. Sampling, inputs, benchmark C++, production code, and validation logic are unchanged. The exact measured originals remain in the linked evidence artifact.

- run.py: measured `5f3348a51f7a430a611aab236716068d6445c62a6c2d0bee7054ae4e463f43d9`; published `132b551740533d7f2a0453bd59b6db2040678e5d73c50f5865cab27b09aefbd5`.
- protocol.md: measured `f6dc82fb11eb26393659d73c7ad618d64d6f8dc8e032cadaa5bc1cad8620d9ac`; published `7bddb1a273ec4fb981c3ce08de052032a91b785ebe2f3f25facca3e35fd33ae6`.
- source-manifest.json: measured `55d0a48ac66392911d3dc859b5644147c4041340c684eeca559886e844fffc06`; published `f255757e4cd91cf8173ed929f0764f992e6fae86d1e3934341d4541251a43f74`.

## Complete stacked per-shape results

CPU and wall times are median microseconds per call. Paired ratios are medians of baseline/candidate pairs. CPU ROM is the ratio of independent CPU medians. All 44 shapes are included for each build.

### Generic

| n | m | Paired CPU | CPU ROM | Baseline CPU us | Candidate CPU us | Paired wall | Baseline wall us | Candidate wall us |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 32 | 4096 | 0.99959 | 0.99920 | 67.6230 | 67.6773 | 0.99944 | 67.6265 | 67.6837 |
| 33 | 1024 | 0.99756 | 0.99842 | 186.1011 | 186.3956 | 0.99772 | 186.1171 | 186.4229 |
| 129 | 32768 | 0.99341 | 0.99502 | 8729.3199 | 8772.9889 | 0.99340 | 8731.0961 | 8773.0447 |
| 1024 | 33 | 1.28422 | 1.28463 | 51.2529 | 39.8970 | 1.28416 | 51.2577 | 39.9020 |
| 1024 | 63 | 1.32570 | 1.32554 | 46.8571 | 35.3494 | 1.32569 | 46.8586 | 35.3510 |
| 1024 | 65 | 1.28612 | 1.28413 | 48.8521 | 38.0429 | 1.28615 | 48.8538 | 38.0448 |
| 1024 | 128 | 1.25968 | 1.25928 | 53.7854 | 42.7112 | 1.25976 | 53.7923 | 42.7132 |
| 1024 | 129 | 1.18177 | 1.18275 | 57.9405 | 48.9880 | 1.18176 | 57.9475 | 48.9911 |
| 1024 | 512 | 1.06376 | 1.06307 | 106.3685 | 100.0575 | 1.06378 | 106.3815 | 100.0580 |
| 1024 | 1024 | 0.99890 | 0.99897 | 185.6619 | 185.8542 | 0.99878 | 185.6860 | 185.8655 |
| 1025 | 33 | 1.31095 | 1.31087 | 97.9595 | 74.7286 | 1.31084 | 97.9656 | 74.7395 |
| 1025 | 129 | 1.26363 | 1.26354 | 100.4401 | 79.4911 | 1.26376 | 100.4518 | 79.4915 |
| 1025 | 1025 | 0.99679 | 0.99695 | 261.7913 | 262.5927 | 0.99670 | 261.8419 | 262.5938 |
| 4096 | 32 | 1.00024 | 1.00000 | 51.9632 | 51.9630 | 1.00020 | 51.9676 | 51.9730 |
| 4096 | 33 | 1.29310 | 1.29272 | 187.2006 | 144.8111 | 1.29303 | 187.2156 | 144.8224 |
| 4096 | 63 | 1.37340 | 1.37450 | 162.8720 | 118.4959 | 1.37334 | 162.8780 | 118.4990 |
| 4096 | 65 | 1.35902 | 1.36067 | 164.7006 | 121.0441 | 1.35895 | 164.7117 | 121.0486 |
| 4096 | 129 | 1.31436 | 1.31359 | 174.4108 | 132.7738 | 1.31425 | 174.4783 | 132.7748 |
| 4096 | 512 | 1.21437 | 1.21437 | 223.7418 | 184.2456 | 1.21445 | 223.7587 | 184.2844 |
| 4096 | 2048 | 1.05110 | 1.05093 | 480.3705 | 457.0890 | 1.05128 | 480.4110 | 457.1251 |
| 4096 | 4096 | 0.99896 | 0.99821 | 861.9558 | 863.5036 | 0.99907 | 862.2450 | 863.5075 |
| 4097 | 33 | 1.29562 | 1.29688 | 388.9847 | 299.9392 | 1.29556 | 388.9868 | 299.9719 |
| 4097 | 129 | 1.32946 | 1.32794 | 334.6210 | 251.9858 | 1.32960 | 334.6563 | 252.0001 |
| 4097 | 1025 | 1.14100 | 1.14237 | 499.5652 | 437.3042 | 1.14098 | 499.6056 | 437.3258 |
| 4097 | 4097 | 0.99701 | 0.99616 | 1164.5860 | 1169.0789 | 0.99694 | 1164.6858 | 1169.2148 |
| 32768 | 33 | 1.25996 | 1.25241 | 1558.6893 | 1244.5471 | 1.25991 | 1558.9247 | 1244.6486 |
| 32768 | 63 | 1.33584 | 1.33214 | 1308.3335 | 982.1277 | 1.33583 | 1308.3426 | 982.1348 |
| 32768 | 65 | 1.35688 | 1.35154 | 1322.3668 | 978.4143 | 1.35769 | 1322.4745 | 978.5061 |
| 32768 | 129 | 1.33532 | 1.32871 | 1324.8619 | 997.1011 | 1.33521 | 1325.1804 | 997.1666 |
| 32768 | 4096 | 1.19765 | 1.19789 | 2067.8497 | 1726.2503 | 1.19756 | 2068.1046 | 1726.3329 |
| 32768 | 16384 | 1.03696 | 1.03175 | 4700.1259 | 4555.4875 | 1.03688 | 4700.3963 | 4556.0250 |
| 32768 | 16385 | 0.99942 | 0.99999 | 5304.7905 | 5304.8506 | 0.99933 | 5305.0701 | 5305.0266 |
| 32768 | 32768 | 0.99738 | 0.99826 | 8704.5464 | 8719.7351 | 0.99738 | 8706.0230 | 8719.7521 |
| 32769 | 33 | 1.22372 | 1.22404 | 3199.9191 | 2614.2220 | 1.22370 | 3200.1749 | 2614.3795 |
| 32769 | 129 | 1.31348 | 1.30798 | 2750.4478 | 2102.8198 | 1.31359 | 2750.7130 | 2102.8360 |
| 32769 | 1025 | 1.26224 | 1.26802 | 2922.6544 | 2304.9025 | 1.26228 | 2923.0618 | 2304.9547 |
| 32769 | 32769 | 1.00189 | 1.00368 | 11560.3859 | 11518.0078 | 1.00203 | 11560.9375 | 11518.4281 |
| 131072 | 33 | 1.24375 | 1.23717 | 6773.9529 | 5475.3653 | 1.24363 | 6773.9852 | 5475.5311 |
| 131072 | 63 | 1.32707 | 1.32314 | 5538.2095 | 4185.6643 | 1.32717 | 5538.6318 | 4185.8309 |
| 131072 | 65 | 1.31516 | 1.31418 | 5555.5845 | 4227.4116 | 1.31511 | 5556.6934 | 4227.9021 |
| 131072 | 129 | 1.32286 | 1.32335 | 5554.7112 | 4197.4611 | 1.32294 | 5555.3534 | 4197.7147 |
| 131072 | 16384 | 1.23191 | 1.24010 | 9733.9262 | 7849.3243 | 1.23192 | 9734.4987 | 7850.3217 |
| 131072 | 65536 | 1.04006 | 1.04292 | 22244.5508 | 21329.1913 | 1.04015 | 22247.4313 | 21339.5497 |
| 131072 | 131072 | 1.00214 | 1.00236 | 43259.5379 | 43157.6653 | 1.00188 | 43333.7870 | 43192.3865 |

### Native

| n | m | Paired CPU | CPU ROM | Baseline CPU us | Candidate CPU us | Paired wall | Baseline wall us | Candidate wall us |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 32 | 4096 | 1.00123 | 1.00160 | 67.7143 | 67.6063 | 1.00123 | 67.7214 | 67.6067 |
| 33 | 1024 | 0.99958 | 1.00000 | 190.5276 | 190.5277 | 0.99974 | 190.5412 | 190.5384 |
| 129 | 32768 | 0.99686 | 1.00335 | 9299.7936 | 9268.7610 | 0.99674 | 9299.9099 | 9269.6776 |
| 1024 | 33 | 1.32868 | 1.32789 | 51.9205 | 39.0999 | 1.32884 | 51.9314 | 39.1043 |
| 1024 | 63 | 1.34281 | 1.35452 | 47.3858 | 34.9834 | 1.34284 | 47.3957 | 34.9888 |
| 1024 | 65 | 1.32656 | 1.33283 | 49.5910 | 37.2074 | 1.32668 | 49.5977 | 37.2192 |
| 1024 | 128 | 1.26969 | 1.25998 | 54.1499 | 42.9767 | 1.26968 | 54.1539 | 42.9804 |
| 1024 | 129 | 1.19562 | 1.18856 | 58.2192 | 48.9829 | 1.19318 | 58.2241 | 49.0140 |
| 1024 | 512 | 1.06419 | 1.06494 | 108.2969 | 101.6926 | 1.06438 | 108.3388 | 101.7133 |
| 1024 | 1024 | 1.00061 | 0.99791 | 189.5555 | 189.9524 | 1.00049 | 189.5648 | 190.0148 |
| 1025 | 33 | 1.34788 | 1.35661 | 99.5056 | 73.3489 | 1.34762 | 99.5129 | 73.3493 |
| 1025 | 129 | 1.26848 | 1.26570 | 98.4474 | 77.7810 | 1.26839 | 98.4637 | 77.7905 |
| 1025 | 1025 | 0.99932 | 0.99997 | 265.7507 | 265.7589 | 0.99929 | 265.8844 | 265.8247 |
| 4096 | 32 | 0.99909 | 0.99920 | 51.8707 | 51.9121 | 0.99902 | 51.8746 | 51.9195 |
| 4096 | 33 | 1.35478 | 1.35477 | 195.5169 | 144.3176 | 1.35471 | 195.5177 | 144.3265 |
| 4096 | 63 | 1.38228 | 1.38655 | 162.6044 | 117.2725 | 1.38218 | 162.6299 | 117.2874 |
| 4096 | 65 | 1.39340 | 1.39497 | 164.9952 | 118.2786 | 1.39323 | 165.0045 | 118.2983 |
| 4096 | 129 | 1.34270 | 1.34557 | 175.1030 | 130.1325 | 1.34253 | 175.1129 | 130.1438 |
| 4096 | 512 | 1.22054 | 1.22181 | 224.4129 | 183.6721 | 1.22045 | 224.4314 | 183.6870 |
| 4096 | 2048 | 1.05081 | 1.05029 | 489.6243 | 466.1813 | 1.05039 | 489.6266 | 466.1839 |
| 4096 | 4096 | 0.99770 | 0.99767 | 883.3417 | 885.4076 | 0.99767 | 883.4180 | 885.5633 |
| 4097 | 33 | 1.32423 | 1.31045 | 386.8504 | 295.2047 | 1.32394 | 386.8526 | 295.2163 |
| 4097 | 129 | 1.36226 | 1.36365 | 334.4381 | 245.2525 | 1.36227 | 334.4532 | 245.2655 |
| 4097 | 1025 | 1.14807 | 1.14864 | 501.4532 | 436.5629 | 1.14753 | 501.5000 | 436.6119 |
| 4097 | 4097 | 1.00087 | 1.00112 | 1190.0367 | 1188.7087 | 1.00106 | 1190.2489 | 1188.8828 |
| 32768 | 33 | 1.34935 | 1.32780 | 1609.3835 | 1212.0652 | 1.34925 | 1610.2610 | 1212.3259 |
| 32768 | 63 | 1.34100 | 1.35576 | 1311.9730 | 967.7037 | 1.34090 | 1311.9818 | 967.8847 |
| 32768 | 65 | 1.34662 | 1.34800 | 1303.7244 | 967.1562 | 1.34676 | 1303.7333 | 967.2067 |
| 32768 | 129 | 1.34341 | 1.34379 | 1316.0004 | 979.3228 | 1.34327 | 1316.0091 | 979.3812 |
| 32768 | 4096 | 1.19137 | 1.18714 | 2049.5962 | 1726.5013 | 1.19129 | 2049.7001 | 1726.8086 |
| 32768 | 16384 | 1.04004 | 1.04140 | 4880.7509 | 4686.7291 | 1.04005 | 4881.3103 | 4687.8950 |
| 32768 | 16385 | 1.00204 | 1.00850 | 5576.5040 | 5529.5183 | 1.00187 | 5576.7409 | 5529.5797 |
| 32768 | 32768 | 0.99940 | 0.99388 | 9207.5350 | 9264.1869 | 0.99922 | 9207.5865 | 9264.2717 |
| 32769 | 33 | 1.29817 | 1.29731 | 3287.1516 | 2533.8280 | 1.29797 | 3287.5545 | 2533.9851 |
| 32769 | 129 | 1.34448 | 1.38019 | 2854.0938 | 2067.9064 | 1.34431 | 2854.2133 | 2068.3145 |
| 32769 | 1025 | 1.28820 | 1.29329 | 2946.6502 | 2278.4094 | 1.28798 | 2946.6785 | 2278.4263 |
| 32769 | 32769 | 1.00076 | 1.00234 | 12162.6543 | 12134.3087 | 1.00092 | 12164.1587 | 12135.4028 |
| 131072 | 33 | 1.27847 | 1.29754 | 6815.6112 | 5252.7024 | 1.27877 | 6818.6010 | 5254.0715 |
| 131072 | 63 | 1.33730 | 1.34263 | 5571.8141 | 4149.9193 | 1.33701 | 5572.3339 | 4150.3608 |
| 131072 | 65 | 1.34486 | 1.34032 | 5569.2142 | 4155.1304 | 1.34503 | 5569.8665 | 4155.1527 |
| 131072 | 129 | 1.33870 | 1.33798 | 5556.5355 | 4152.9279 | 1.33820 | 5556.8697 | 4153.9722 |
| 131072 | 16384 | 1.17218 | 1.17591 | 9386.6248 | 7982.4260 | 1.17218 | 9388.8052 | 7982.9890 |
| 131072 | 65536 | 1.03187 | 1.02983 | 23015.8998 | 22349.1253 | 1.03192 | 23019.1588 | 22351.4513 |
| 131072 | 131072 | 0.99067 | 0.99358 | 44826.6704 | 45116.1920 | 0.99140 | 44876.5959 | 45118.8982 |

## Reproduction

Use a Linux x86_64 checkout containing the narrow candidate patch, the retained unit test, and this directory. The baseline commit must be available in its Git object database. From the repository root:

```sh
python3 test/benchmark/multipoint_tree_stacked/run.py --output /tmp/tree-stacked-prepare --prepare-only
python3 test/benchmark/multipoint_tree_stacked/run.py --output /tmp/tree-stacked-results --cpu 0 --cxx g++
```

The first command performs source-only verification. The second runs the complete correctness and timing protocol sequentially. Use an otherwise idle runner, a fresh output directory, and a recorded CPU affinity; if CPU 0 is unavailable, explicitly record the chosen permitted core. Do not run competing compilation or benchmarks. The runner rejects prior CSV outputs rather than overwriting observations. No selective retry or rerun-until-green.

`--checks-only` runs correctness without timing. `--memory-only` in a separate fresh directory runs allocation and structural accounting, with each allocation variant in its own process after identical baseline-then-candidate warmup. Neither command was substituted for the complete remote timing run.

The baseline is reconstructed from the pinned Git commit by renaming its function/header guard and normalizing three quoted includes. Its resulting hash and every real recursive dependency are checked. Generated headers use `-iquote`, binaries have `.bin` names, and raw outputs remain outside the repository. Unrelated repository headers are not used as the dependency comparison set. The adversarial harness differs from its original test only by replacing an absolute include with adjacent `correctness.cpp`.

No temporary workflow, raw logs, or historical pre-SIMD production header is part of the publication packet. Apply only the two tree-specific production hunks to preserve existing SIMD guards and unrelated main-branch changes.

## Evidence hashes

- Remote generic CSV SHA-256: `dc9e5f41821b882ffcc32935221605d0f7c807fdf5bcc5e3c154d1908401846f`.
- Remote native CSV SHA-256: `f394f0deb02189443f6c42e4ecbab536d87c2fb7af42fb0d9bc7896012685e82`.
- Generic binary SHA-256: `721ea73d23389915bb4e9a0b960772e521742cab5a4ca5d3dc54a2aaf7c81878`.
- Native binary SHA-256: `8a6ce12007c4a8d32b1d31cd8f6b806b7d8df62d5e757b26fc4ba4100457c535`.
- Frozen runner SHA-256: `5f3348a51f7a430a611aab236716068d6445c62a6c2d0bee7054ae4e463f43d9`.
- Frozen protocol SHA-256: `f6dc82fb11eb26393659d73c7ad618d64d6f8dc8e032cadaa5bc1cad8620d9ac`.
