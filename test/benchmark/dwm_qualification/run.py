#!/usr/bin/env python3
"""Source-only draft: bounded log-only, own-library phase-two qualification.

DO NOT execute a proposal. Invocation after review/freeze and authorization:
    python3 runner_draft.py REPOSITORY EMPTY_WORK_DIRECTORY FROZEN_PLAN SHARD_ID

The plan is self-contained. In addition to baseline, variants, cases, compiler,
sample_policy, comparison_rows and jobs, it must contain frozen:true and:
  package.paths = {bench:'bench.cpp', caller:'caller.cpp', oracle:'verify_oracle.py',
                   escape:'escape.cpp', full_oracle:'vector_oracle.cpp', patches:'patches'}
  package.source_sha256 = {every packaged source/patch/runner relative path: SHA256}
  package.runner = 'runner_draft.py'  # optional; default shown
Every used variant must specify base, ordered components, component_patch_sha256,
and headers for both PUBLIC_HEADERS below, including baseline and individuals.
Paths are relative to the plan's directory. The frozen package must contain the
runner, bench, caller, both oracles, escape, and all selected patches. No absolute or
parent-traversing package paths are accepted. Freeze these after independent
review; this file intentionally does not fill in, infer or relax missing hashes.
The plan itself is logged by SHA256; do not include its self-referential hash in
package.source_sha256. Package snapshots / signatures are a separate review step.

No network, uploads, persistent caches, third-party sources, git writes or
artifact outputs. Only ephemeral work files and normal stdout JSONL are used.
Optionally writes a bounded summary to the existing GITHUB_STEP_SUMMARY path.
All child commands have finite timeouts, bounded streamed stdout/stderr, start
and end events, and output SHA256/lengths even on failure. objdump -d and git
archive binary output are not dumped: archive goes to an ephemeral local file;
disassembly is streamed through a bounded-memory hash/ISA scanner. Its explicitly
labeled digest-only command output is not confused with complete textual logs.
Event-chain validation: remove event_chain_sha256 from each parsed record, encode
with canonical(), then SHA256(previous-chain bytes || canonical record); sequence
numbers must be contiguous and the completed event must be present with its
matching final_event_count_including_this. Missing terminal/truncated logs are
incomplete regardless of any earlier pass records.
Truncated ordinary command output, log-budget exhaustion, failures and timeouts
make the job incomplete. Nothing is retried to obtain a favorable result.

Timed harness contract (review/freeze source; not proved by a JSON claim): each
replay constructs a fresh object and performs the same workload; output digest
is computed AFTER the timer on every replay and compared with preflight; only
the FIRST final vector is read back after timing. Preflight uses exactly THREE
fresh replay resets with full output/final-vector equality. A small independent
Python vector oracle is additional to full-sized baseline byte equality and an
independent C++ vector oracle on each distinct full input/type. C++ expected
outputs are computed once, then cached only after success; reuse requires exact
input/type and output bytes. A failed/timed-out oracle is never retried or
downgraded to baseline equality. build/rebuild
have q=0: their timed boundary is external object escape, not per-replay state
validation. Preflight does full readback, timed samples validate first final
state only. The empty build output digest provides no build-state guarantee.

The runner is descriptive, never an acceptance/rejection or merge decision.
It does not establish Mac compatibility, latest-head CI, external-reference
performance, broad-width coverage or completion of separate correctness suites.
"""
from __future__ import annotations

import base64
import collections
import dataclasses
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import selectors
import shlex
import shutil
import signal
import statistics
import struct
import subprocess
import sys
import tarfile
import time
from typing import Callable

PUBLIC_HEADERS = ('components/dynamic_bit_vector.hpp',
                  'data_structure/dynamic_wavelet_matrix.hpp')
OWN_COMPONENTS = frozenset(('neighbors', 'uniformneighbors', 'selectprefix',
                            'rangeprefix', 'pdep', 'avxbuild', 'leaf'))
REQUIRED_COMMON = ('-O2', '-std=gnu++23', '-fmodules',
                   '-ftrivial-auto-var-init=zero', '-DONLINE_JUDGE', '-DATCODER')
REQUIRED_SCALAR = ('-march=x86-64', '-mno-avx', '-mno-avx2', '-mno-bmi', '-mno-bmi2')
ABSENT_SCALAR = ('__AVX__', '__AVX2__', '__AVX512F__', '__BMI__', '__BMI2__')
CYCLE_TOKENS = (('AA', 'ABBA', 'BB', 'BAAB'), ('BB', 'BAAB', 'AA', 'ABBA'))
TYPES = {'uint32_t': 'std::uint32_t', 'int32_t': 'std::int32_t',
         'uint64_t': 'std::uint64_t', 'int64_t': 'std::int64_t'}
CALLERS = {'broad_dispatch': (0, 'bench'), 'independent_access_only': (1, 'caller'),
           'independent_mixed_ordinary': (2, 'caller')}
SHA = re.compile(r'^[0-9a-f]{64}$')
CHUNK = 4096
MAX_RECORD = 32768
COMMAND_OUTPUT_LIMIT = 65536  # each ordinary stdout/stderr stream
CAPTURE_LIMIT = 262144
SUMMARY_HARD_LIMIT = (1 << 20) - 2
TERMINAL_RESERVE = 131072


def require(condition, message):
    if not condition:
        raise GateError(message)


class GateError(RuntimeError):
    pass


class LogLimit(RuntimeError):
    pass


def canonical(obj):
    return json.dumps(obj, sort_keys=True, separators=(',', ':'), allow_nan=False).encode()


def sha_bytes(data):
    return hashlib.sha256(data).hexdigest()


def file_info(path):
    path = Path(path)
    h = hashlib.sha256()
    size = 0
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
            size += len(chunk)
    return {'sha256': h.hexdigest(), 'bytes': size}


def first_byte_difference(actual, expected):
    """Exact bounded-memory comparison; return only the first mismatching window."""
    offset = 0
    with Path(actual).open('rb') as a, Path(expected).open('rb') as e:
        while True:
            aa, ee = a.read(1 << 20), e.read(1 << 20)
            if aa != ee:
                within = next((i for i,(x,y) in enumerate(zip(aa,ee)) if x != y),min(len(aa),len(ee)))
                first = offset+within
                lower = max(0,first-32)
                a.seek(lower); e.seek(lower)
                return dict(first_byte_offset=first,window_start=lower,window_max_bytes=65,
                            actual_window_base64=base64.b64encode(a.read(65)).decode(),
                            expected_window_base64=base64.b64encode(e.read(65)).decode(),
                            actual=file_info(actual),expected=file_info(expected))
            if not aa:
                return None
            offset += len(aa)


def safe_relative(value):
    require(isinstance(value, str) and value and '\\' not in value,
            f'Invalid package relative path: {value!r}')
    p = PurePosixPath(value)
    require(not p.is_absolute() and '..' not in p.parts and p.as_posix() == value,
            f'Noncanonical package path: {value!r}')
    return p


class Log:
    def __init__(self, limit=8 << 20):
        self.limit, self.bytes, self.seq = limit, 0, 0
        self.incomplete = False
        self.chain = '0'*64
        self.run_id = sha_bytes(canonical([os.getpid(), time.time_ns(), os.environ.get('GITHUB_RUN_ID'),
                                           os.environ.get('GITHUB_RUN_ATTEMPT')]))[:24]

    def emit(self, event, terminal=False, **fields):
        next_seq = self.seq + 1
        obj = dict(event=event, seq=next_seq, run_id=self.run_id, utc_unix=time.time(),
                   previous_event_chain_sha256=self.chain, **fields)
        next_chain = sha_bytes(bytes.fromhex(self.chain) + canonical(obj))
        obj['event_chain_sha256'] = next_chain
        raw = canonical(obj) + b'\n'
        if len(raw) > MAX_RECORD:
            raise LogLimit(f'record exceeds {MAX_RECORD} bytes: {event}')
        ceiling = self.limit if terminal else self.limit - TERMINAL_RESERVE
        if self.bytes + len(raw) > ceiling:
            self.incomplete = True
            raise LogLimit('normal JSONL log budget exhausted; no further work is allowed')
        sys.stdout.buffer.write(raw)
        sys.stdout.buffer.flush()
        self.bytes += len(raw)
        self.seq, self.chain = next_seq, next_chain


@dataclasses.dataclass
class Result:
    command_id: int
    returncode: int | None
    stdout: bytes
    stderr: bytes
    duration: float
    timed_out: bool
    output_complete: bool
    stdout_info: dict
    stderr_info: dict


class CommandError(RuntimeError):
    def __init__(self, message, result):
        super().__init__(message)
        self.result = result


class Commands:
    def __init__(self, log, environment, deadline):
        self.log, self.env, self.deadline = log, environment, deadline
        self.next_id = 0
        self.counts = collections.Counter()

    def run(self, argv, *, timeout, context=None, cwd=None, env_delta=None,
            stdin=b'', digest_stdout=False, observer: Callable | None = None):
        self.next_id += 1
        cid = self.next_id
        context = context or {}
        self.counts[context.get('stage',context.get('gate','other'))] += 1
        argv = list(map(str, argv))
        remaining = self.deadline - time.monotonic()
        require(timeout > 0 and remaining > 0, 'Shard execution deadline reached')
        timeout = min(float(timeout), remaining)
        delta = env_delta or {}
        env = dict(self.env, **delta)
        self.log.emit('command_start', command_id=cid, argv=argv,
                      cwd=str(cwd) if cwd else None, timeout_seconds=timeout,
                      env_delta=delta, stdin_sha256=sha_bytes(stdin), stdin_bytes=len(stdin),
                      stdout_policy='digest_only_disassembly' if digest_stdout else 'streamed_base64',
                      context=context)
        started = time.monotonic()
        proc = None
        hashes = {s: hashlib.sha256() for s in ('stdout', 'stderr')}
        lengths = dict(stdout=0, stderr=0)
        emitted = dict(stdout=0, stderr=0)
        capture = dict(stdout=bytearray(), stderr=bytearray())
        tails = dict(stdout=bytearray(), stderr=bytearray())
        incomplete = False
        timeout_hit = False
        pending_error = None
        selector = selectors.DefaultSelector()
        try:
            proc = subprocess.Popen(argv, cwd=cwd, env=env, stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    start_new_session=True)
            # All caller stdin payloads are empty or short macro-probe source.
            if stdin:
                proc.stdin.write(stdin)
            proc.stdin.close()
            for stream in ('stdout', 'stderr'):
                pipe = getattr(proc, stream)
                os.set_blocking(pipe.fileno(), False)
                selector.register(pipe, selectors.EVENT_READ, stream)
            while selector.get_map():
                elapsed = time.monotonic() - started
                if elapsed >= timeout and not timeout_hit:
                    timeout_hit = True
                    self._kill(proc)
                    self.log.emit('command_timeout', command_id=cid, elapsed_seconds=elapsed,
                                  context=context)
                # Killing the process group normally closes pipes. Bound even a
                # descendant which escaped its group and retained an inherited FD.
                if elapsed >= timeout + 5:
                    incomplete = True
                    for item in list(selector.get_map().values()):
                        selector.unregister(item.fileobj)
                        item.fileobj.close()
                    break
                for key, _ in selector.select(min(0.2, max(0.01, timeout - elapsed))):
                    chunk = os.read(key.fd, CHUNK)
                    stream = key.data
                    if not chunk:
                        selector.unregister(key.fileobj)
                        key.fileobj.close()
                        continue
                    offset = lengths[stream]
                    lengths[stream] += len(chunk)
                    hashes[stream].update(chunk)
                    tails[stream].extend(chunk)
                    del tails[stream][:-CHUNK]
                    if len(capture[stream]) < CAPTURE_LIMIT:
                        capture[stream].extend(chunk[:CAPTURE_LIMIT-len(capture[stream])])
                    if stream == 'stdout' and digest_stdout:
                        if observer:
                            observer(chunk)
                        continue
                    room = COMMAND_OUTPUT_LIMIT - emitted[stream]
                    shown = chunk[:max(0, room)]
                    if shown:
                        self.log.emit('command_output', command_id=cid, stream=stream,
                                      offset=offset, encoding='base64', data=base64.b64encode(shown).decode())
                        emitted[stream] += len(shown)
                    if len(shown) != len(chunk):
                        incomplete = True
                        # Never keep an unbounded chatty command alive. Its partial
                        # output is preserved and this command can never pass.
                        self._kill(proc)
            if proc.poll() is None:
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    timeout_hit = True
                    self._kill(proc)
                    proc.wait(timeout=5)
        except BaseException as error:
            pending_error = error
            incomplete = True
            if proc is not None:
                self._kill(proc)
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    pass
        finally:
            selector.close()
            for stream in ('stdout', 'stderr', 'stdin'):
                pipe = getattr(proc, stream, None) if proc else None
                if pipe and not pipe.closed:
                    pipe.close()
            duration = time.monotonic() - started
            infos = {s: dict(sha256=hashes[s].hexdigest(), bytes=lengths[s],
                             emitted_bytes=emitted[s],
                             digest_only=(s == 'stdout' and digest_stdout))
                     for s in hashes}
            if incomplete:
                self.log.incomplete = True
            result = Result(cid, proc.returncode if proc else None, bytes(capture['stdout']),
                            bytes(capture['stderr']), duration, timeout_hit, not incomplete,
                            infos['stdout'], infos['stderr'])
            end = dict(command_id=cid, returncode=result.returncode, duration_seconds=duration,
                       timeout=timeout_hit, output_complete=not incomplete, output=infos,
                       context=context)
            if pending_error:
                end['error'] = f'{type(pending_error).__name__}: {pending_error}'[:2048]
            if incomplete:
                end['partial_tail_base64'] = {s: base64.b64encode(tails[s]).decode() for s in tails}
            # Reserve is used only when a logging failure would otherwise lose
            # the final command outcome. Remaining execution is terminated.
            self.log.emit('command_end', terminal=isinstance(pending_error, LogLimit), **end)
        if pending_error:
            raise pending_error
        if timeout_hit or incomplete or result.returncode != 0:
            raise CommandError('command failed, timed out, or has incomplete output', result)
        return result

    @staticmethod
    def _kill(proc):
        # Kill the original process group even if its leader already exited;
        # compiler children may still hold output pipes or consume CPU.
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass


class ISAScanner:
    """Hashes are handled by Commands; retain only mnemonic counts/eight examples."""
    BMI = re.compile(r'^(andn|bextr|blsi|blsmsk|blsr|bzhi|pdep|pext|mulx|rorx|sarx|shlx|shrx)[bwlq]?$')
    def __init__(self):
        self.pending = b''
        self.counts = collections.Counter()
        self.examples = []
        self.instructions = 0
        self.rep_bsf_aliases = 0

    def __call__(self, chunk):
        self.pending += chunk
        lines = self.pending.split(b'\n')
        self.pending = lines.pop()
        require(len(self.pending) < 16384, 'Unexpected unbounded objdump line')
        for line in lines:
            m = re.match(rb'^\s*[0-9a-f]+:\s+([a-zA-Z0-9.]+)\s*(.*)$', line)
            if not m:
                continue
            self.instructions += 1
            op = m[1].decode('ascii', 'replace').lower()
            body = m[2].decode('ascii', 'replace').lower()
            reason = None
            if re.fullmatch(r'tzcnt[bwlq]?',op):
                self.rep_bsf_aliases += 1
            if (op.startswith('v') and op not in ('verr', 'verw')) or re.search(r'%[yz]mm\d', body):
                reason = 'avx_or_avx512'
            if self.BMI.match(op):
                reason = 'bmi_or_bmi2'
            if reason:
                self.counts[reason] += 1
                if len(self.examples) < 8:
                    self.examples.append(line.decode('utf8', 'replace')[:512])

    def report(self):
        if self.pending:
            self(b'\n')
        return dict(instructions=self.instructions, forbidden_counts=dict(self.counts),
                    bounded_examples=self.examples,rep_bsf_tzcnt_encoding_count=self.rep_bsf_aliases,
                    compatibility_note='REP-BSF may disassemble as TZCNT. GCC may emit this backward-compatible encoding for nonzero ctz; the mnemonic alone is not proof of a BMI requirement. Zero-input semantics remain a correctness-gate concern.')


def elf_sections(path):
    """Hash each ELF64 little-endian section without external dumps or large ASM."""
    sections = []
    with Path(path).open('rb') as f:
        header = f.read(64)
        require(header[:6] == b'\x7fELF\x02\x01', 'Expected ELF64 little-endian binary')
        shoff = struct.unpack_from('<Q', header, 40)[0]
        entsize, count, stridx = struct.unpack_from('<HHH', header, 58)
        require(entsize >= 64 and 0 < count < 1024 and stridx < count,
                'Unexpected ELF section table')
        table = []
        for i in range(count):
            f.seek(shoff+i*entsize)
            table.append(struct.unpack('<IIQQQQIIQQ', f.read(64)))
        names = table[stridx]
        require(names[5] < 1 << 20, 'Unexpected section-name table size')
        f.seek(names[4]); strings = f.read(names[5])
        for s in table:
            name = strings[s[0]:].split(b'\0', 1)[0].decode('ascii', 'replace')
            h = hashlib.sha256()
            if s[1] != 8:  # SHT_NOBITS has logical bytes but no on-disk content.
                f.seek(s[4]); left = s[5]
                while left:
                    chunk = f.read(min(1 << 20, left))
                    require(bool(chunk), 'Truncated ELF section')
                    h.update(chunk); left -= len(chunk)
            sections.append(dict(name=name, type=s[1], flags=s[2], size=s[5],
                                 file_bytes=0 if s[1] == 8 else s[5],
                                 sha256=None if s[1] == 8 else h.hexdigest()))
    return sections


def dump_info(path):
    info = file_info(path)
    with Path(path).open('rb') as f:
        raw = f.read(16)
        require(len(raw) == 16, 'Truncated preflight header')
        outputs, finals = struct.unpack('<QQ', raw)
        require(info['bytes'] == 16 + 8*(outputs+finals), 'Invalid preflight byte lengths')
        parts = {}
        for name, count in (('output_array', outputs), ('final_vector', finals)):
            h = hashlib.sha256()
            left = 8*count
            while left:
                chunk = f.read(min(1 << 20, left))
                require(bool(chunk), 'Truncated preflight data')
                h.update(chunk); left -= len(chunk)
            parts[name] = dict(elements=count, bytes=8*count, sha256=h.hexdigest())
    return dict(container=info, **parts, input_stream=file_info(str(path)+'.input'))


def execution_schedule(rows, policy):
    """Cycle outermost. Fixed rotate-then-reverse order, never performance-based."""
    schedule = []
    per_comparison = collections.Counter()
    for cycle in range(4):
        shift = cycle % len(rows)
        order = list(range(shift, len(rows))) + list(range(shift))
        if cycle % 2:
            order.reverse()
        for case_order, comparison_index in enumerate(order):
            row = rows[comparison_index]
            tokens = policy['cycle_tokens_odd' if cycle % 2 else 'cycle_tokens_even']
            for phase_index, token in enumerate(tokens):
                for position, side in enumerate(token):
                    schedule.append(dict(comparison_id=row['id'], comparison_index=comparison_index,
                        cycle_index=cycle, case_order_index=case_order, phase_index=phase_index,
                        phase=token, phase_block_index=4*cycle+phase_index,
                        phase_position=position, phase_pair_index=position//2,
                        phase_pair_position=position%2,
                        measurement_index=per_comparison[row['id']], side=side))
                    per_comparison[row['id']] += 1
    require(all(n == 48 for n in per_comparison.values()), 'Invalid 48-sample schedule')
    return schedule


class Runner:
    def __init__(self, repo, work, plan_path, shard, log):
        self.repo, self.work, self.plan_path = repo, work, plan_path
        self.package = plan_path.parent
        self.shard, self.log = shard, log
        self.plan = json.loads(plan_path.read_text())
        self.rows, self.results, self.failures = [], [], []
        self.binaries, self.bad_binaries = {}, {}
        self.preflights, self.pilots = {}, {}
        self.failed_keys, self.replays = {}, {}
        self.variants, self.sources = {}, {}
        self.summary = None
        self.commands = None
        self.expected_measurements = 0
        self.expected_samples = {}
        self.observed_full_input_keys = set()
        self.actual_head = None
        self.source_manifest_sha256 = None
        self.freeze_verified = False
        self.plan_sha256 = file_info(plan_path)['sha256']
        self.full_oracle_cache, self.full_oracle_failures = {}, {}
        self.full_oracle_binary = None
        self.escape_objects, self.escape_failures = {}, {}
        self.sample_attempts, self.sample_successes = collections.Counter(),collections.Counter()
        self.oracle_counts = collections.Counter()

    def initial_manifest(self):
        # Do not gate on frozen/version/header state before these events exist.
        self.log.emit('started', schema='dwm_phase2_logonly_v1', shard=self.shard,
                      repo=str(self.repo), work=str(self.work), plan=str(self.plan_path),
                      plan_info=file_info(self.plan_path), runner_info=file_info(__file__),
                      python=sys.version, python_executable=sys.executable,
                      observed_affinity=sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None,
                      runner_environment={k: os.environ.get(k) for k in
                        ('RUNNER_OS', 'RUNNER_ARCH', 'ImageOS', 'ImageVersion', 'GITHUB_RUN_ID',
                         'GITHUB_RUN_ATTEMPT', 'GITHUB_SHA', 'GITHUB_JOB')})
        manifest = self.plan.get('package', {}).get('source_sha256', {})
        for rel, expected in sorted(manifest.items()):
            observed, error = None, None
            try:
                safe_relative(rel)
                p = (self.package/rel).resolve()
                require(p.is_relative_to(self.package), 'Package source escapes package')
                observed = file_info(p)
            except Exception as e:
                error = str(e)[:1024]
            self.sources[rel] = observed
            self.log.emit('source_manifest', path=rel, expected_sha256=expected,
                          observed=observed, read_error=error)
        self.source_manifest_sha256 = sha_bytes(canonical(self.sources))
        self.log.emit('source_manifest_end', entries=len(manifest),
                      observed_manifest_sha256=self.source_manifest_sha256)

    def validate_plan(self):
        p = self.plan
        require(p.get('frozen') is True, 'Plan is not frozen; proposal execution prohibited')
        require(re.fullmatch('[0-9a-f]{40}', p.get('baseline', '')),
                'Exact baseline commit is required')
        jobs = p['jobs']
        require(len({x['id'] for x in jobs}) == len(jobs), 'Duplicate shard IDs')
        matches = [j for j in jobs if j['id'] == self.shard]
        require(len(matches) == 1, 'Shard not uniquely specified by frozen plan')
        self.job = matches[0]
        self.job_index = jobs.index(self.job)
        limit = self.job.get('normal_log_limit_bytes', 8 << 20)
        require(TERMINAL_RESERVE*2 <= limit <= 64 << 20, 'Invalid normal log limit')
        self.log.limit = limit
        summary_limit = self.job.get('summary_limit_bytes', 32768)
        require(1024 <= summary_limit <= SUMMARY_HARD_LIMIT, 'Summary must be below 1 MiB')
        require(0 < self.job.get('command_timeout_seconds', 120) <= 240,
                'Every command must have a bounded timeout <=240 seconds')
        require(0 < self.job.get('execution_step_timeout_minutes', 40) <= 45,
                'Shard execution budget must be <=45 minutes')
        rows = p['comparison_rows']
        require(len({r['id'] for r in rows}) == len(rows), 'Duplicate comparison IDs')
        byid = {r['id']: r for r in rows}
        ids = self.job['comparison_ids']
        require(0 < len(ids) <= 4 and len(set(ids)) == len(ids), 'Shard must contain 1..4 distinct pair/cases')
        self.rows = [byid[x] for x in ids]
        require(all(j['flags_order'] in (['native','scalar'],['scalar','native']) for j in jobs),
                'Every frozen shard must execute both flags sequentially')
        require(sum(j['flags_order'][0] == 'native' for j in jobs)*2 == len(jobs),
                'First flag must be globally counterbalanced across frozen shards')
        variants = {'baseline'} | {r[side] for r in self.rows for side in ('a', 'b')}
        paths = p['package']['paths']
        for key in ('bench', 'caller', 'oracle', 'escape', 'full_oracle', 'patches'):
            safe_relative(paths[key])
        source_required = {paths[k] for k in ('bench', 'caller', 'oracle', 'escape', 'full_oracle')}
        source_required.add(p['package'].get('runner', 'runner_draft.py'))
        require((self.package/p['package'].get('runner', 'runner_draft.py')).resolve() == Path(__file__).resolve(),
                'Executing runner is not the frozen package runner')
        for name in sorted(variants):
            require(re.fullmatch('[A-Za-z0-9_]+', name), 'Invalid variant name')
            v = p['variants'][name]
            require(v.get('base') == p['baseline'], 'Variant baseline mismatch')
            require(set(v.get('components', [])) <= OWN_COMPONENTS,
                    'Third-party or unknown component prohibited')
            require(len(v['components']) == len(set(v['components'])), 'Duplicate patch component')
            if name == 'baseline':
                require(not v['components'], 'Baseline must not contain patches')
            for header in PUBLIC_HEADERS:
                require(SHA.fullmatch(v['headers'][header]), f'Missing frozen header hash: {name}/{header}')
            for component in v['components']:
                rel = str(PurePosixPath(paths['patches'])/(component+'.patch'))
                source_required.add(rel)
                require(p['package']['source_sha256'].get(rel) == v['component_patch_sha256'][component],
                        f'Patch manifest mismatch: {component}')
        for rel in source_required:
            require(rel in self.sources, f'Required source missing from manifest: {rel}')
        for rel, expected in p['package']['source_sha256'].items():
            require(SHA.fullmatch(expected) and self.sources[rel] and self.sources[rel]['sha256'] == expected,
                    f'Source hash gate failed: {rel}')
        compiler = p['compiler']
        require(compiler['required_version'] in ('GCC 15.2.0', '15.2.0'), 'Compiler version must be GCC 15.2.0')
        require(tuple(compiler['common_flags']) == REQUIRED_COMMON, 'Unexpected common compiler flags')
        require(compiler['native_flags'] == ['-march=native'], 'Unexpected native flags')
        require(all(f in compiler['scalar_flags'] for f in REQUIRED_SCALAR), 'Scalar flags are insufficient')
        scalar_allowed = set(REQUIRED_SCALAR) | {'-mtune=generic', '-mno-avx512f',
                        '-fno-tree-loop-vectorize', '-fno-tree-slp-vectorize'}
        require(set(compiler['scalar_flags']) <= scalar_allowed, 'Unexpected scalar flag')
        sample = p['sample_policy']
        for key, value in [('target_cpu_seconds', 1.5), ('short_sample_floor_seconds', 1.0),
                           ('cycles', 4), ('pilot_count_per_variant_case_flag', 1),
                           ('pilot_replays', 1), ('preflight_replays', 3)]:
            require(sample[key] == value, f'Unexpected fixed sample policy: {key}')
        require(tuple(sample['cycle_tokens_even']) == CYCLE_TOKENS[0] and
                tuple(sample['cycle_tokens_odd']) == CYCLE_TOKENS[1], 'Unexpected cycle protocol')
        require(sample.get('replay_cap', 4096) == 4096, 'Replay cap must be fixed at 4096')
        require(sample.get('AA_BB_max_multiplicative_deviation',0.005) == 0.005 and
                sample.get('adverse_control_margin',0.01) == 0.01,
                'Descriptive resolution/adverse thresholds changed from reviewed protocol')
        require(sample.get('timed_full_final_state_replays') == 'first only' and
                sample.get('timed_output_hash_each_replay') is True and
                sample.get('all_individual_timed_build_states_verified') is False,
                'Plan claims do not match approved timed verification boundary')
        require(sample.get('case_order_policy', 'rotate_left_by_cycle_then_reverse_odd') ==
                'rotate_left_by_cycle_then_reverse_odd', 'Unexpected case-order policy')
        self.case_names = list(dict.fromkeys(r['case'] for r in self.rows))
        for name in self.case_names:
            c = p['cases'][name]
            require(c['caller'] in CALLERS and c['value_type'] in TYPES, 'Unknown case instantiation')
            require(c['n'] > 0 and c['q'] >= 0 and c['bits'] in (20, 64), 'Invalid case bounds')
            require(c['bits'] <= (32 if '32' in c['value_type'] else 64), 'Type width mismatch')
            require(c['block'] > 0 and c['branch'] > 1 and c['seed'] >= 0, 'Invalid case parameters')
            require(c.get('caller_shape',CALLERS[c['caller']][0]) == CALLERS[c['caller']][0],
                    'Caller shape disagrees with frozen source selection')
            require(c['dist'] in ('full','duplicates','equal','sparse','sorted','narrow20') and
                    c['loc'] in ('random','front','back','local','boundary') and
                    c['cache'] in ('warm','cold'), 'Invalid distribution/locality/cache contract')
            require(c['seed'] == sample['case_seed'], 'Timing seed must be the frozen held-out seed')
            if c['kind'] in ('build', 'rebuild'):
                require(c['q'] == 0, 'Build/rebuild must have q=0')
            if c['caller'] == 'independent_access_only':
                require(c['kind'] == 'access', 'Access-only caller requires access kind')
            if c['caller'] == 'independent_mixed_ordinary':
                require(c['kind'] == 'simplemixed', 'Independent mixed caller requires simplemixed')
        self.variant_names = sorted(variants, key=lambda v: (v != 'baseline', v))
        self.schedule = execution_schedule(self.rows, sample)
        self.expected_measurements = len(self.schedule)*2
        unique_variant_cases = {(r[side],r['case']) for r in self.rows for side in ('a','b')}
        unique_variant_cases.update(('baseline',case) for case in self.case_names)
        count = len(unique_variant_cases)*2
        self.expected_samples = dict(exact_preflight=count,small_vector_oracle_preflight=count,
                                     pilot=count,measurement=self.expected_measurements)
        self.log.emit('frozen_plan_verified', baseline=p['baseline'], shard=self.job,
                      selected_variants=self.variant_names, selected_cases=self.case_names,
                      schedule_sha256=sha_bytes(canonical(self.schedule)),
                      measurement_slots_per_flag=len(self.schedule), cycles=4,
                      case_order_policy='rotate_left_by_cycle_then_reverse_odd',
                      numerical_policy='descriptive_only_no_acceptance_or_rejection')
        self.freeze_verified = True
        for name in self.case_names:
            self.log.emit('case_definition', case_id=name, case=self.plan['cases'][name],
                          case_sha256=sha_bytes(canonical(self.plan['cases'][name])))

    def compare_bytes(self, actual, expected, *, kind, context):
        difference = first_byte_difference(actual,expected)
        if difference is None:
            return
        offset = difference['first_byte_offset']
        if kind == 'input':
            # Input lines: dimensions, initial vector, then one operation/line.
            # Count newlines in bounded chunks; do not dump a huge vector line.
            lines = 0
            with Path(expected).open('rb') as f:
                left = offset
                while left:
                    chunk = f.read(min(1 << 20,left))
                    if not chunk:
                        break
                    lines += chunk.count(b'\n'); left -= len(chunk)
            difference.update(input_line_index=lines,
                              operation_index=lines-2 if lines >= 2 else None)
        elif kind == 'output' and offset >= 16:
            word = (offset-16)//8
            with Path(expected).open('rb') as f:
                prefix = f.read(16)
            if len(prefix) == 16:
                outputs,finals = struct.unpack('<QQ',prefix)
                difference.update(result_word_index=word,
                                  operation_index=word//2 if word < outputs else None,
                                  final_vector_index=word-outputs if word >= outputs else None,
                                  expected_output_words=outputs,expected_final_words=finals)
        self.log.emit('first_byte_mismatch',kind=kind,context=context,**difference)
        raise GateError(f'Exact {kind} bytes differ at offset {offset}; bounded first mismatch logged')

    def recheck_source(self, path):
        rel = str(Path(path).resolve().relative_to(self.package))
        observed = file_info(path)
        require(observed['sha256'] == self.plan['package']['source_sha256'][rel],
                f'Packaged source changed after initial hash gate: {rel}')
        return observed

    def cmd(self, argv, **kwargs):
        kwargs.setdefault('cwd',self.work)
        return self.commands.run(argv, timeout=self.job.get('command_timeout_seconds', 120), **kwargs)

    def setup(self):
        require(not self.work.exists() or not any(self.work.iterdir()),
                'Work directory must be empty; refuse cache reuse or overwrites')
        self.work.mkdir(parents=True, exist_ok=True)
        for d in ('variants', 'bins', 'preflight', 'oracle', 'oracle-cache', 'tmp'):
            (self.work/d).mkdir()
        require(sys.flags.optimize == 0 and not os.environ.get('PYTHONOPTIMIZE'),
                'Python -O/-OO/PYTHONOPTIMIZE prohibited: independent oracle assertions must execute')
        self.log.emit('python_assertion_gate',optimization_level=sys.flags.optimize,
                      PYTHONOPTIMIZE=os.environ.get('PYTHONOPTIMIZE'),assertions_enabled=True)
        require(sys.byteorder == 'little' and hasattr(os, 'sched_setaffinity'),
                'This qualification runner requires Linux little-endian with CPU affinity')
        affinity = sorted(os.sched_getaffinity(0))
        require(bool(affinity), 'No available CPU')
        self.cpu = affinity[0]
        os.sched_setaffinity(0, {self.cpu})
        require(os.sched_getaffinity(0) == {self.cpu}, 'CPU pinning failed')
        env = os.environ.copy()
        removed = []
        for key in list(env):
            if key.startswith('DWM_') or key in ('LD_PRELOAD', 'LD_LIBRARY_PATH', 'GCC_EXEC_PREFIX',
                    'COMPILER_PATH', 'LIBRARY_PATH', 'CPATH', 'CPLUS_INCLUDE_PATH', 'C_INCLUDE_PATH'):
                removed.append(key); env.pop(key)
        env.update(LC_ALL='C', LANG='C', TZ='UTC', TMPDIR=str(self.work/'tmp'))
        internal_minutes = self.job.get('execution_step_timeout_minutes', 40) - 1
        require(internal_minutes > 0, 'Step budget must leave a one-minute shutdown reserve')
        self.log.emit('execution_budget', internal_minutes=internal_minutes, shutdown_reserve_seconds=60)
        deadline = time.monotonic()+60*internal_minutes
        self.commands = Commands(self.log, env, deadline)
        self.log.emit('affinity', pinned_cpu=self.cpu, actual=sorted(os.sched_getaffinity(0)),
                      removed_environment_keys=sorted(removed), environment_policy='inherited_except_listed')
        tools = {}
        for name in ('g++-15', 'git', 'patch', 'taskset', 'objdump', 'size'):
            path = shutil.which(name)
            require(path is not None, f'Required tool unavailable: {name}')
            real = Path(path).resolve()
            tools[name] = str(real)
            self.log.emit('tool_provenance', name=name, lookup_path=path, resolved_path=str(real), **file_info(real))
        self.tools = tools
        self.cxx = tools['g++-15']
        version = self.cmd([self.cxx, '-dumpfullversion', '-dumpversion'], context={'gate':'compiler_version'}).stdout.decode().strip()
        require(version == '15.2.0', f'Exact GCC 15.2.0 required, got {version!r}')
        self.cmd([self.cxx, '--version'], context={'gate':'compiler_banner'})
        for prog in ('cc1plus', 'as', 'ld'):
            p = self.cmd([self.cxx, '-print-prog-name='+prog], context={'gate':'toolchain_path', 'tool':prog}).stdout.decode().strip()
            resolved = Path(p).resolve() if '/' in p else Path(shutil.which(p) or p).resolve()
            require(resolved.is_file(), f'Cannot resolve toolchain program: {prog}')
            self.log.emit('toolchain_component', name=prog, path=str(resolved), **file_info(resolved))
            if prog != 'cc1plus':
                self.cmd([resolved, '--version'], context={'gate':'toolchain_banner', 'tool':prog})
        self.cmd(['uname', '-a'], context={'gate':'host'})
        self.cmd(['lscpu'], context={'gate':'cpu'})
        cpuinfo = Path('/proc/cpuinfo').read_bytes()
        selected = None
        for entry in cpuinfo.decode(errors='replace').strip().split('\n\n'):
            vals = dict((line.split(':',1)[0].strip(), line.split(':',1)[1].strip()) for line in entry.splitlines() if ':' in line)
            if vals.get('processor') == str(self.cpu):
                selected = vals; break
        require(selected is not None, 'Pinned CPU not present in /proc/cpuinfo')
        self.log.emit('actual_cpu', cpu=self.cpu, proc_cpuinfo_sha256=sha_bytes(cpuinfo),
                      fields={k:selected.get(k) for k in ('vendor_id','model name','cpu family','model','stepping','microcode','flags')})
        self.actual_head = self.cmd([tools['git'], '-C', self.repo, 'rev-parse', 'HEAD'],
                                    context={'gate':'repository_head'}).stdout.decode().strip()
        self.log.emit('run_identity', head=self.actual_head, baseline=self.plan['baseline'],
                      plan_sha256=file_info(self.plan_path)['sha256'],
                      source_manifest_sha256=self.source_manifest_sha256,
                      github_run_id=os.environ.get('GITHUB_RUN_ID'),
                      github_run_attempt=os.environ.get('GITHUB_RUN_ATTEMPT'))
        commit = self.cmd([tools['git'], '-C', self.repo, 'rev-parse', self.plan['baseline']+'^{commit}'],
                          context={'gate':'baseline_commit'}).stdout.decode().strip()
        require(commit == self.plan['baseline'], 'Baseline object does not match exact commit')
        archive = self.work/'baseline.tar'
        self.cmd([tools['git'], '-C', self.repo, 'archive', '--format=tar', '-o', archive,
                  self.plan['baseline'], '--', 'components', 'data_structure'], context={'gate':'exact_baseline_export'})
        self.log.emit('baseline_archive', **file_info(archive), baseline=commit)
        baseline = self.work/'variants'/'baseline'; baseline.mkdir()
        with tarfile.open(archive, 'r:') as tar:
            for member in tar:
                safe_relative(member.name.rstrip('/'))
                require(member.isfile() or member.isdir(), 'Archive contains a link or special file')
                dest = baseline/member.name
                require(dest.resolve().is_relative_to(baseline.resolve()), 'Archive path escapes baseline')
                if member.isdir():
                    dest.mkdir(parents=True, exist_ok=True)
                else:
                    dest.parent.mkdir(parents=True, exist_ok=True)
                    with tar.extractfile(member) as src, dest.open('wb') as out:
                        shutil.copyfileobj(src, out)
        archive.unlink()
        self.variants['baseline'] = baseline
        for name in self.variant_names:
            dest = baseline if name == 'baseline' else self.work/'variants'/name
            if name != 'baseline':
                shutil.copytree(baseline, dest)
                for index, component in enumerate(self.plan['variants'][name]['components']):
                    patch = self.package/self.plan['package']['paths']['patches']/(component+'.patch')
                    # Only own-library target headers may be modified by this runner.
                    self.recheck_source(patch)
                    text = patch.read_text()
                    targets = re.findall(r'^\+\+\+ b/([^\t\n]+)', text, re.M)
                    original = re.findall(r'^--- a/([^\t\n]+)', text, re.M)
                    require(targets and original == targets and set(targets) <= set(PUBLIC_HEADERS),
                            'Patch modifies a non-target path or creates/deletes a file')
                    self.cmd([tools['patch'], '--batch', '--forward', '--fuzz=0', '--no-backup-if-mismatch',
                              '-d', dest, '-p1', '-i', patch],
                             context={'gate':'component_patch', 'variant':name, 'component':component, 'component_index':index})
                self.variants[name] = dest
            observed = {h:file_info(dest/h) for h in PUBLIC_HEADERS}
            self.log.emit('variant_headers', variant=name, composition=self.plan['variants'][name]['components'],
                          observed=observed, expected=self.plan['variants'][name]['headers'])
            require(all(observed[h]['sha256'] == self.plan['variants'][name]['headers'][h] for h in PUBLIC_HEADERS),
                    f'Frozen variant header mismatch: {name}')
        # Full own-library exported include tree manifest, chunked per file.
        for path in sorted(baseline.rglob('*')):
            if path.is_file():
                self.log.emit('baseline_source', path=str(path.relative_to(baseline)), **file_info(path))

    def verify_dependencies(self, depfile, *, variant=None, label):
        # GCC -MMD gives actual user-header dependencies of this translation
        # unit, rather than a predicted textual #include closure. System headers
        # are intentionally not claimed by this manifest.
        raw = depfile.read_text().replace('\\\n',' ')
        match = re.search(r'(?<!\\):',raw)
        require(match is not None,'Compiler dependency file has no target separator')
        lexer = shlex.shlex(raw[match.end():],posix=True)
        lexer.whitespace_split=True; lexer.commenters=''
        deps = []
        for token in lexer:
            token = token.replace('$$','$')
            path = Path(token)
            if not path.is_absolute():
                path = self.work/path
            path = path.resolve(strict=True)
            if path not in deps:
                deps.append(path)
        actual = {}
        for header in PUBLIC_HEADERS:
            found = [p for p in deps if p.as_posix().endswith('/'+header)]
            actual[header] = [dict(path=str(p),**file_info(p)) for p in found]
        self.log.emit('compiler_dependencies',label=label,variant=variant,
                      dependency_file=file_info(depfile),resolved_dependencies=[str(p) for p in deps],
                      product_headers=actual,system_headers_included=False)
        if variant is None:
            require(not any(actual.values()),'Independent translation unit includes a product header')
            require(not any(p.is_relative_to(self.work/'variants') for p in deps),
                    'Independent translation unit depends on a materialized product variant')
        else:
            root = self.variants[variant].resolve()
            for header,found in actual.items():
                require(len(found)==1,'Actual dependency does not uniquely contain '+header)
                observed = found[0]
                path = Path(observed['path'])
                require(path.is_relative_to(root) and path == (root/header).resolve() and
                        observed['sha256'] == self.plan['variants'][variant]['headers'][header],
                        'Actual compiler dependency resolves outside selected variant or has wrong bytes: '+header)
        return dict(dependency_file=file_info(depfile),product_headers=actual,
                    resolved_dependencies=[str(p) for p in deps])

    def escape_object(self,target):
        if target in self.escape_failures:
            raise GateError('Escape compilation previously failed; not retried: '+self.escape_failures[target])
        if target in self.escape_objects:
            return self.escape_objects[target]
        source = self.package/self.plan['package']['paths']['escape']
        path = self.work/'bins'/('escape-'+target+'.o')
        depfile = path.with_suffix('.d')
        try:
            self.recheck_source(source)
            command = [self.cxx,*self.flags(target),'-MMD','-MF',depfile,'-c',source,'-o',path]
            self.cmd(command,context={'stage':'escape_compile','flag':target})
            deps = self.verify_dependencies(depfile,label='escape-'+target)
            self.log.emit('escape_object',flag=target,path=str(path),**file_info(path),
                          dependencies=deps,command=list(map(str,command)),lto=False)
            self.escape_objects[target] = path
            return path
        except Exception as e:
            self.escape_failures[target] = str(e)[:1024]
            raise

    def build_full_oracle(self):
        source = self.package/self.plan['package']['paths']['full_oracle']
        self.recheck_source(source)
        path = self.work/'bins'/'full-vector-oracle'
        flags = ['-O2','-std=gnu++23','-march=x86-64']
        depfile = path.with_suffix('.d')
        command = [self.cxx,*flags,'-MMD','-MF',depfile,source,'-o',path]
        self.cmd(command,context={'stage':'full_oracle_compile'})
        deps = self.verify_dependencies(depfile,label='independent-full-vector-oracle')
        scanner = ISAScanner()
        assembly = self.cmd([self.tools['objdump'],'-d','--no-show-raw-insn',path],
                           digest_stdout=True,observer=scanner,context={'stage':'full_oracle_disassembly'})
        isa = scanner.report()
        sections = elf_sections(path)
        self.cmd([self.tools['size'],'-A','-d',path],context={'stage':'full_oracle_section_sizes'})
        self.log.emit('full_oracle_binary',path=str(path),**file_info(path),
                      compiler=self.cxx,flags=flags,source=file_info(source),
                      production_headers_linked=False,actual_dependencies=deps,lto=False,
                      command=list(map(str,command)),disassembly=assembly.stdout_info,
                      isa=isa,sections=sections,section_manifest_sha256=sha_bytes(canonical(sections)))
        require(isa['instructions'] > 0 and not isa['forbidden_counts'],
                'Portable full-oracle ISA disassembly gate failed')
        self.full_oracle_binary = path

    def full_oracle(self, dump, c, context):
        require(self.full_oracle_binary is not None,'Full-input oracle binary is unavailable')
        actual_input = Path(str(dump)+'.input')
        input_info = file_info(actual_input)
        # Type includes semantic interpretation: build/rebuild ignore operation
        # records, while normal cases execute them. No type or input aliasing.
        type_identity = dict(value_type=c['value_type'],bits=c['bits'],kind=c['kind'])
        type_hash = sha_bytes(canonical(type_identity))
        key = sha_bytes(canonical(dict(input_sha256=input_info['sha256'],input_bytes=input_info['bytes'],type_sha256=type_hash)))
        self.observed_full_input_keys.add(key)
        if key in self.full_oracle_failures:
            raise GateError('Full-input oracle previously failed; no fallback/retry: '+self.full_oracle_failures[key])
        try:
            if key in self.full_oracle_cache:
                cached = self.full_oracle_cache[key]
                require(type_identity == cached['type'],'Full oracle cache type identity mismatch')
                self.compare_bytes(actual_input,cached['input'],kind='input',context=dict(boundary='full_oracle_cache',**context))
                self.compare_bytes(dump,cached['output'],kind='output',context=dict(boundary='full_oracle_cache',**context))
                self.oracle_counts['full_cache_hits'] += 1
                self.log.emit('full_oracle_cache_hit',cache_key=key,context=context,
                              type_identity=type_identity,type_sha256=type_hash,input=input_info,
                              verified_output=file_info(cached['output']),
                              exact_input_and_output_bytes=True,
                              original_oracle_command_id=cached['command_id'])
            else:
                self.oracle_counts['full_verifier_attempts'] += 1
                result = self.commands.run([self.tools['taskset'],'-c',str(self.cpu),self.full_oracle_binary,dump,c['kind']],
                                           timeout=min(120,self.job['command_timeout_seconds']),cwd=self.work,
                                           context=dict(stage='independent_full_vector_oracle',cache_key=key,**context))
                answer = json.loads(result.stdout.decode().strip())
                require(answer.get('oracle') == 'full_vector' and answer.get('status') == 'pass' and
                        answer.get('n') == c['n'] and answer.get('q') == c['q'],
                        'Full vector oracle did not report a matching successful verification')
                dest = self.work/'oracle-cache'/key
                shutil.copyfile(dump,dest)
                shutil.copyfile(actual_input,str(dest)+'.input')
                self.compare_bytes(dest,dump,kind='output',context=dict(boundary='oracle_cache_copy',**context))
                self.compare_bytes(str(dest)+'.input',actual_input,kind='input',context=dict(boundary='oracle_cache_copy',**context))
                self.oracle_counts['full_verifier_successes'] += 1
                self.full_oracle_cache[key] = dict(type=type_identity,output=dest,
                    input=Path(str(dest)+'.input'),command_id=result.command_id)
                self.log.emit('full_oracle_verified',cache_key=key,context=context,
                              type_identity=type_identity,type_sha256=type_hash,input=input_info,
                              verified_output=file_info(dest),oracle_result=answer,
                              oracle_command_id=result.command_id,oracle_binary=file_info(self.full_oracle_binary),
                              expected_computations_for_this_key=1)
            self.oracle_counts['full_preflight_verifications'] += 1
            return key
        except Exception as e:
            self.full_oracle_failures[key] = f'{type(e).__name__}: {e}'[:1024]
            raise

    def flags(self, target):
        return self.plan['compiler']['common_flags'] + self.plan['compiler'][target+'_flags']

    def check_isa(self, target):
        result = self.cmd([self.cxx, *self.flags(target), '-dM', '-E', '-x', 'c++', '-'],
                          context={'gate':'predefined_macros', 'flag':target})
        macros = {}
        for line in result.stdout.decode().splitlines():
            m = re.match(r'#define (\w+)(?:\s+(.*))?$', line)
            if m:
                macros[m[1]] = m[2] or ''
        require([macros.get(x) for x in ('__GNUC__','__GNUC_MINOR__','__GNUC_PATCHLEVEL__')] == ['15','2','0']
                and '__clang__' not in macros, 'Compiler macro identity is not GCC 15.2.0')
        require('NDEBUG' not in macros, 'Assertions must remain enabled')
        relevant = sorted(set(ABSENT_SCALAR) | set(self.plan['compiler'].get('log_macro_probe', [])))
        self.log.emit('isa_macros', flag=target, flags=self.flags(target), macros={k:macros.get(k) for k in relevant},
                      full_macro_sha256=result.stdout_info['sha256'])
        if target == 'scalar':
            require(not any(k in macros for k in ABSENT_SCALAR), 'Scalar ISA macro gate failed')

    def binary(self, name, c, target):
        shape, source_key = CALLERS[c['caller']]
        key = (name, target, c['bits'], c['block'], c['branch'], c['value_type'], shape)
        if key in self.bad_binaries:
            raise GateError('Previously failed binary, not retried: '+self.bad_binaries[key])
        if key in self.binaries:
            return self.binaries[key]
        label = '-'.join(map(str, key)); path = self.work/'bins'/label
        try:
            source = self.package/self.plan['package']['paths'][source_key]
            escape = self.package/self.plan['package']['paths']['escape']
            self.recheck_source(source); self.recheck_source(escape)
            extra = [f'-DBITS={c["bits"]}', f'-DBLOCK={c["block"]}', f'-DBRANCH={c["branch"]}',
                     f'-DVALUE_TYPE={TYPES[c["value_type"]]}', f'-DCALLER_SHAPE={shape}']
            obj = path.with_suffix('.o'); depfile = path.with_suffix('.d')
            escape_obj = self.escape_object(target)
            command = [self.cxx,*self.flags(target),*extra,'-I',self.variants[name],
                       '-MMD','-MF',depfile,'-c',source,'-o',obj]
            self.cmd(command,context={'stage':'benchmark_compile','binary_id':label,'variant':name,'flag':target})
            deps = self.verify_dependencies(depfile,variant=name,label=label)
            link = [self.cxx,*self.flags(target),obj,escape_obj,'-o',path]
            self.cmd(link,context={'stage':'benchmark_link','binary_id':label,'variant':name,'flag':target})
            scanner = ISAScanner()
            disassembly = self.cmd([self.tools['objdump'], '-d', '--no-show-raw-insn', path], digest_stdout=True,
                                  observer=scanner, context={'stage':'binary_disassembly', 'binary_id':label})
            isa = scanner.report()
            sections = elf_sections(path)
            self.cmd([self.tools['size'], '-A', '-d', path], context={'stage':'section_sizes', 'binary_id':label})
            metadata = dict(binary_id=label, path=str(path), **file_info(path), variant=name, flag=target,
                            command=list(map(str,command)),link_command=list(map(str,link)),
                            actual_dependencies=deps,benchmark_object=file_info(obj),escape_object=file_info(escape_obj),
                            source_info=file_info(source),escape_info=file_info(escape),
                            disassembly=disassembly.stdout_info, isa=isa, sections=sections,
                            section_manifest_sha256=sha_bytes(canonical(sections)),
                            header_sha256=self.plan['variants'][name]['headers'], lto=False,
                            escape_boundary='external separate translation unit, no LTO')
            self.log.emit('binary_provenance', **metadata)
            require(isa['instructions'] > 0, 'No decoded instructions in objdump ISA gate')
            if target == 'scalar':
                require(not isa['forbidden_counts'], 'Scalar disassembly ISA gate failed')
            self.binaries[key] = metadata
            return metadata
        except Exception as e:
            self.bad_binaries[key] = f'{type(e).__name__}: {e}'[:1024]
            raise

    def sample(self, name, case_name, c, target, reps, stage, *, dump=None, validate=False,
               expected=None, schedule=None):
        binary = self.binary(name, c, target)
        command = [self.tools['taskset'], '-c', str(self.cpu), binary['path'], c['kind'], str(c['n']),
                   str(c['q']), str(c['seed']), c['dist'], c['loc'], str(reps), c['cache'],
                   str(dump) if dump else '-']
        delta = {}
        if validate:
            delta['DWM_VALIDATE_REPLAYS'] = '1'
        if expected:
            delta.update(DWM_EXPECTED_HASH=expected['hash'], DWM_EXPECTED_FINAL_HASH=expected['finalhash'])
        ctx = dict(stage=stage, variant=name, case_id=case_name, flag=target, **(schedule or {}))
        self.sample_attempts[stage] += 1
        try:
            r = self.cmd(command, env_delta=delta, context=ctx)
        except CommandError:
            if dump:
                partial = {}
                for part in (Path(dump),Path(str(dump)+'.input')):
                    if part.is_file():
                        partial[part.name] = file_info(part)
                self.log.emit('partial_preflight_files',context=ctx,files=partial,
                              parameters=c,note='Failure evidence only; not a passed byte comparison')
            raise
        lines = r.stdout.decode().strip().splitlines()
        require(len(lines) == 1, 'Benchmark must emit exactly one JSON timing record')
        data = json.loads(lines[0])
        require(data['reps'] == reps, 'Harness replay count mismatch')
        require(all(isinstance(data[k], (float,int)) and math.isfinite(data[k]) and data[k] >= 0
                    for k in ('cpu','wall','buildcpu','memory','rss_kib')), 'Invalid benchmark metric')
        require(data['cpu'] > 0 and data['wall'] > 0, 'Non-positive timing result')
        require(all(isinstance(data[k], str) and data[k].isdigit() for k in ('hash','finalhash')),
                'Invalid output/final digest')
        if expected:
            require(all(data[k] == expected[k] for k in ('hash','finalhash')),
                    'Returned timing digests differ from exact preflight')
        record = dict(**ctx, **data, command_id=r.command_id, process_wall_seconds=r.duration,
                      binary_id=binary['binary_id'], binary_sha256=binary['sha256'],
                      case_sha256=sha_bytes(canonical(c)), fresh_object_each_replay=True,
                      validation='full_output_and_final_bytes_all_3_replays' if validate else
                                 'all_replay_output_digests_and_first_final_vector_only',
                      validation_outside_timer=True,
                      build_state_claim='preflight_full_readback; timed_first_final_only' if c['kind'] in ('build','rebuild') else None)
        if expected:
            record['expected_preflight'] = dict(
                input_stream=expected['dump']['input_stream'],
                full_output_container_sha256=expected['dump']['container']['sha256'],
                output_hash=expected['hash'], first_final_hash=expected['finalhash'],
                input_provenance='same frozen generator/binary/parameters; timed input not re-dumped')
        if dump:
            record['dump'] = dump_info(dump)
            require(record['dump']['output_array']['elements'] == 2*c['q'], 'Unexpected output count')
        self.log.emit('sample', **record)
        self.sample_successes[stage] += 1
        if stage == 'measurement':
            self.results.append(record)
        return record

    def failure(self, stage, error, **ctx):
        info = dict(stage=stage, error_type=type(error).__name__, error=str(error)[:2048], **ctx)
        if isinstance(error, CommandError):
            info.update(command_id=error.result.command_id, timed_out=error.result.timed_out,
                        returncode=error.result.returncode)
        self.failures.append(info)
        self.log.emit('failure', **info)

    def exact_preflight(self, name, case_name, target):
        key = (name, case_name, target)
        if key in self.failed_keys:
            raise GateError('Previously failed preflight, not retried: '+self.failed_keys[key])
        if key in self.preflights:
            return self.preflights[key]
        c = self.plan['cases'][case_name]
        label = '-'.join(key)
        try:
            # Baseline is always the byte comparator, including a future ablation
            # where neither comparison side is baseline.
            baseline = self.exact_preflight('baseline',case_name,target) if name != 'baseline' else None
            dump = self.work/'preflight'/(label+'.bin')
            r = self.sample(name, case_name, c, target, 3, 'exact_preflight', dump=dump, validate=True)
            if baseline:
                ctx = dict(boundary='candidate_vs_baseline',variant=name,case_id=case_name,flag=target)
                self.compare_bytes(str(dump)+'.input',str(baseline[1])+'.input',kind='input',context=ctx)
                self.compare_bytes(dump,baseline[1],kind='output',context=ctx)
            # Cross-flag exact baseline equality detects any flag-dependent input
            # or behavior before its timings are interpreted.
            other = self.preflights.get(('baseline',case_name,'scalar' if target == 'native' else 'native'))
            if name == 'baseline' and other:
                ctx = dict(boundary='baseline_across_flags',variant=name,case_id=case_name,flag=target)
                self.compare_bytes(str(dump)+'.input',str(other[1])+'.input',kind='input',context=ctx)
                self.compare_bytes(dump,other[1],kind='output',context=ctx)
            full_oracle_key = self.full_oracle(dump,c,dict(variant=name,case_id=case_name,flag=target))
            small = dict(c, n=min(c['n'], 512), q=min(c['q'], 512),
                         seed=self.plan['sample_policy']['correctness_second_seed'])
            if small['kind'] == 'select-churn':
                small['q'] -= small['q'] % 10
                require(small['n'] >= 2 and small['q'] > 0, 'Small churn oracle requires full ten-operation cycles')
            odump = self.work/'oracle'/(label+'.bin')
            oracle_sample = self.sample(name, case_name, small, target, 3, 'small_vector_oracle_preflight',
                                       dump=odump, validate=True)
            oracle_path = self.package/self.plan['package']['paths']['oracle']
            self.recheck_source(oracle_path)
            require(sys.flags.optimize == 0 and not os.environ.get('PYTHONOPTIMIZE'),
                    'Python assertion optimization changed before oracle')
            self.oracle_counts['small_python_verifier_attempts'] += 1
            self.cmd([sys.executable, oracle_path, odump, c['kind']],
                     context={'stage':'independent_vector_oracle', 'variant':name, 'case_id':case_name, 'flag':target})
            self.oracle_counts['small_python_verifier_successes'] += 1
            self.log.emit('preflight_pass', variant=name, case_id=case_name, flag=target,
                          exact_baseline_bytes=True, complete_input_bytes=True, replays=3,
                          independent_small_vector_oracle=True, small_case=small,
                          independent_full_vector_oracle=True,full_oracle_cache_key=full_oracle_key,
                          exact_dump=r['dump'], small_dump=oracle_sample['dump'],
                          claim='full readback for all three fresh resets, not timed per-replay final validation')
            self.preflights[key] = (r, dump)
            return r, dump
        except Exception as e:
            self.failed_keys[key] = f'{type(e).__name__}: {e}'[:1024]
            raise

    def prepare_flag(self, target):
        case_variants = {}
        for case_name in self.case_names:
            names = {'baseline'} | {r[s] for r in self.rows if r['case'] == case_name for s in ('a','b')}
            case_variants[case_name] = sorted(names,key=lambda n:(n!='baseline',n))
            for name in case_variants[case_name]:
                try:
                    self.exact_preflight(name, case_name, target)
                except (LogLimit, KeyboardInterrupt):
                    raise
                except Exception as e:
                    self.failure('preflight', e, variant=name, case_id=case_name, flag=target)
        # Exactly one pilot per distinct variant/case/flag, no pilot retries or
        # replay retuning. All pilots precede measurements for this flag.
        for case_name, names in case_variants.items():
            pilots = []
            for name in names:
                key = (name,case_name,target)
                if key not in self.preflights:
                    continue
                try:
                    r = self.sample(name,case_name,self.plan['cases'][case_name],target,1,'pilot',
                                    expected=self.preflights[key][0])
                    self.pilots[key] = r; pilots.append(r)
                except (LogLimit, KeyboardInterrupt):
                    raise
                except Exception as e:
                    self.failure('pilot', e, variant=name, case_id=case_name, flag=target)
            if len(pilots) == len(names):
                fastest = min(r['cpu'] for r in pilots)
                raw = max(1,math.ceil(1.5/fastest))
                reps = min(4096,raw)
                self.replays[(case_name,target)] = (reps,raw>4096)
                self.log.emit('replay_policy', case_id=case_name, flag=target, variants=names,
                              pilot_cpu_seconds={r['variant']:r['cpu'] for r in pilots},
                              fastest_pilot_seconds=fastest, target_cpu_seconds=1.5,
                              requested_replays=raw, replays=reps, cap=4096, capped=raw>4096,
                              same_replays_for_every_AA_BB_and_cross_leg=True,
                              pilot_retries=0, adaptive_retuning=False)
            else:
                self.log.emit('replay_policy_unavailable', case_id=case_name, flag=target,
                              required_variants=names, available_variants=[r['variant'] for r in pilots],
                              interpretation='incomplete, not acceptance or rejection')

    def measure_flag(self, target):
        disabled = {}
        for slot_index, slot in enumerate(self.schedule):
            row = self.rows[slot['comparison_index']]
            case_name = row['case']
            name = row['a' if slot['side']=='A' else 'b']
            ctx = dict(slot, slot_index=slot_index, comparison_stage=row['stage'], role=row['role'])
            reason = disabled.get(row['id'])
            if (case_name,target) not in self.replays:
                reason = reason or 'no complete common-case pilot/preflight set'
            if reason:
                self.log.emit('measurement_unavailable', flag=target, variant=name, case_id=case_name,
                              reason=reason, **ctx)
                continue
            try:
                reps, capped = self.replays[(case_name,target)]
                self.sample(name,case_name,self.plan['cases'][case_name],target,reps,'measurement',
                            expected=self.preflights[(name,case_name,target)][0], schedule=ctx)
            except (LogLimit, KeyboardInterrupt):
                raise
            except Exception as e:
                self.failure('measurement',e,flag=target,variant=name,case_id=case_name,**ctx)
                disabled[row['id']] = 'a predetermined measurement failed; no retry or replacement'

    def summarize_pair(self, row, target):
        rows = [r for r in self.results if r['comparison_id']==row['id'] and r['flag']==target]
        groups = collections.defaultdict(list)
        for r in rows:
            groups[(r['cycle_index'],r['phase_index'],r['phase'])].append(r)
        aa, bb, cross = [], [], []
        for (_,_,phase), group in sorted(groups.items()):
            group.sort(key=lambda r:r['phase_position'])
            if len(group) != len(phase):
                continue
            if phase in ('AA','BB'):
                (aa if phase == 'AA' else bb).append(dict(
                    cycle_index=group[0]['cycle_index'],phase_block_index=group[0]['phase_block_index'],
                    signed_log_drift=math.log(group[1]['cpu']/group[0]['cpu']),
                    second_over_first=group[1]['cpu']/group[0]['cpu']))
            else:
                a = [r['cpu'] for r in group if r['side']=='A']
                b = [r['cpu'] for r in group if r['side']=='B']
                contrast = statistics.mean(map(math.log,b))-statistics.mean(map(math.log,a))
                cross.append(dict(cycle_index=group[0]['cycle_index'],
                    phase_block_index=group[0]['phase_block_index'],phase=phase,
                    D_log_B_over_A=contrast,latency_ratio_B_over_A=math.exp(contrast),
                    raw_measurement_indexes=[r['measurement_index'] for r in group]))
        calibration = aa+bb
        envelope = max((abs(x['signed_log_drift']) for x in calibration),default=None)
        reasons = []
        if len(rows) != 48 or len(aa) != 4 or len(bb) != 4 or len(cross) != 8:
            reasons.append('incomplete_fixed_schedule')
        if any(r['cpu'] < 1.0 for r in rows):
            reasons.append('sample_below_1_second_floor')
        if envelope is None or envelope > math.log(1.005):
            reasons.append('AA_BB_empirical_envelope_exceeds_0.5_percent_or_missing')
        if self.replays.get((row['case'],target),(None,False))[1]:
            reasons.append('replay_cap_reached')
        if self.log.incomplete:
            reasons.append('incomplete_normal_log')
        ratios = [x['latency_ratio_B_over_A'] for x in cross]
        ratio_summary = None if not ratios else dict(median=statistics.median(ratios),minimum=min(ratios),maximum=max(ratios),blocks=cross)
        return dict(comparison_id=row['id'], stage=row['stage'], case_id=row['case'], role=row['role'],
                    a=row['a'],b=row['b'],flag=target,measurements=len(rows),expected_measurements=48,
                    AA=aa,BB=bb,cross_block_contrasts=ratio_summary,
                    ratio_direction='B/A CPU latency; <1 means B faster',
                    statistical_unit='eight complete four-sample ABBA/BAAB block contrasts, not 16 independent pairs',
                    empirical_calibration_max_multiplicative_deviation=None if envelope is None else math.expm1(envelope),
                    min_sample_cpu_seconds=min((r['cpu'] for r in rows),default=None),
                    status='inconclusive' if reasons else 'descriptive_complete',reasons=reasons,
                    adverse_control_beyond_1_percent=(row['role'] in ('adverse_control','independent_caller_control','short_query_control')
                       and bool(ratios) and statistics.median(ratios) > 1.01),
                    inference='No confidence interval or acceptance/rejection; 1-3% adverse observations remain explicit.')

    def finish(self, fatal=None):
        if self.freeze_verified:
            changed = []
            for rel,expected in self.plan['package']['source_sha256'].items():
                try:
                    if file_info(self.package/rel)['sha256'] != expected:
                        changed.append(rel)
                except OSError:
                    changed.append(rel)
            try:
                if file_info(self.plan_path)['sha256'] != self.plan_sha256:
                    changed.append('<frozen plan>')
            except OSError:
                changed.append('<frozen plan>')
            if changed:
                fatal = fatal or 'Frozen sources changed during execution'
            self.log.emit('source_postflight',terminal=True,changed=changed,unchanged=not changed)
        summaries = [self.summarize_pair(row,target) for target in ('native','scalar') for row in self.rows]
        count_mismatches = []
        for stage,expected in self.expected_samples.items():
            attempts,successes = self.sample_attempts[stage],self.sample_successes[stage]
            if attempts != expected or successes != expected:
                count_mismatches.append(dict(stage=stage,expected=expected,attempts=attempts,successes=successes))
        expected_preflights = self.expected_samples.get('exact_preflight',0)
        oracle_expected = dict(small_python_verifier_attempts=expected_preflights,
            small_python_verifier_successes=expected_preflights,
            full_preflight_verifications=expected_preflights,
            full_verifier_attempts=len(self.observed_full_input_keys),
            full_verifier_successes=len(self.observed_full_input_keys),
            full_cache_hits=max(0,expected_preflights-len(self.observed_full_input_keys)))
        for stage,expected in oracle_expected.items():
            if self.oracle_counts[stage] != expected:
                count_mismatches.append(dict(stage=stage,expected=expected,actual=self.oracle_counts[stage]))
        if len(self.preflights) != expected_preflights:
            count_mismatches.append(dict(stage='fully_gated_preflight_keys',expected=expected_preflights,actual=len(self.preflights)))
        if len(self.pilots) != self.expected_samples.get('pilot',0):
            count_mismatches.append(dict(stage='pilot_keys',expected=self.expected_samples.get('pilot',0),actual=len(self.pilots)))
        if len(self.full_oracle_cache) != len(self.observed_full_input_keys) or self.full_oracle_failures:
            count_mismatches.append(dict(stage='independent_full_oracle_keys',
                expected=len(self.observed_full_input_keys),successful=len(self.full_oracle_cache),failed=len(self.full_oracle_failures)))
        incomplete = bool(fatal or self.failures or self.log.incomplete or count_mismatches or len(self.results)!=self.expected_measurements)
        summary = dict(schema='dwm_phase2_summary_v1', shard=self.shard,
                       status='incomplete' if incomplete else ('inconclusive' if any(s['reasons'] for s in summaries) else 'descriptive_complete'),
                       fatal=fatal, expected_measurements=self.expected_measurements,
                       completed_measurements=len(self.results), failure_count=len(self.failures),
                       log_complete=not self.log.incomplete, comparisons=summaries,
                       independent_full_oracle_computations=len(self.full_oracle_cache),
                       independent_full_oracle_failed_keys=len(self.full_oracle_failures),
                       expected_benchmark_processes=self.expected_samples,expected_oracle_counts=oracle_expected,
                       count_mismatches=count_mismatches,
                       benchmark_process_attempts=dict(self.sample_attempts),
                       benchmark_process_successes=dict(self.sample_successes),oracle_counts=dict(self.oracle_counts),
                       command_counts_by_stage=dict(self.commands.counts) if self.commands else {},
                       not_established=['Mac compatibility','latest-head CI','external-reference performance',
                                        'merge acceptance','separate full correctness-suite completion'],
                       interpretation='All scheduled adverse rows retained. No reruns/replacements or acceptance decision.')
        raw = canonical(summary)
        limit = min(self.plan.get('jobs',[{}])[0].get('summary_limit_bytes',32768),SUMMARY_HARD_LIMIT)
        if hasattr(self,'job'):
            limit = min(self.job.get('summary_limit_bytes',32768),SUMMARY_HARD_LIMIT)
        if len(raw) > limit:
            summary = dict(schema='dwm_phase2_summary_v1',shard=self.shard,status='incomplete',
                           error='Summary exceeded frozen byte budget; details remain in JSONL',
                           complete_summary_sha256=sha_bytes(raw),complete_summary_bytes=len(raw))
            raw = canonical(summary); incomplete=True
        path = os.environ.get('GITHUB_STEP_SUMMARY')
        if path:
            # This is an allowed normal workflow output, never an artifact.
            try:
                with open(path,'w',encoding='utf8') as f:
                    f.write(raw.decode()+'\n')
            except OSError as e:
                incomplete = True
                summary['status'] = 'incomplete'
                summary['summary_write_error'] = str(e)[:1024]
                raw = canonical(summary)
                self.log.emit('summary_write_failure',terminal=True,error=str(e)[:1024])
        # Chunk rather than emit a potentially oversized JSONL record.
        for offset in range(0,len(raw),8192):
            self.log.emit('summary_chunk',terminal=True,offset=offset,encoding='base64',
                          data=base64.b64encode(raw[offset:offset+8192]).decode())
        self.log.emit('completed',terminal=True,summary_sha256=sha_bytes(raw),summary_bytes=len(raw),
                      status=summary['status'],jsonl_bytes_before_completion=self.log.bytes,
                      no_artifacts_or_cache_uploads=True,
                      prior_event_count=self.log.seq,prior_event_chain_sha256=self.log.chain,
                      final_event_count_including_this=self.log.seq+1,
                      expected_benchmark_processes=self.expected_samples,expected_oracle_counts=oracle_expected,
                      count_mismatches=count_mismatches,
                      benchmark_process_attempts=dict(self.sample_attempts),
                      benchmark_process_successes=dict(self.sample_successes),oracle_counts=dict(self.oracle_counts),
                      command_count=self.commands.next_id if self.commands else 0,
                      command_counts_by_stage=dict(self.commands.counts) if self.commands else {},
                      head=self.actual_head,source_manifest_sha256=self.source_manifest_sha256,
                      plan_sha256=self.plan_sha256,
                      github_run_id=os.environ.get('GITHUB_RUN_ID'),
                      github_run_attempt=os.environ.get('GITHUB_RUN_ATTEMPT'),
                      completeness_contract='Require this terminal event, contiguous seq 1..final_event_count and valid event chain; missing/truncated logs are incomplete')
        self.summary = summary
        return 1 if incomplete else 0

    def run(self):
        self.initial_manifest()
        self.validate_plan()
        self.setup()
        self.build_full_oracle()
        for target in self.job['flags_order']:
            self.log.emit('flag_start',flag=target,flags_order=self.job['flags_order'],cpu=self.cpu)
            try:
                self.check_isa(target)
                self.prepare_flag(target)
                self.measure_flag(target)
            except (LogLimit, KeyboardInterrupt):
                raise
            except Exception as e:
                self.failure('flag_gate',e,flag=target)
            self.log.emit('flag_end',flag=target,measurement_count=sum(r['flag']==target for r in self.results))


def main(argv):
    log = Log()
    runner = None
    fatal = None
    try:
        require(len(argv) == 5, 'Usage: runner_draft.py REPO EMPTY_WORK PLAN SHARD_ID')
        repo,work,plan = (Path(x).resolve() for x in argv[1:4])
        # Basic started evidence exists even if plan JSON cannot be parsed.
        log.emit('invocation',argv=argv[1:],runner=file_info(__file__),pid=os.getpid(),
                 note='log-only execution; requires frozen plan and separate launch authorization')
        runner = Runner(repo,work,plan,argv[4],log)
        runner.run()
    except BaseException as e:
        fatal = f'{type(e).__name__}: {e}'[:4096]
        if isinstance(e,LogLimit):
            log.incomplete = True
        log.emit('fatal',terminal=True,error=fatal,benchmark_retry=False)
    if runner is not None:
        try:
            return runner.finish(fatal)
        except BaseException as e:
            log.emit('completed',terminal=True,status='incomplete',
                     fatal=f'Summary failure: {type(e).__name__}: {e}'[:2048],
                     prior_event_count=log.seq,prior_event_chain_sha256=log.chain,
                     final_event_count_including_this=log.seq+1)
            return 1
    log.emit('completed',terminal=True,status='incomplete',fatal=fatal,
             prior_event_count=log.seq,prior_event_chain_sha256=log.chain,
             final_event_count_including_this=log.seq+1)
    return 1


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))
