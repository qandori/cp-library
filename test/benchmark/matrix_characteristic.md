# Characteristic polynomial: batched Hessenberg column updates

## Scope

The public `Matrix<T>::characteristic_polynomial(eps)` API and coefficient order are unchanged.
For the existing compile-time small-modint dispatch and matrices of order at least 16,
the Hessenberg reduction first performs all row eliminations for a pivot, then updates
the destination column with contiguous row dot products. Generic fields, floating-point
fields, and smaller matrices keep the original arithmetic order.

The elementary row updates commute with one another, as do the column updates. Left
and right matrix multiplication commute as operations on the working matrix. Their
factors depend only on column `k - 1`, which the column updates do not modify. Thus
regrouping the same similarity transforms preserves the Hessenberg matrix exactly
in the modular path. All-zero factors skip the column work.

The dot product reduces after at most 16 products when the modulus is at most `2^30`,
and after at most 4 otherwise. The existing dispatch requires `1 < mod < 2^31`.
The accumulator is bounded by `(mod - 1) + step * (mod - 1)^2 < 2^64`, including
the residue carried from the preceding block. No additional ISA requirement is introduced;
the existing AVX2 row operations retain their portable fallback.

## Reproduction

Baseline: `6af78c247f5c9beba109116966f3a3d4e9dc5104`.
Create an unmodified baseline checkout and run:

```sh
python3 test/benchmark/matrix_characteristic.py \
  --baseline /path/to/baseline --output /tmp/matrix-characteristic-results --cpu 0
```

The driver includes both the baseline matrix implementation and its arithmetic helpers
under separate names in the same executable. It alternates the order of the two variants,
checks output checksums, and records process CPU time as well as wall time. Each case is
calibrated to at least 0.15 CPU seconds per variant before five fixed paired trials.
The generic `-O2` and `-O2 -march=native -ftrivial-auto-var-init=zero` builds use C++23.
Machine-generated measurements remain outside the repository.

## Reference implementation

Library Checker AC submission [404541](https://judge.yosupo.jp/submission/404541),
by adamant, was listed at 0.032 seconds in the official fastest-AC query retrieved
2026-10-17 04:31 UTC. Its source was independently retrieved again at 10:05 UTC.
Source SHA-256: `fd132a5b49252ddb26f464661c9240b66b8194acc6b181ed489341d862b660d7`.
It uses randomized Krylov/Frobenius decomposition and batched vector arithmetic.
This patch keeps the existing deterministic Hessenberg algorithm; the reference's
judge runtime is not an apples-to-apples speed comparison or an AtCoder measurement.

## Validation

The permanent focused test compares every coefficient against the untouched generic-field
algorithm for six moduli, fourteen dimensions, and nine matrix shapes, including sparse,
upper-triangular, Hessenberg, permutation-cycle, low-rank, maximal-residue, and pivot-swap
cases. It also evaluates the polynomial against independent determinant computations,
checks that the input is unchanged, and exercises dot-product block boundaries including
moduli exactly `2^30` and `2^30 + 1`. Existing matrix unit tests cover integration with
matrix multiplication, inversion, kernels, rational arithmetic, and floating-point fields.

## Local paired measurements (2026-10-17)

Linux x86-64, Intel Xeon Platinum 8573C, GCC 14.2.0, CPU 0 affinity.
These are local measurements, not AtCoder hardware or Mac performance measurements.
All 220 measured pairs (44 build/case cells) met the 0.15-second CPU-time floor and
matched checksums. Ratios below are medians of baseline/candidate CPU-time ratios
within each pair; greater than one is faster. Absolute timings are separate
per-variant medians per call and need not yield exactly the paired median ratio.

| Field | Size | Shape | Generic `-O2` ratio | Native ratio | Native baseline / candidate (µs) |
|---|---:|---|---:|---:|---:|
| mint | 0 | dense | 0.931x | 0.951x | 0.066 / 0.069 |
| mint | 1 | dense | 1.011x | 0.974x | 0.126 / 0.129 |
| mint | 8 | dense | 1.020x | 1.017x | 3.360 / 3.338 |
| mint | 15 | dense | 0.951x | 1.052x | 13.422 / 12.911 |
| mint | 16 | dense | 0.945x | 1.165x | 15.508 / 12.963 |
| mint | 17 | dense | 1.229x | 1.189x | 17.271 / 14.879 |
| mint | 64 | dense | 1.736x | 1.618x | 439.715 / 273.177 |
| mint | 128 | dense | 2.274x | 2.252x | 3961.208 / 1721.275 |
| mint | 256 | dense | 2.594x | 2.366x | 31262.889 / 12864.333 |
| mint | 512 | dense | 3.115x | 2.817x | 341415.667 / 121734.000 |
| mint | 64 | zero | 1.000x | 1.039x | 90.326 / 86.946 |
| mint | 512 | zero | 0.983x | 1.022x | 14888.467 / 14713.333 |
| mint | 128 | triangular | 0.991x | 1.077x | 440.480 / 409.058 |
| mint | 128 | hessenberg | 0.951x | 0.998x | 474.952 / 470.307 |
| mint | 128 | sparse | 2.179x | 2.233x | 3977.636 / 1767.371 |
| mint | 128 | cycle | 0.993x | 1.105x | 557.256 / 506.492 |
| mint | 128 | max | 1.111x | 1.102x | 486.603 / 443.745 |
| mod31 | 128 | dense | 1.942x | 1.821x | 4648.412 / 2568.647 |
| mod31 | 512 | dense | 2.101x | 1.981x | 322860.000 / 163398.000 |
| mod2 | 128 | dense | 1.505x | 1.340x | 1807.070 / 1348.063 |
| generic | 16 | dense | 0.932x | 1.055x | 19.296 / 18.722 |
| generic | 64 | dense | 1.051x | 1.004x | 938.458 / 927.159 |

Dense `mint` matrices of order 64–512 improve by 1.62–3.11x across the two builds;
sparse order-128 inputs improve by 2.18–2.23x. The `2^31 - 1` modulus and binary field
also improve. This is not a blanket no-regression result:

- Empty matrices have paired ratios 0.931x/0.951x; the separate median call times
  increase by approximately 4.3 ns/2.7 ns in the generic/native builds.
- Generic-build dense sizes 15 and 16 have paired medians 0.951x and 0.945x.
  Individual pairs range 0.713–1.329x and 0.357–1.275x respectively, so the
  magnitude of these small-size changes is not established precisely.
- Generic-build order-128 Hessenberg input has a 0.951x paired ratio; its
  separate median per-call times are 402.45/414.43 µs. The native counterpart
  is 0.998x. Already-reduced matrices do not benefit from batched elimination.
- The untouched generic-field size-16 control has a 0.932x generic-build median
  and 1.055x native-build median. It is retained rather than removed from the report.

No samples were discarded and no implementation changes were made in response to
these final measurements. Performance acceptance must weigh the substantial target
workload improvement against these explicit small/structural-control caveats.

The independent reviewer additionally passed 720 permuted-triangular known-polynomial
cases, 720 dense/sparse exact-baseline comparisons, and 216 floating-point exact-bit
comparisons in ordinary, `MATRIX_DISABLE_AVX2`, and ASan/UBSan builds. The focused
permanent test and the existing full matrix unit suite passed locally. macOS and
project CI still need to pass on the final PR revision before merge.
