# Multipoint evaluation: direct Horner batching

**Qualified performance acceptance; latest-head correctness CI remains required before merge.** The original frozen SIMD patch has substantial target-workload gains in both local and independent remote validation. Acceptance explicitly includes hardware/compiler-dependent scalar fallback tradeoffs, not universal non-regression. All original local adverse results and the failed gate-factorization diagnostic remain below.

## Independent remote validation and decision

[GitHub Actions run 39029747234](https://github.com/qandori/cp-library/actions/runs/39029747234) ran the unchanged full protocol once on an Ubuntu 24.04 AMD EPYC 7763 VM with GCC 13.3.0. Both 78-shape configurations retained all eight balanced-order pairs per shape (1,248 pairs total), with every variant meeting the 0.15 process CPU second floor. Original candidate, harness and relevant dependencies matched the frozen sources; no alternative gate was substituted.

All 36 SIMD-active shapes improved: median paired wall speedups were 1.622–10.398x for portable O2 and 1.752–10.400x for native O2. Important fallback comparisons, using paired CPU speedups (baseline / candidate), are:

| Mode / shape | Original Intel / GCC 14.2 | Remote AMD / GCC 13.3 |
|---|---:|---:|
| Portable 7×8 | 0.946225 | 1.004180 |
| Portable 7×31 | 0.958497 | 1.001437 |
| Portable 8×31 | 0.968598 | 1.001464 |
| Portable 31×7 | 0.962531 | 1.000376 |
| Native 4×33 | 0.872726 | 1.004264 |
| Native 16×16 | 0.871821 | 1.001156 |
| Native 7×16 | 0.967818 | 0.982303 |

Remote controls were more stable, but native 7×16 still slowed by about 1.8%, and some other controls and individual pairs remained slower. This single run changes both hardware and compiler, so it neither isolates the cause of the earlier losses nor proves them to be noise. It is not an AtCoder measurement or a noiseless dedicated-host guarantee. Target gains are accepted with these explicit tradeoffs; this is not a retroactive claim that every workload improves.

The separate gate-factorization screen did not establish a reliable repair and is preserved as negative evidence at the end of this report. The original production code is unchanged.

## Scope and reference

Baseline: `117fda44639532534bc91fb02517a1cba09ee738`.

The public API, generic modint conversion, and existing `min(n,m) <= 32` direct-vs-product-tree selection are unchanged. On runtime AVX2-capable x86, selected direct calls process 32 points as four independent eight-lane Montgomery Horner chains. The last fewer than eight points use scalar Horner; other incomplete tiles use zero-padded scratch space. Other CPUs retain the old scalar path. There is no claim of measured Apple Silicon speedup.

The reference was the official AC submission [409516](https://judge.yosupo.jp/submission/409516), inspected at source lines7545–7567 (four eight-lane Horner chains). Its product-work threshold was not adopted. The new code reuses this library's existing `NTT::mul8/add8` arithmetic. Downloaded reference code was not executed.

Within the old direct route, the conservative dispatch requires either `min(n,m)>=8 && max(n,m)>=32`, or `n>=4 && m>=4096`. These gates are bounded choices informed by the exploratory grid, not a claim of optimal crossover on all hardware.

The independent point-sized-product-tree idea is not part of this patch. Its preliminary asymmetric gains were too noisy for acceptance, and no combined speedup is claimed.

## Reproduction

Run from repository root:

```
python3 test/benchmark/multipoint_eval.py --output /tmp/multipoint-final --cpu 0 --pairs 8 --seconds 0.15
python3 test/benchmark/multipoint_eval.py --output /tmp/multipoint-native --cpu 0 --pairs 8 --seconds 0.15 --native
```

The script obtains the exact old function from the baseline commit, renames only its function/header guard and adjusts include locations, then compiles it together with the candidate against the same unchanged dependencies. Compiler flags are `g++ -std=c++2b -O2 -DNDEBUG -Wall -Wextra`; the portable configuration has no global AVX2 requirement, and the second configuration adds `-march=native` for both baseline and candidate. Output directory stores environment/source hashes and every raw observation, outside the repository.

Each configuration has 78 fixed shapes, eight alternating baseline/candidate pairs per shape, and at least 0.15 process CPU seconds actually executed per variant in each pair. Fixed seed731. Allocation, evaluation, full-output checksum and destruction are timed. Each shape first compares complete baseline/candidate results. The checksum is volatile-consumed. Process CPU time is checked after batches of 16 calls; raw rows retain actual CPU seconds and iteration counts for both variants. Reported latency is wall time divided by calls. Four pairs use each execution order. Cases and pair order are predetermined, and all adverse cells are retained.

Speedup is baseline time / candidate time. The median of the eight paired ratios is reported separately from the ratio of median times; these statistics are not interchangeable. The benchmark supports `--grid` for the56-shape forced-helper exploratory grid; this option uses the current helper, not an archived earlier candidate.

## Exploratory evidence, not final validation

An initial 19-cell pilot with the original all-padded helper exposed the8×8 regression (0.559x) while32×4096,4096×32 and131072×32 showed7.38x,11.50x and13.43x paired speedups. The8×8 gate was therefore excluded.

A predeclared56-cell forced-helper grid then used three alternating pairs, each variant at least 0.15s, CPU 0. The entire grid below is preserved, including losses and obvious runtime noise. Original helper SHA256: `eeec013177e8c27d213790f2b6efdb503d979c4487e8559ac18dca8b31629e79`.

| n / m | 7 | 8 | 16 | 31 | 32 | 33 | 64 | 4096 |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 2 | 0.246 | 0.255 | 0.350 | 0.498 | 0.555 | 0.519 | 0.735 | 0.938 |
| 4 | 0.321 | 0.354 | 0.428 | 0.488 | 0.918 | 3.305 | 1.065 | 1.474 |
| 7 | 0.434 | 0.448 | 0.720 | 1.178 | 1.283 | 0.328 | 0.585 | 2.024 |
| 8 | 0.459 | 0.502 | 0.822 | 1.334 | 1.575 | 0.899 | 1.587 | 2.260 |
| 16 | 0.264 | 0.756 | 1.585 | 1.499 | 2.736 | 0.533 | 3.064 | 3.250 |
| 31 | 0.672 | 2.259 | 1.952 | 6.241 | 4.377 | 2.761 | 5.247 | 5.455 |
| 32 | 1.134 | 1.381 | 1.356 | 6.617 | 3.927 | 2.440 | 5.760 | 5.849 |

At7×33, candidate times ranged0.78–9.09us between pairs, so isolated noisy grid wins were not used as threshold evidence. The8×33 loss also exposed waste from evaluating a singleton tail with32 SIMD lanes. One structural change replaced tails shorter than eight with scalar Horner, then source and final harness were frozen. No repeated exploratory run was selected to hide the original loss.

## Memory and portability

Asymptotic time and output storage are unchanged. The helper adds two fixed32-word stack arrays (256bytes total) plus registers; it does not cache or retain heap memory. Output allocation remains one vector. The new header is self-contained and does not introduce public aliases or template macros. AVX2 target attributes are confined to an x86 guard; entry is runtime-gated. Mac arm64 continues through scalar Horner. Real macOS CI is still required before merge.

## Correctness validation

An independent reviewer checked 6,456 cases against ordinary uint64 modular Horner, not the same Montgomery operations, including empty inputs, tails0–65, duplicates, zero points and raw Montgomery representatives0,1,p−1,p,p+1,2p−2,2p−1. Normal dispatch, a temporary copy with `NTT::vectorized()` forced false, and ASan+UBSan all passed. An additional 36 long-point boundary shapes passed that independent oracle.

The affected GCC units `fps.cpp`, expanded `fps_algorithms.cpp` (internal and independent ordinary modints), and `standalone.cpp` passed. Both changed/new headers passed standalone checks. No local Clang or native macOS execution is claimed; the repository's Linux Clang/sanitizer and macOS jobs must pass on the exact final PR revision before merge.

## Frozen final results

Measured on 2026-10-17, Intel Xeon Platinum 8573C, GCC 14.2.0, Linux x86-64 container, CPU 0. The container does not expose the topology file needed by `lscpu`; the runner retains that diagnostic and `/proc/cpuinfo` instead. Each configuration contains 78 shapes and 624 paired observations; CPU-floor and order checks are applied to every row.

### Portable O2

All 36 SIMD-activated cells improved, with median paired wall speedups1.416–10.337x. The 42 unchanged-path controls include a small7×8 regression: median wall latency0.228us to0.245us, paired wall speedup0.938x and CPU speedup0.946x. Other control losses above 3% in paired CPU time are7×31 (0.9585x; wall medians0.713us to0.725us), 31×7 (0.9625x; wall medians1.139us to1.161us), and8×31 (0.968598x). There are four portable CPU controls below0.97x including7×8. All are retained rather than described as noise-free equivalence. No cell had greater than 5% disagreement between paired wall and CPU-per-call speedups. The exact public-path branch and timing overhead remain included.

| n | m | baseline us | candidate us | median paired wall speedup | ratio of wall medians | median paired CPU speedup | path |
|---:|---:|---:|---:|---:|---:|---:|:---|
| 2 | 7 | 0.075 | 0.083 | 0.982 | 0.909 | 0.982 | unchanged |
| 2 | 8 | 0.080 | 0.077 | 1.054 | 1.037 | 1.055 | unchanged |
| 2 | 16 | 0.133 | 0.121 | 1.072 | 1.102 | 1.066 | unchanged |
| 2 | 31 | 0.207 | 0.211 | 1.017 | 0.981 | 1.017 | unchanged |
| 2 | 32 | 0.221 | 0.210 | 1.054 | 1.053 | 1.038 | unchanged |
| 2 | 33 | 0.215 | 0.228 | 0.969 | 0.942 | 0.972 | unchanged |
| 2 | 64 | 0.386 | 0.383 | 1.001 | 1.008 | 1.001 | unchanged |
| 2 | 4096 | 24.030 | 23.537 | 0.996 | 1.021 | 0.996 | unchanged |
| 4 | 7 | 0.103 | 0.103 | 0.990 | 1.004 | 0.990 | unchanged |
| 4 | 8 | 0.131 | 0.121 | 1.062 | 1.084 | 1.045 | unchanged |
| 4 | 16 | 0.223 | 0.217 | 1.051 | 1.028 | 1.050 | unchanged |
| 4 | 31 | 0.371 | 0.364 | 0.990 | 1.021 | 0.990 | unchanged |
| 4 | 32 | 0.412 | 0.390 | 1.002 | 1.057 | 1.002 | unchanged |
| 4 | 33 | 0.477 | 0.447 | 1.039 | 1.068 | 1.014 | unchanged |
| 4 | 64 | 0.935 | 0.932 | 1.023 | 1.002 | 1.020 | unchanged |
| 4 | 4096 | 54.762 | 36.880 | 1.523 | 1.485 | 1.520 | SIMD |
| 7 | 7 | 0.212 | 0.213 | 1.013 | 0.995 | 1.016 | unchanged |
| 7 | 8 | 0.228 | 0.245 | 0.938 | 0.929 | 0.946 | unchanged |
| 7 | 16 | 0.385 | 0.395 | 0.980 | 0.974 | 1.009 | unchanged |
| 7 | 31 | 0.713 | 0.725 | 0.958 | 0.983 | 0.958 | unchanged |
| 7 | 32 | 0.781 | 0.776 | 1.024 | 1.006 | 1.023 | unchanged |
| 7 | 33 | 0.765 | 0.756 | 1.002 | 1.012 | 1.011 | unchanged |
| 7 | 64 | 1.561 | 1.380 | 1.017 | 1.131 | 1.017 | unchanged |
| 7 | 4096 | 95.825 | 45.398 | 1.983 | 2.111 | 1.984 | SIMD |
| 8 | 7 | 0.263 | 0.277 | 0.976 | 0.950 | 0.976 | unchanged |
| 8 | 8 | 0.236 | 0.236 | 1.023 | 1.001 | 1.023 | unchanged |
| 8 | 16 | 0.511 | 0.419 | 1.213 | 1.220 | 1.213 | unchanged |
| 8 | 31 | 0.749 | 0.764 | 0.969 | 0.981 | 0.969 | unchanged |
| 8 | 32 | 0.744 | 0.519 | 1.416 | 1.433 | 1.416 | SIMD |
| 8 | 33 | 0.763 | 0.401 | 1.977 | 1.904 | 1.977 | SIMD |
| 8 | 64 | 1.525 | 0.813 | 1.868 | 1.874 | 1.863 | SIMD |
| 8 | 4096 | 92.596 | 38.105 | 2.345 | 2.430 | 2.344 | SIMD |
| 16 | 7 | 0.446 | 0.461 | 0.985 | 0.966 | 0.985 | unchanged |
| 16 | 8 | 0.506 | 0.512 | 0.995 | 0.989 | 0.995 | unchanged |
| 16 | 16 | 0.967 | 1.014 | 0.988 | 0.954 | 0.987 | unchanged |
| 16 | 31 | 1.877 | 1.854 | 1.008 | 1.012 | 1.008 | unchanged |
| 16 | 32 | 2.118 | 0.713 | 2.843 | 2.972 | 2.843 | SIMD |
| 16 | 33 | 2.193 | 0.656 | 3.340 | 3.346 | 3.331 | SIMD |
| 16 | 64 | 3.863 | 1.174 | 3.255 | 3.291 | 3.252 | SIMD |
| 16 | 4096 | 231.739 | 60.719 | 3.884 | 3.817 | 3.883 | SIMD |
| 31 | 7 | 1.139 | 1.161 | 0.967 | 0.981 | 0.963 | unchanged |
| 31 | 8 | 1.317 | 1.347 | 0.970 | 0.978 | 0.984 | unchanged |
| 31 | 16 | 2.568 | 2.589 | 0.993 | 0.992 | 0.993 | unchanged |
| 31 | 31 | 4.841 | 4.928 | 0.963 | 0.982 | 0.983 | unchanged |
| 31 | 32 | 5.071 | 1.045 | 4.764 | 4.853 | 4.765 | SIMD |
| 31 | 33 | 5.515 | 1.077 | 5.118 | 5.121 | 5.124 | SIMD |
| 31 | 64 | 10.725 | 1.895 | 5.910 | 5.658 | 5.910 | SIMD |
| 31 | 4096 | 690.382 | 113.722 | 6.396 | 6.071 | 6.277 | SIMD |
| 32 | 7 | 1.217 | 1.207 | 0.997 | 1.008 | 1.012 | unchanged |
| 32 | 8 | 1.431 | 0.986 | 1.463 | 1.452 | 1.463 | SIMD |
| 32 | 16 | 3.373 | 1.175 | 2.987 | 2.871 | 2.986 | SIMD |
| 32 | 31 | 5.633 | 1.106 | 5.114 | 5.095 | 5.184 | SIMD |
| 32 | 32 | 5.450 | 1.109 | 4.883 | 4.913 | 4.929 | SIMD |
| 32 | 33 | 5.820 | 1.098 | 5.344 | 5.299 | 5.191 | SIMD |
| 32 | 64 | 11.040 | 1.928 | 5.677 | 5.726 | 5.676 | SIMD |
| 32 | 4096 | 692.798 | 111.986 | 6.111 | 6.186 | 6.234 | SIMD |
| 4 | 4095 | 43.540 | 42.956 | 1.022 | 1.014 | 1.016 | unchanged |
| 4 | 4097 | 43.698 | 29.327 | 1.465 | 1.490 | 1.465 | SIMD |
| 7 | 4095 | 82.680 | 84.401 | 0.999 | 0.980 | 0.999 | unchanged |
| 7 | 4097 | 84.323 | 39.640 | 2.041 | 2.127 | 2.040 | SIMD |
| 8 | 63 | 1.693 | 0.931 | 1.871 | 1.819 | 1.871 | SIMD |
| 8 | 65 | 1.614 | 0.732 | 2.077 | 2.206 | 2.078 | SIMD |
| 32 | 9 | 1.606 | 1.089 | 1.515 | 1.475 | 1.549 | SIMD |
| 32 | 15 | 3.554 | 1.207 | 2.633 | 2.945 | 2.633 | SIMD |
| 32 | 63 | 11.078 | 2.055 | 5.326 | 5.391 | 5.326 | SIMD |
| 32 | 65 | 11.312 | 1.896 | 5.689 | 5.965 | 5.689 | SIMD |
| 32 | 131072 | 21867.595 | 3302.846 | 6.594 | 6.621 | 6.594 | SIMD |
| 33 | 8 | 1.501 | 1.042 | 1.443 | 1.440 | 1.463 | SIMD |
| 256 | 8 | 15.426 | 5.788 | 2.701 | 2.665 | 2.637 | SIMD |
| 4096 | 7 | 224.208 | 228.682 | 1.018 | 0.980 | 1.018 | unchanged |
| 4096 | 8 | 263.151 | 91.466 | 2.759 | 2.877 | 2.711 | SIMD |
| 4096 | 31 | 878.642 | 85.038 | 10.237 | 10.332 | 10.455 | SIMD |
| 4096 | 32 | 966.535 | 106.453 | 9.632 | 9.079 | 9.632 | SIMD |
| 131072 | 32 | 31379.087 | 3079.960 | 10.337 | 10.188 | 10.247 | SIMD |
| 33 | 33 | 11.322 | 10.542 | 1.062 | 1.074 | 1.048 | unchanged |
| 1024 | 33 | 80.367 | 77.219 | 1.099 | 1.041 | 1.071 | unchanged |
| 32768 | 129 | 2097.119 | 2004.583 | 1.045 | 1.046 | 1.045 | unchanged |
| 32768 | 32768 | 12970.395 | 13256.122 | 0.989 | 0.978 | 0.988 | unchanged |

### Native O2

All 36 SIMD-activated cells improved, with median paired wall speedups1.213–11.587x. This is not a no-regression result across all inputs. The unchanged4×33 and16×16 controls have paired wall/CPU speedups about0.873x and0.872x. Their median wall latencies are0.348us to0.395us and1.111us to1.157us respectively; the ratio of these medians is not the median of paired ratios. For16×16, candidate CPU-per-call observations span0.983–2.344us versus baseline0.948–1.343us, with late paired ratios0.630x,0.505x and0.520x. Thus the small difference between medians does not describe the entire observed variation; this nonstationarity is not grounds to remove observations or assume the slowdown is harmless.

Other unchanged controls below0.97x in paired wall or CPU speedup are2×8 (CPU 0.962x),2×16 (CPU 0.960x),2×4096 (CPU 0.966x; wall medians20.901us to21.955us),7×16 (CPU 0.968x),8×8 (CPU 0.966x),4×4095 (CPU 0.951x;43.864us to45.453us), and32×7 (wall0.965x but CPU1.023x). The latter is the only cell with wall/CPU speedup disagreement above 5%, so its wall slowdown should not be presented as a corroborated compute regression. All observations remain included; no cases were removed or rerun.

| n | m | baseline us | candidate us | median paired wall speedup | ratio of wall medians | median paired CPU speedup | path |
|---:|---:|---:|---:|---:|---:|---:|:---|
| 2 | 7 | 0.075 | 0.077 | 0.975 | 0.976 | 0.974 | unchanged |
| 2 | 8 | 0.081 | 0.083 | 0.958 | 0.975 | 0.962 | unchanged |
| 2 | 16 | 0.123 | 0.127 | 0.987 | 0.972 | 0.960 | unchanged |
| 2 | 31 | 0.199 | 0.205 | 0.994 | 0.968 | 0.994 | unchanged |
| 2 | 32 | 0.201 | 0.200 | 1.023 | 1.006 | 1.022 | unchanged |
| 2 | 33 | 0.205 | 0.201 | 1.045 | 1.020 | 1.027 | unchanged |
| 2 | 64 | 0.352 | 0.336 | 1.066 | 1.047 | 1.066 | unchanged |
| 2 | 4096 | 20.901 | 21.955 | 0.966 | 0.952 | 0.966 | unchanged |
| 4 | 7 | 0.109 | 0.104 | 1.056 | 1.045 | 1.056 | unchanged |
| 4 | 8 | 0.119 | 0.118 | 1.033 | 1.007 | 1.034 | unchanged |
| 4 | 16 | 0.202 | 0.204 | 1.000 | 0.993 | 1.000 | unchanged |
| 4 | 31 | 0.329 | 0.345 | 0.975 | 0.953 | 0.991 | unchanged |
| 4 | 32 | 0.358 | 0.363 | 0.982 | 0.987 | 0.981 | unchanged |
| 4 | 33 | 0.348 | 0.395 | 0.873 | 0.879 | 0.873 | unchanged |
| 4 | 64 | 0.695 | 0.681 | 0.995 | 1.021 | 0.994 | unchanged |
| 4 | 4096 | 42.514 | 30.977 | 1.385 | 1.372 | 1.385 | SIMD |
| 7 | 7 | 0.192 | 0.190 | 1.021 | 1.013 | 1.004 | unchanged |
| 7 | 8 | 0.237 | 0.210 | 1.089 | 1.124 | 1.089 | unchanged |
| 7 | 16 | 0.367 | 0.368 | 0.968 | 0.997 | 0.968 | unchanged |
| 7 | 31 | 0.692 | 0.672 | 0.985 | 1.031 | 0.985 | unchanged |
| 7 | 32 | 0.703 | 0.716 | 1.041 | 0.982 | 1.041 | unchanged |
| 7 | 33 | 0.658 | 0.691 | 1.013 | 0.953 | 1.013 | unchanged |
| 7 | 64 | 1.350 | 1.344 | 0.980 | 1.005 | 0.980 | unchanged |
| 7 | 4096 | 84.547 | 37.375 | 2.238 | 2.262 | 2.292 | SIMD |
| 8 | 7 | 0.226 | 0.233 | 0.981 | 0.969 | 0.972 | unchanged |
| 8 | 8 | 0.237 | 0.247 | 0.966 | 0.957 | 0.966 | unchanged |
| 8 | 16 | 0.416 | 0.419 | 1.000 | 0.992 | 0.998 | unchanged |
| 8 | 31 | 0.959 | 0.833 | 1.023 | 1.150 | 1.008 | unchanged |
| 8 | 32 | 0.944 | 0.378 | 2.474 | 2.500 | 2.461 | SIMD |
| 8 | 33 | 0.802 | 0.420 | 1.961 | 1.909 | 2.002 | SIMD |
| 8 | 64 | 1.525 | 0.638 | 2.457 | 2.392 | 2.481 | SIMD |
| 8 | 4096 | 95.040 | 37.613 | 2.519 | 2.527 | 2.492 | SIMD |
| 16 | 7 | 0.530 | 0.505 | 1.110 | 1.049 | 1.110 | unchanged |
| 16 | 8 | 0.526 | 0.541 | 0.972 | 0.973 | 0.973 | unchanged |
| 16 | 16 | 1.111 | 1.157 | 0.872 | 0.960 | 0.872 | unchanged |
| 16 | 31 | 1.896 | 1.865 | 1.010 | 1.016 | 1.008 | unchanged |
| 16 | 32 | 1.994 | 0.509 | 3.858 | 3.918 | 3.939 | SIMD |
| 16 | 33 | 2.045 | 0.660 | 3.168 | 3.097 | 3.168 | SIMD |
| 16 | 64 | 4.189 | 1.020 | 4.062 | 4.106 | 4.028 | SIMD |
| 16 | 4096 | 257.868 | 61.431 | 4.245 | 4.198 | 4.216 | SIMD |
| 31 | 7 | 1.193 | 1.174 | 1.015 | 1.016 | 1.015 | unchanged |
| 31 | 8 | 1.448 | 1.473 | 0.991 | 0.983 | 0.991 | unchanged |
| 31 | 16 | 2.811 | 2.689 | 1.022 | 1.046 | 1.021 | unchanged |
| 31 | 31 | 5.146 | 5.196 | 0.978 | 0.990 | 0.978 | unchanged |
| 31 | 32 | 5.470 | 0.808 | 6.676 | 6.767 | 6.680 | SIMD |
| 31 | 33 | 5.676 | 1.082 | 5.250 | 5.245 | 5.251 | SIMD |
| 31 | 64 | 10.877 | 1.719 | 6.721 | 6.326 | 6.721 | SIMD |
| 31 | 4096 | 2363.242 | 251.582 | 10.109 | 9.394 | 10.099 | SIMD |
| 32 | 7 | 6.954 | 3.743 | 0.965 | 1.858 | 1.023 | unchanged |
| 32 | 8 | 4.531 | 3.175 | 1.213 | 1.427 | 1.212 | SIMD |
| 32 | 16 | 3.617 | 0.980 | 3.607 | 3.689 | 3.606 | SIMD |
| 32 | 31 | 5.097 | 0.850 | 5.860 | 5.994 | 5.860 | SIMD |
| 32 | 32 | 5.403 | 0.836 | 6.459 | 6.462 | 6.459 | SIMD |
| 32 | 33 | 5.965 | 1.843 | 3.897 | 3.236 | 3.896 | SIMD |
| 32 | 64 | 12.446 | 1.743 | 7.437 | 7.142 | 7.437 | SIMD |
| 32 | 4096 | 720.346 | 98.314 | 7.357 | 7.327 | 7.226 | SIMD |
| 4 | 4095 | 43.864 | 45.453 | 0.951 | 0.965 | 0.951 | unchanged |
| 4 | 4097 | 43.824 | 30.725 | 1.388 | 1.426 | 1.388 | SIMD |
| 7 | 4095 | 81.415 | 78.022 | 1.006 | 1.043 | 1.006 | unchanged |
| 7 | 4097 | 85.925 | 39.717 | 2.247 | 2.163 | 2.192 | SIMD |
| 8 | 63 | 1.581 | 0.712 | 2.180 | 2.219 | 2.183 | SIMD |
| 8 | 65 | 1.703 | 0.741 | 2.222 | 2.298 | 2.222 | SIMD |
| 32 | 9 | 2.055 | 1.174 | 1.998 | 1.750 | 1.998 | SIMD |
| 32 | 15 | 2.659 | 0.805 | 3.381 | 3.303 | 3.381 | SIMD |
| 32 | 63 | 11.514 | 1.712 | 6.635 | 6.726 | 6.616 | SIMD |
| 32 | 65 | 15.160 | 2.690 | 5.769 | 5.637 | 5.769 | SIMD |
| 32 | 131072 | 22710.485 | 3416.585 | 7.133 | 6.647 | 7.131 | SIMD |
| 33 | 8 | 1.476 | 0.795 | 1.836 | 1.856 | 1.856 | SIMD |
| 256 | 8 | 13.482 | 5.026 | 2.639 | 2.683 | 2.639 | SIMD |
| 4096 | 7 | 195.478 | 197.169 | 0.992 | 0.991 | 0.993 | unchanged |
| 4096 | 8 | 220.703 | 75.101 | 3.017 | 2.939 | 3.017 | SIMD |
| 4096 | 31 | 862.251 | 77.579 | 11.152 | 11.114 | 11.184 | SIMD |
| 4096 | 32 | 871.966 | 77.236 | 11.451 | 11.290 | 11.450 | SIMD |
| 131072 | 32 | 29189.524 | 2633.850 | 11.587 | 11.082 | 11.586 | SIMD |
| 33 | 33 | 10.636 | 10.468 | 0.998 | 1.016 | 0.998 | unchanged |
| 1024 | 33 | 74.044 | 74.049 | 1.018 | 1.000 | 1.018 | unchanged |
| 32768 | 129 | 1900.994 | 1948.790 | 0.975 | 0.975 | 0.975 | unchanged |
| 32768 | 32768 | 11941.466 | 12072.539 | 0.999 | 0.989 | 0.999 | unchanged |

### Interpretation and limitations

The intended direct-Horner workloads improve in both compiler configurations, including4–7 coefficient polynomials evaluated at4096 or more points, and long polynomials evaluated at8–32 points. The strongest examples are degree-size131072 with32 points (paired wall10.337x portable,11.587x native) and32 coefficients with131072 points (6.594x portable,7.133x native). The singleton-tail case8×33 improves1.977x portable after the structural fix.

This comes with small absolute but measurable relative overhead on several unchanged scalar-path controls, including the native4×33 and16×16 losses above. Acceptance should weigh those explicit tradeoffs, not claim universal speedup. There is no claim that these dispatch thresholds are universally optimal or that measurements on this container predict Mac performance. Baseline and candidate use the same unchanged product-tree/NTT dependencies. All 156 shape/configuration summaries and all exploratory adverse cells are retained above.

Final source hashes:

- `fps/multipoint_eval.hpp`: `ac33415ec7aaead7adba8e1114f1ffc303a666c10549158be1ff1612fc839a94`
- `components/fps_direct_eval.hpp`: `73f3c335566cad50271ed5a0c8336ec10c57c6ec2c69a193c463939d15136380`
- `test/benchmark/multipoint_eval.cpp`: `565af0e1f069031c513f5eca40facb150c8644c6d56e53e7fb32f4893a066751`
- `test/benchmark/multipoint_eval.py`: `e58ccdb4de88df4115eb1c3bc2e99da01811604ed92ab488a2bf56468c4eeb67`

## Separate structural diagnostic: negative result

After the original full validation, one logically equivalent gate was tested in a separate workspace: `n>=4 && m>=8 && ((n|m)>=32) && (n>=8 || m>=4096)`. It changes no eligible SIMD dimensions. The original source and all original observations remained intact.

A reconstructed native three-variant binary compared baseline, original SIMD and factored SIMD. It was not the original timing binary. Assembly showed the same Montgomery Horner arithmetic in all scalar loops; the factored gate recovered baseline register allocation and reduced some dispatch comparisons, but increased the 4×33 comparison count. This did not establish the cause of the original regressions.

One predeclared screen used 19 active/control shapes, six rounds, all six permutations of the three variants, CPU 0 and at least 0.15 process CPU seconds per variant. All 342 observations were retained. It found no consistent repair:

- At 4×33, median paired CPU baseline/original was 1.019x, baseline/factored 0.967x, and original/factored 0.956x.
- At 16×16, baseline/original was 1.014x and baseline/factored 1.027x, while original/factored was 0.976x. The original source's earlier slowdown did not recur in this reconstructed binary, so its apparent disappearance cannot be credited to factorization. Medians of different paired ratios are not transitive.
- The six activated shapes retained baseline/factored speedups of 1.268–9.538x, but mixed unchanged-path outcomes did not justify replacing the original candidate or claiming that its regressions were fixed.

The alternative was not published and was not substituted into the full tables. This negative diagnostic explains the initial HOLD and remains part of the qualified decision; it is not a rerun selected to erase the original adverse results.
