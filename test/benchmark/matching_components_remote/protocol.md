# Frozen GeneralMatching component reset validation

This temporary branch is evidence only. Do not merge it. Production files and existing workflows stay unchanged.

Source commit: 13578ced48ae2e7013f0ae7308d5672bcce1d076.
Frozen candidate SHA-256: 9e7cbfa85321e32aa85d8d8a53a2376a78f1f496c5c83575d388be9c34b5550f.
Baseline commit: 8c38d74c07610dfd1a5fb13b1beeb18313ce1e04. Baseline header SHA-256: 62c215ab4bf9d24bcffc8da5defb9e5b187e69975276e78413d05f965e0007be.
Original report SHA-256: 393f06d0e69533f62d3c62d6f64a030b0e80f5dec9505df5585d6ebf91efea53.
Original local results, including native star paired median 0.942 and generic tree 0.839, remain evidence; independent results do not replace them. The current performance decision is HOLD.

Run exactly once on GitHub Actions ubuntu-24.04, GCC, CPU 0 (abort if unavailable). Run frozen unit and independent exact-mate correctness at GCC O2 and GCC ASan/UBSan first; then invoke the original frozen Python final benchmark once. No pilot, source retuning, retry, case removal, or favorable-run selection.

The frozen C++ defines all 22 cases, six AB/BA-balanced pairs per case, measured CPU time at least 0.15 seconds per measured variant, generic -O2 then -O2 -march=native, assertions enabled. Input generation and equality checking are excluded, construction/add/build/workspace destruction are included. The mate vector destruction and checksum are excluded. Every repeated result must equal the expected mate vector.

The original reset bound >1e9 omits all three 100000-vertex baseline cases; these remain candidate-only, never speedups. An initial baseline solve >5 CPU seconds censors that whole case, as the frozen harness specifies. Preserve every observation and censor/omission. Per-pair CPU times are available; per-pair wall times are not. Record whole-protocol wall time, hashes, environment and binaries separately without changing the benchmark.

Audit all expected rows, six-pair ordering, actual CPU floors, cap semantics, and source hashes. Report every control and adverse pair. Hardware/compiler changes are confounded and do not establish the cause of local variation. No AtCoder equivalence or macOS validation claim. Interpretation gates fixed before the remote run:
- Correctness must pass exactly, including all mate arrays and 42344 independent differential checkpoints per build.
- Meaningful target gains must reproduce: every 10000-vertex deficient target must have paired median and ratio of medians at least 2x, with all six pairs faster. This deliberately conservative floor is far below the original local gains.
- For every common connected or saturated control, report paired median, ratio of medians, all six ratios, and slower-pair counts. A persistent >5% common-control runtime loss means HOLD. For this run, flag persistence when at least five of six pairs are slower and either aggregate has candidate/baseline >1.05 (equivalently baseline/candidate <1/1.05). Also surface all other adverse observations, even those not meeting this flag.
- Original local native-star six-of-six loss remains unresolved even if the remote runner shows parity. Remote parity cannot by itself release the hold or establish that original losses were noise. Hardware and compiler effects are confounded. Portability tradeoffs require a separate review.
- Do not redefine these gates after results. Material or unresolved adverse evidence means HOLD pending review. No production PR or merge is authorized by this run.
