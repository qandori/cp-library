# Dynamic Wavelet Matrix: second Nim reference

The supplied source embeds kemuniku/cplib commit
[`244c855f5961a2ce03a3acebed8eac1f5f4099fc`](https://github.com/kemuniku/cplib/tree/244c855f5961a2ce03a3acebed8eac1f5f4099fc).
`prepare.py` saves the original bytes in an ignored cache, records their SHA256,
and retrieves the fixed commit's CC0-1.0 LICENSE. The reference itself is not
vendored into this directory.

`prepare.py` distinguishes two compiler configurations:

| Name | Backend and flags | Source checks |
| --- | --- | --- |
| `official_*` | Nim2.2.4, the published AtCoder C++ backend/release flags | Original dependency settings; `second_compile` only skips self-recompilation |
| `optimized_*` | The supplied `optimize()` second-stage C backend command, ARC, danger, LTO, native | Disabled by the supplied command |

These configurations use the same GCC installation, but their backend language,
optimization flags, and checks differ. They are separate controls.
Nim's generated build JSON records the actual backend commands, including the
release configuration's O3 flags. C++ consumers use GCC O2 with their existing
local header optimization pragmas, assertions enabled, and no LTO.

Both Nim sources retain unsigned/nonnegative H=30, `FastQuadVector`, and the
public `kth_largest(0)`, `prev_value`, `count` calls. The API source reads all
operations and allocates answers before timing, then writes every answer after
timing. The stream source keeps the supplied online main, including the guarded
`optimize()` call. An AVX-512 kernel is requested only when runtime detection
allows it. The i7-13700F test host has AVX2 but no AVX-512; results on this host
do not measure the AVX-512 branch.

`cases.py` reuses the independent top-two segment tree oracle from the first
comparison. It generates the original six N=Q=200000 workloads plus
`update_heavy` (99% updates) and `query_only`. Small cases additionally cover
positions around 4096, 8192, 16384, and 32768, with every small-case answer
cross-checked against Counter and sorting. Manifests record operation counts,
fixed seeds, and input/answer hashes.

```sh
python3 test/benchmark/dynamic_wavelet_nim_round2/cases.py \
  --root .verify-helper/cache/dynamic-wavelet-nim-round2 --small --large
python3 test/benchmark/dynamic_wavelet_nim_round2/prepare.py \
  --source /path/to/supplied.txt \
  --work .verify-helper/cache/dynamic-wavelet-nim-round2/reference --compile
python3 test/benchmark/dynamic_wavelet_nim_round2/validate_reference.py \
  --reference .verify-helper/cache/dynamic-wavelet-nim-round2/reference \
  --cases .verify-helper/cache/dynamic-wavelet-nim-round2/cases-small \
  --output .verify-helper/cache/dynamic-wavelet-nim-round2/nim-small-validation.json
```

Nim2.2.4 is fetched from the official tarball and checked against its published
SHA256, or supplied with `prepare.py --nim /path/to/nim`. The original source
file and toolchain stay in the ignored cache.

`export_reference.py` records the fixed upstream dependency hashes and exports
the actual backend commands with `${REFERENCE}` replacing the private cache
directory. It checks the frozen source and binary hashes before exporting.

`compare.py prepare` freezes the baseline and current C++ headers and driver.
It compiles signed32/32-bit and unsigned32/30-bit controls separately, both with
FastInput/FastOutput and the same `kth_largest(0)`, `prev_value`, `count` calls.
The signed32 control preserves the public default type; unsigned30 also matches
the Nim value domain.
Streaming results also retain the original input-library difference: C++
FastInput maps regular files, whereas the supplied Nim source disables mmap by
default. API timers exclude both input implementations and answer output.

```sh
python3 test/benchmark/dynamic_wavelet_nim_round2/compare.py prepare \
  --repo . --baseline 5012542 \
  --work .verify-helper/cache/dynamic-wavelet-nim-round2/final
python3 test/benchmark/dynamic_wavelet_nim_round2/compare.py measure \
  --work .verify-helper/cache/dynamic-wavelet-nim-round2/final \
  --reference .verify-helper/cache/dynamic-wavelet-nim-round2/reference \
  --cases .verify-helper/cache/dynamic-wavelet-nim-round2/cases-large \
  --small .verify-helper/cache/dynamic-wavelet-nim-round2/cases-small \
  --output .verify-helper/cache/dynamic-wavelet-nim-round2/final/results.json \
  --repeats 7 --cpu 2
```

Size controls reuse the same frozen C++ binaries. `--only-variants` omits Nim
and all other compiler controls; `--validated-from` reuses successful small-case
checks only when both the input and binary hashes match.

```sh
python3 test/benchmark/dynamic_wavelet_nim_round2/compare.py measure \
  --work .verify-helper/cache/dynamic-wavelet-nim-round2/final \
  --reference .verify-helper/cache/dynamic-wavelet-nim-round2/reference \
  --cases .verify-helper/cache/dynamic-wavelet-nim-round2/cases-sizes \
  --small .verify-helper/cache/dynamic-wavelet-nim-round2/cases-small \
  --only-variants cpp_before_i32_api cpp_after_i32_api \
  --validated-from test/benchmark/results/dynamic_wavelet_nim_round2/final.json \
  --output test/benchmark/results/dynamic_wavelet_nim_round2/final_sizes.json \
  --repeats 3 --cpu 2
```

Every compiler/test/timing process takes the exclusive
`/tmp/cp-wm-opt.lock`. Timed children are pinned to CPU2 and waited on with a
blocking wait. Input and output use regular files. Runs include one warmup and
shuffled execution order; all timed outputs are checked by SHA256. Raw process
wall/CPU samples, API construction/operation timers, source/binary/compiler
hashes, and both compiler configurations are recorded. Comparisons of stream
and API measurements remain separate.

## Broad API and select controls

`select_screen.py` reconstructs the three extended-select controls from the
final production source and commit `5012542`: the original 1024-element leaves,
8192-element leaves with the old linear word scan, and the same 8192-element
leaves with prefix binary search. The latter two use quarter occupancy and full
bulk build. They isolate the select correction and do not use the final compact
bulk build. `variants.py` removes compact-build changes before generating these
controls; the old selector is read from the fixed baseline commit.

The benchmark source and escape helper are reused from `dwm_avx_final`, with the
same public-matrix, no-op-update, and existing-value-query substitutions as
`dynamic_wavelet_quaternary/run_api.py`. `select_oracle.cpp` checks every
occurrence, first/final words, exact prefix boundaries, long runs of empty
words, uniform/missing values, and changes after set/move/insert/erase.
The helper runs it with native GCC and scalar Clang ASan+UBSan before timing.

```sh
python3 test/benchmark/dynamic_wavelet_nim_round2/select_screen.py \
  --repo . --work .verify-helper/cache/dynamic-wavelet-nim-round2/select-replay \
  --check-recorded test/benchmark/results/dynamic_wavelet_nim_round2/select_screen.json \
  --output .verify-helper/cache/dynamic-wavelet-nim-round2/select-replay.json
```

The helper uses CPU2, the common exclusive lock, five shuffled samples, and
records full input/output/final-state hashes. Unsigned7 `select` uses one million
operations; signed64/unsigned64, duplicate valid selection, missing values, and
out-of-range ranks use 200000 operations. Operation timers exclude construction,
warmup, hashing, final extraction, and destruction. Construction time and memory
are recorded separately. Use `--prepare-only` with the same hash check to verify
the controls without compiling or rerunning measurements.

The 52-workload API screen uses the existing runner through a small adapter:

```sh
python3 test/benchmark/dynamic_wavelet_nim_round2/select_screen.py \
  --api --repo . --work .verify-helper/cache/dynamic-wavelet-nim-round2/api-replay \
  --output .verify-helper/cache/dynamic-wavelet-nim-round2/api-replay.json
```

The adapter presents the frozen scan control as the current source to
`dynamic_wavelet_quaternary/run_api.py`, points read-only Git object lookup at
the real repository, and holds the common lock for the entire child run.
The existing runner keeps its four integer domains, 13 operation kinds,
104 independent small vector-oracle checks, and 312 large differential runs.
It compares the original baseline with the full-build scan candidate, before
the select correction. The separate extended-select screen records that
regression and its correction. Both commands require a fresh output directory
and GCC15/Clang21 on `PATH`; they do not modify production headers.
