# Separately bound rerooting correction: local review packet v2

This document records a local preparation scope, not authorization to publish or
launch. No v2 remote object, branch, workflow run or real timing is authorized
until independent review and a separate instruction to proceed.

## Preserved outcomes and explicit amendment

Original stage commit: 29ed98e92d5a30ef19a385f141bdf028d9d9ac53.
Original launch commit: 6becc2056a9c36d000a3e9765a5517a12a440e4d.
Original protocol SHA-256:
5df8eea912d81db9ca7c79665c1ccbac487524f55c1b8f154fb9c333e9b1010f.
Amended protocol SHA-256:
434d67065f3a7a3de061732c589a38af8f64a3197252ac144ae4b5956219e1ca.

Runs 39293044477 and 39293127017 are separate preserved inconclusive outcomes.
Each stopped at baseline-only centered-star calibration before reaching the
100 ms floor at 64 repetitions. Neither reached AA, AB, or candidate real-timed
batches. The second push run was unexpected and was not an authorized retry.
Its trigger cause remains unknown; no result is pooled or selected.

The authorized local correction changes only:
- Maximum repetitions to 128.
- Retained-tree accounting limit to 512 MiB, worker peak RSS to 1 GiB, and combined
  historical worker peaks plus collector to 2304 MiB.
- First-run admission in both workflow and prepare adapter.
- Lossless gzip/base64 log transport and its strict decoder.

All production source hashes, ten cells/seeds, compiler flags, output semantics,
timer boundaries, 100 ms floor, AA/AB schedules, performance gates and deadlines
remain fixed. Every future count starts fresh at 1; old calibrations cannot be
reused. The larger envelope does not guarantee adequacy. A cap, timeout, noise,
output/provenance, admission or transport failure remains terminal/inconclusive.

## Unchanged bounded resources

One public standard GitHub-hosted ubuntu-26.04 x86-64 job, installed GCC 15.2,
portable flags and no installation, paid/larger runner, job matrix, artifact
upload, cache, site output or deployment. Read-only contents permission only.
No additional API token or runtime persistent ledger is introduced.

- Pinned checkout: 60 seconds, no persisted credentials.
- Copy-only prepare: hard 30 seconds; one-minute step emergency ceiling.
- Full experiment supervisor: 660 seconds.
- Collector including full-size output preparation: 360 seconds.
- Post-run transport: hard 60 seconds; one-minute step ceiling.
- Outer job: 14 minutes. The sub-bounds total 810 seconds before scheduling overhead.
- Batch 2 seconds, each complete worker request 5 seconds.
- Collector RSS 256 MiB and total local evidence 256 MiB.

Timeouts invalidate qualification even if complete bytes later drain into logs.
Neither complete transport nor a successful GitHub color substitutes for a valid
complete collector outcome and unchanged performance gates.

## New identities and one-shot admission

Branch: codex/rerooting-identity-screen-v2-20261019.
Package: test/benchmark/rerooting_identity_v2.
Workflow: .github/workflows/rerooting-identity-screen-v2.yml.
Marker: test/benchmark/rerooting_identity_v2/launch.json.
Base main: 9f05f01500656c65e394a8aa11c3f5afa4d9495a.
Base tree: a2d8f814ffb2df1010396d9b487e8b9d41ceeaa3.

Use a unique previously unused workflow filename and previously unseen workflow
ID. After staging, and again before any marker update, a read-only publication
audit must retain the verified workflow identity and an unfiltered complete
workflow-run response proving zero prior runs for that ID. Do not substitute
head-only filtering, branch-only filtering, an incomplete page, or absence of a
visible job for this check. If identity or absence cannot be verified, stop.

Both the workflow job condition and prepare adapter require run_number == 1 AND
run_attempt == 1 and record both numbers. Existing exact repository/public,
branch/ref/head, marker/stage/base, no creation/deletion/force, clean checkout,
file-mode/hash and complete workflow-inventory checks remain. A skipped stage
run consuming number 1 blocks launch. Never relax the guard to 2, reset a counter,
rename/recreate the workflow, or send a second trigger within this experiment.
The guard mitigates duplicate execution; it does not establish the old cause.
GitHub documents distinct per-workflow run numbers and per-run attempts:
https://docs.github.com/en/actions/reference/workflows-and-actions/variables

Create a new branch directly at the reviewed isolated stage, never at bare main.
That stage must lack the marker. A separately authorized marker-only commit must
have the exact stage as sole parent and bind its commit/tree/binding digest.
Recheck actual head and use a nonforce expected-head update at most once, subject
to the action-specific authorization and uncertain-outcome rules.

Only the bound package/new workflow and temporary exact-branch exclusions in
verify.yml/macos-portability.yml may differ from pinned main. Preserve tag pushes
with tags: ['**']; PR/dispatch and the other 17 workflow bodies remain unchanged.
No production header/test is changed. Never merge this experiment branch or its
temporary controls into main.

## Non-circular binding and isolated build

binding.json hashes stage files except itself, including workflow, adapter,
protocol, decoder and all source inputs. The later marker binds the reviewed
binding digest. Prepare verifies the complete stage and emits only the bound
64-hex workflow hash to the unchanged seven-argument launcher. It copies exactly
the bound packet files into a new isolated RUNNER_TEMP directory and refuses
existing output. Do not import old binaries, manifests, fixtures or calibrations
into a future remote experiment.

Local changed-constant validation may rebuild only the common worker and reuse
hash-verified unchanged kernel objects. That local validation is not remote
qualification. The future remote build retains complete compiler/backend,
object, dependency, library, symbol, disassembly and source provenance.

Focused local preflight boundaries and synthetic collector/admission/transport
tests passed. The first 128-repetition synthetic-clock worker batch exceeded the
unchanged five-second request watchdog and was killed without a response; later
lifecycle checks did not run. Preserve this failure. It is not a production-source
regression finding, a full 128-repetition lifecycle pass, or proof of remote
adequacy. No retry or limit relaxation was made.

## Lossless bounded transport

Canonical binary fixtures and executable/object bytes remain hash-only records;
complete outputs are logically and bytewise verified on the runner. This is not
a claim that checksums recover those bytes. Source/seed identities, comparison
counts and all canonical-file hashes remain retained.

Every raw timing, warmup, calibration, compiler, provenance, validation, control,
failure and partial text file is preserved exactly after decoding. Unknown
generated files are not silently omitted. Each exact-byte file is deterministically
gzip-compressed with standard-library level 6, mtime 0 and an empty filename, then
emitted in indexed base64 chunks.
Inventory binds original size/SHA-256 and compressed size/SHA-256. No compressed
spool or unbounded in-memory payload is required.

Reconstruction checks the complete inventory/footer, file set, chunk indices,
offsets and lengths, compressed hashes, bounded expansion, a single complete gzip
member with no trailing data, original length and original hash. Corruption,
truncation, duplicate/missing chunks/files, oversize expansion, source mutation
or a symlink fails closed. Hash-only classes keep their exact identities.
No credential or arbitrary environment dump is added.

Both old logs had complete independently reconstructed byte streams despite
step timeouts. Their raw payloads were about 5.253 MB, dominated by 3.588 MB of
disassembly; fixed level-6 per-file gzip of the preserved first/second evidence
was 811566/811485 bytes before base64. Each of the 96 exact files in each run
round-tripped with its original length and hash; the 143 historical hash-only
files were not re-encoded. These are offline byte counts, not timings
or a guarantee that v2 transport will fit. Queued runner log draining is a possible
mechanism for the old timeout/footer combination, not an established cause.
The 60-second cap is not increased.

## Review and next authorization boundary

Freeze exact v2 source, tests, manifests, all changed-line diffs and focused
synthetic results. Preserve both old runs and their original bytes/hashes.
Independent review is required before any remote stage publication. A separate
instruction is required before the marker update. No PR or merge is authorized
by this correction packet; performance, latest-head tests/CI and Mac requirements
remain separate.
