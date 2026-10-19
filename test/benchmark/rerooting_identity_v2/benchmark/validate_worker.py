#!/usr/bin/env python3
"""Clock-free worker validation; synthetic elapsed values are never measurements.

The monotonic clock below is used solely for external watchdogs. No worker is
allowed to use the real clock. One conflicting-option parse-only test passes both
clock options with stdin closed and requires exit 2. Existing reports are never
overwritten, and every request/response is flushed into an append-only log.
"""
from __future__ import annotations

import argparse
from contextlib import ExitStack
import hashlib
import json
import os
from pathlib import Path
import re
import selectors
import signal
import struct
import subprocess
import tempfile
import time

HERE = Path(__file__).resolve().parent
SEEDS = (857123, 857124, 857125)
PROTOCOL_SHA256 = "434d67065f3a7a3de061732c589a38af8f64a3197252ac144ae4b5956219e1ca"
MAX_REPS = 128
MAX_STORAGE = 512 << 20
MAX_RSS = 1024 << 20
MAX_ARTIFACTS = 256 << 20
MAX_ELAPSED_NS = 2_000_000_000
HEADER = struct.Struct("<8s6I")
PAIR = struct.Struct("<qI")
MATRIX = struct.Struct("<16I")
MOD = 1_000_000_007
MASK = (1 << 64) - 1
HEX64 = re.compile(r"[0-9a-f]{16}\Z")


class ValidationError(RuntimeError):
    pass


def require(condition, detail):
    if not condition:
        raise ValidationError(str(detail))


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def fnv64(data, initial=14695981039346656037):
    value = initial
    for octet in data:
        value = ((value ^ octet) * 1099511628211) & MASK
    return value


def digest(data):
    return f"{fnv64(data):016x}"


def strict_json(data):
    def unique(items):
        result = {}
        for key, value in items:
            require(key not in result, f"Duplicate JSON key: {key}")
            result[key] = value
        return result
    return json.loads(data, object_pairs_hook=unique,
                      parse_constant=lambda value: (_ for _ in ()).throw(
                          ValidationError(f"Non-finite JSON value: {value}")))


class MT19937:
    """Independent standard 32-bit MT19937, not Python's differently seeded RNG."""
    def __init__(self, seed):
        self.state = [seed & 0xffffffff]
        for index in range(1, 624):
            last = self.state[-1]
            self.state.append((1812433253 * (last ^ (last >> 30)) + index) & 0xffffffff)
        self.index = 624

    def next(self):
        if self.index == 624:
            for index in range(624):
                joined = ((self.state[index] & 0x80000000) |
                          (self.state[(index + 1) % 624] & 0x7fffffff))
                self.state[index] = (self.state[(index + 397) % 624] ^
                                     (joined >> 1) ^ (0x9908b0df if joined & 1 else 0))
            self.index = 0
        value = self.state[self.index]
        self.index += 1
        value ^= value >> 11
        value ^= (value << 7) & 0x9d2c5680
        value ^= (value << 15) & 0xefc60000
        value ^= value >> 18
        return value & 0xffffffff


def specification(cell, seed):
    require(cell in range(1, 11) and seed in SEEDS, "Not a fixed cell/seed")
    kind = int(cell > 5)
    n = 8192 if kind else 32768
    shape = (cell - 1) % 5
    rng = MT19937(seed + 104729 * shape)
    parents = []
    for vertex in range(1, n):
        if shape == 0:
            parent = vertex - 1
        elif shape == 1:
            parent = 0
        elif shape == 2:
            parent = (vertex - 1) // 2
        elif shape == 3:
            parent = vertex - 1 if vertex < n // 2 else n // 2 - 1
        else:
            parent = rng.next() % vertex
        parents.append(parent)
    labels = list(range(n))
    for index in range(n - 1, 0, -1):
        other = rng.next() % (index + 1)
        labels[index], labels[other] = labels[other], labels[index]
    edges = []
    degrees = [0] * n
    for vertex, parent in enumerate(parents, 1):
        u, v = labels[parent], labels[vertex]
        if rng.next() & 1:
            u, v = v, u
        edges.append((u, v))
        degrees[u] += 1
        degrees[v] += 1
    for index in range(len(edges) - 1, 0, -1):
        other = rng.next() % (index + 1)
        edges[index], edges[other] = edges[other], edges[index]
    root = labels[0]
    m = n - 1
    length = n + 2 * m
    fields = (cell, seed, n, m, root, kind)
    canonical = bytearray(HEADER.pack(b"RRIN0001", *fields))
    for edge in edges:
        canonical.extend(struct.pack("<2I", *edge))
    if kind:
        matrices = MT19937(seed ^ 0x9e3779b9)
        for _ in range(n):
            canonical.extend(MATRIX.pack(*(1 + matrices.next() % (MOD - 1)
                                           for _ in range(16))))
    expected = {
        "cell": cell, "seed": seed, "type": "matrix" if kind else "pair",
        "shape": shape, "n": n, "m": m, "root": root,
        "output_length": length, "scalar_count": length * (16 if kind else 2),
        "saved_merges": sum(degree > 1 for vertex, degree in enumerate(degrees)
                            if vertex != root),
        "storage_per_result": 256 + 2 * (8 * m + length * (64 if kind else 16)),
    }
    require(expected["storage_per_result"] * MAX_REPS <= MAX_STORAGE,
            "Fixed protocol count 128 must fit conservative storage cap")
    if shape == 0:
        require(expected["saved_merges"] == n - 2, "Path savings formula")
    if shape == 1:
        require(expected["saved_merges"] == 0, "Centered-star savings formula")
    return expected, fields, bytes(canonical)


class Worker:
    """Bounded nonblocking pipes; no readline, communicate, or unbounded wait."""
    def __init__(self, harness, label, *options, fixtures_root=None, parse_conflict_only=False):
        forbidden = "--enable-timing" in options or "--real-clock" in options
        require(not forbidden or (parse_conflict_only and tuple(options) ==
                ("--enable-timing", "--synthetic-clock-ns", "1")),
                "Real timing options are prohibited except mutually exclusive parse-only rejection")
        self.parse_conflict_only = parse_conflict_only
        self.harness = harness
        self.label = label
        self.options = list(options)
        self.root = Path(fixtures_root or harness.roots[label]).resolve()
        self.root.mkdir(parents=True, exist_ok=True)
        self.argv = [harness.manifest["binaries"][label]["path"],
                     "--fixtures-root", str(self.root), *options]
        self.p = subprocess.Popen(self.argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE, bufsize=0, start_new_session=True)
        self.selector = selectors.DefaultSelector()
        self.stdout = bytearray()
        self.stderr = bytearray()
        self.stderr_total = 0
        self.closed = False
        for stream, kind in ((self.p.stdin, "stdin"), (self.p.stdout, "stdout"),
                             (self.p.stderr, "stderr")):
            os.set_blocking(stream.fileno(), False)
            if kind != "stdin":
                self.selector.register(stream, selectors.EVENT_READ, kind)
        harness.workers.append(self)
        harness.log({"event": "start", "build": label, "argv": self.argv,
                     "pid": self.p.pid})

    def _remaining(self, deadline):
        remaining = min(deadline, self.harness.deadline) - time.monotonic()
        require(remaining > 0, "External worker request/campaign watchdog expired")
        return remaining

    def _read(self, stream, kind):
        try:
            data = os.read(stream.fileno(), 65536)
        except BlockingIOError:
            return
        if not data:
            self.selector.unregister(stream)
            return
        if kind == "stdout":
            self.stdout.extend(data)
            require(len(self.stdout) <= 65536, "Worker stdout response exceeds 64 KiB")
        else:
            self.stderr_total += len(data)
            self.stderr.extend(data)
            if len(self.stderr) > 65536:
                del self.stderr[:-65536]
            require(self.stderr_total <= 1 << 20, "Worker stderr exceeds 1 MiB")

    def _drain_ready(self):
        for key, _ in self.selector.select(0):
            if key.data != "stdin":
                self._read(key.fileobj, key.data)

    def query(self, command):
        require(not self.closed, "Request sent to closed worker")
        require(not self.parse_conflict_only, "Commands forbidden for parse-only option rejection")
        deadline = min(self.harness.deadline, time.monotonic() + 5.0)
        self.harness.log({"event": "request", "build": self.label,
                          "options": self.options, "command": command})
        try:
            self._drain_ready()
            require(not self.stdout, "Unexpected unsolicited/duplicate worker response")
            pending = memoryview((command + "\n").encode("ascii"))
            self.selector.register(self.p.stdin, selectors.EVENT_WRITE, "stdin")
            while True:
                for key, _ in ([] if b"\n" in self.stdout else
                               self.selector.select(self._remaining(deadline))):
                    if key.data == "stdin":
                        try:
                            count = os.write(key.fileobj.fileno(), pending)
                        except BlockingIOError:
                            continue
                        require(count > 0, "Worker stdin closed during request")
                        pending = pending[count:]
                        if not pending:
                            self.selector.unregister(self.p.stdin)
                    else:
                        self._read(key.fileobj, key.data)
                if b"\n" in self.stdout:
                    require(not pending, "Worker responded before complete command write")
                    line, remainder = self.stdout.split(b"\n", 1)
                    self.stdout = bytearray(remainder)
                    self._drain_ready()
                    require(not self.stdout, "Duplicate or trailing worker response")
                    response = strict_json(line.decode("utf-8"))
                    require(isinstance(response, dict) and type(response.get("ok")) is bool,
                            "Worker response is not an object with Boolean ok")
                    if "clock_reads" in response:
                        require(type(response["clock_reads"]) is int and response["clock_reads"] == 0,
                                "Worker read real elapsed clock during validation")
                    self.harness.log({"event": "response", "build": self.label,
                                      "command": command, "response": response})
                    return response
                if self.p.poll() is not None:
                    self._drain_ready()
                    require(b"\n" in self.stdout,
                            f"Worker exited without response: {self.p.returncode}; "
                            f"stderr={self.stderr.decode('utf-8', errors='replace')}")
        except BaseException:
            self.abort()
            raise

    def close(self):
        if self.closed:
            return
        require(self.query("quit") == {"ok": True, "quit": True}, "Invalid quit response")
        self.p.stdin.close()
        deadline = min(self.harness.deadline, time.monotonic() + 5.0)
        while self.p.poll() is None:
            self._drain_ready()
            require(not self.stdout, "Unexpected output after quit")
            self.selector.select(min(0.02, self._remaining(deadline)))
        self._drain_ready()
        require(not self.stdout and self.p.returncode == 0,
                f"Worker quit status/output incorrect: {self.p.returncode}")
        self._release()

    def abort(self):
        if self.closed:
            return
        try:
            os.killpg(self.p.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        try:
            self.p.wait(timeout=1.0)
        except subprocess.TimeoutExpired:
            self.harness.log({"event": "cleanup_failure", "pid": self.p.pid})
        self.harness.log({"event": "worker_aborted", "build": self.label,
                          "returncode": self.p.poll(),
                          "stderr_tail": self.stderr.decode("utf-8", errors="replace")})
        self._release()

    def _release(self):
        self.closed = True
        self.selector.close()
        for stream in (self.p.stdin, self.p.stdout, self.p.stderr):
            if not stream.closed:
                stream.close()

    def __enter__(self):
        return self

    def __exit__(self, kind, value, traceback):
        if kind is None:
            try:
                self.close()
            except BaseException:
                self.abort()
                raise
        else:
            self.abort()


class Harness:
    def __init__(self, args, raw):
        self.args = args
        self.raw = raw
        self.deadline = min(time.monotonic() + 360.0, args.deadline_monotonic)
        self.manifest_path = args.manifest.resolve()
        self.manifest = strict_json(self.manifest_path.read_text())
        require(self.manifest.get("real_timing_executed") is False,
                "Manifest does not attest zero real timings")
        self.workers = []
        self.batch_hashes = {}
        self.roots = {label: HERE / "fixtures" / label for label in ("baseline", "candidate")}
        require(set(self.manifest["binaries"]) == {"baseline", "candidate"},
                "Manifest must declare exactly both fixed build labels")
        require(self.manifest.get("fixture_roots") ==
                {label: str(path) for label, path in self.roots.items()},
                "Manifest fixture roots differ from fixed validation destinations")
        self.report = {"schema_version": 1, "status": "in_progress",
                       "real_timing_executed": False, "full_size_cases": 0,
                       "untimed_batch_checks": 0, "cross_build_pairs": 0,
                       "logical_comparison_before_canonical_bytes": True,
                       "synthetic_checks": [], "datasets": [], "fixtures": {},
                       "manifest": {"path": str(self.manifest_path),
                                    "sha256": sha256(self.manifest_path)},
                       "raw_log": str(args.output.with_suffix(".raw.jsonl"))}
        self.provenance = {str(self.manifest_path): sha256(self.manifest_path),
                           str(Path(__file__).resolve()): sha256(__file__),
                           str(HERE.parent / "prospective-benchmark.md"): PROTOCOL_SHA256}
        self._discover_provenance(self.manifest)
        self.verify_provenance()

    def _discover_provenance(self, value):
        if isinstance(value, dict):
            if isinstance(value.get("path"), str) and isinstance(value.get("sha256"), str):
                path = Path(value["path"])
                self.provenance[str(path.resolve())] = value["sha256"]
            for key, child in value.items():
                if isinstance(child, str) and re.fullmatch(r"[0-9a-f]{64}", child):
                    path = Path(key)
                    if path.is_absolute():
                        self.provenance[str(path)] = child
                elif isinstance(child, (dict, list)):
                    self._discover_provenance(child)
        elif isinstance(value, list):
            for child in value:
                self._discover_provenance(child)

    def log(self, item):
        self.raw.write(json.dumps(item, sort_keys=True) + "\n")
        self.raw.flush()

    def verify_provenance(self):
        require(time.monotonic() < self.deadline, "Validation campaign deadline expired")
        for path, expected in self.provenance.items():
            require(Path(path).is_file() and sha256(path) == expected,
                    f"Provenance mismatch: {path}")

    def fixture(self, worker, response, name, magic, fields, expected_length):
        path = Path(response[name])
        require(path.is_absolute(), f"Nonabsolute {name}")
        require(path.resolve().is_relative_to(worker.root), f"{name} escapes fixture root")
        require(path.is_file() and path.stat().st_size == expected_length,
                f"Invalid canonical {name} file length")
        content = path.read_bytes()
        require(HEADER.unpack_from(content) == (magic, *fields),
                f"Unexpected canonical {name} schema/version/header")
        self.report["fixtures"][str(path)] = hashlib.sha256(content).hexdigest()
        return content

    def prepare(self, worker, cell, seed, expected, fields, canonical_input):
        result = worker.query(f"prepare {cell} {seed}")
        require(result["ok"], ("prepare", cell, seed, result))
        for field, value in expected.items():
            require(result.get(field) == value and type(result[field]) is type(value),
                    (field, value, result.get(field)))
        require("elapsed_ns" not in result, "Preparation exposed elapsed wrapper timing")
        require(result.get("clock_reads") == 0 and result.get("small_oracle_passed") is True,
                "Preparation did not attest clock-free independent small-oracle success")
        require(type(result.get("peak_rss_bytes")) is int and
                0 < result["peak_rss_bytes"] <= MAX_RSS, "Preparation RSS failure")
        input_bytes = self.fixture(worker, result, "input_file", b"RRIN0001", fields,
                                   len(canonical_input))
        require(input_bytes == canonical_input, "Fixed-generator canonical input differs")
        output_size = HEADER.size + expected["output_length"] * (64 if cell > 5 else 12)
        output_bytes = self.fixture(worker, result, "output_file", b"RROU0001", fields,
                                    output_size)
        if cell > 5:
            require(all(value < MOD for value, in struct.iter_unpack("<I", output_bytes[HEADER.size:])),
                    "Matrix output entry outside canonical residue range")
        else:
            for index, (distance, count) in enumerate(PAIR.iter_unpack(output_bytes[HEADER.size:])):
                require(0 <= distance <= expected["n"] * (expected["n"] - 1) // 2,
                        "Pair distance outside legitimate tree-DP range")
                require(1 <= count <= expected["n"], "Pair count outside legitimate range")
                if index < expected["n"]:
                    require(count == expected["n"], "All-root pair value has wrong vertex count")
        require(result.get("input_hash") == digest(input_bytes), "Canonical input FNV mismatch")
        require(result.get("output_hash") == digest(output_bytes), "Canonical output FNV mismatch")
        self.report["datasets"].append({"build": worker.label, **result})
        self.report["full_size_cases"] += 1
        return result, output_bytes

    def batch(self, worker, reference, count, elapsed=None, *, command="untimed_batch"):
        result = worker.query(f"{command} {reference['cell']} {reference['seed']} {count}")
        require(result["ok"], ("batch", result))
        expected = {"cell": reference["cell"], "seed": reference["seed"],
                    "repetitions": count, "elapsed_ns": elapsed,
                    "synthetic_clock": command == "batch",
                    "storage_bytes": reference["storage_per_result"] * count,
                    "output_hash": reference["output_hash"], "full_outputs_verified": count,
                    "logical_values_verified": count * reference["output_length"]}
        for key, value in expected.items():
            require(result.get(key) == value and type(result[key]) is type(value),
                    (key, value, result.get(key)))
        require(type(result.get("peak_rss_bytes")) is int and 0 < result["peak_rss_bytes"] <= MAX_RSS,
                "RSS cap or RSS reporting failure")
        require(result["storage_bytes"] <= MAX_STORAGE, "Storage cap violated")
        require(isinstance(result.get("batch_hash"), str) and HEX64.fullmatch(result["batch_hash"]),
                "Malformed complete-output batch hash")
        key = (reference["output_hash"], count)
        if key not in self.batch_hashes:
            if count == 1:
                self.batch_hashes[key] = reference["output_hash"]
            else:
                canonical = Path(reference["output_file"]).read_bytes()
                value = 14695981039346656037
                for _ in range(count):
                    require(time.monotonic() < self.deadline, "Campaign deadline during batch hash")
                    value = fnv64(canonical, value)
                self.batch_hashes[key] = f"{value:016x}"
        require(result["batch_hash"] == self.batch_hashes[key],
                "Batch hash differs from independently repeated complete canonical outputs")
        require(result.get("clock_reads") == 0, "Batch read real clock")
        return result

    def fail(self, worker, command, code):
        result = worker.query(command)
        require(result.get("ok") is False and result.get("error_code") == code
                and result.get("clock_reads") == 0,
                (command, code, result))
        return result

    def run(self):
        mt = MT19937(5489)
        require([mt.next() for _ in range(5)] ==
                [3499211612, 581869302, 3890346734, 3586334585, 545404204],
                "Independent MT19937 known-answer test failed")
        references = {}
        with ExitStack() as stack:
            workers = {label: stack.enter_context(Worker(self, label))
                       for label in ("baseline", "candidate")}
            for cell in range(1, 11):
                for seed in SEEDS:
                    expected, fields, canonical_input = specification(cell, seed)
                    a, a_bytes = self.prepare(workers["baseline"], cell, seed,
                                              expected, fields, canonical_input)
                    b, b_bytes = self.prepare(workers["candidate"], cell, seed,
                                              expected, fields, canonical_input)
                    scalar = MATRIX if cell > 5 else PAIR
                    # Compare logical fields first, without using a byte/hash equality shortcut.
                    for index, (left, right) in enumerate(zip(
                            scalar.iter_unpack(a_bytes[HEADER.size:]),
                            scalar.iter_unpack(b_bytes[HEADER.size:]), strict=True)):
                        require(left == right, ("Logical full-output mismatch", cell, seed, index))
                    # Only after all logical fields match, compare canonical complete bytes.
                    require(a_bytes == b_bytes, ("Canonical full-output mismatch", cell, seed))
                    require({key: value for key, value in a.items()
                             if key not in ("input_file", "output_file", "peak_rss_bytes")} ==
                            {key: value for key, value in b.items()
                             if key not in ("input_file", "output_file", "peak_rss_bytes")},
                            "Baseline/candidate preparation metadata differ")
                    self.report["cross_build_pairs"] += 1
                    references[cell, seed] = a
                    batches = [self.batch(workers[label], ref, 1)
                               for label, ref in (("baseline", a), ("candidate", b))]
                    require(batches[0]["batch_hash"] == batches[1]["batch_hash"],
                            "Cross-build untimed batch hashes differ")
                    self.report["untimed_batch_checks"] += 2
                    self.log({"event": "full_size_case_validated", "cell": cell, "seed": seed,
                              "logical_values_per_build": expected["output_length"],
                              "canonical_bytes_per_build": len(a_bytes)})
            for label, worker in workers.items():
                self.fail(worker, "batch 6 857123 1", "timing_disabled")
                for command, code in (
                    ("", "malformed_command"), ("bogus", "malformed_command"),
                    ("prepare", "malformed_command"),
                    ("prepare 1 857123 extra", "malformed_command"),
                    ("prepare 0 857123", "invalid_cell"),
                    ("prepare 11 857123", "invalid_cell"),
                    ("prepare 1 42", "invalid_seed"),
                    ("prepare 1 857126", "invalid_seed"),
                    ("untimed_batch", "malformed_command"),
                    ("untimed_batch 6 857123 1 extra", "malformed_command"),
                    ("quit extra", "malformed_command"),
                    ("untimed_batch 6 857123 0", "repetition_cap"),
                    ("untimed_batch 6 857123 129", "repetition_cap"),
                    ("untimed_batch 6 857123 18446744073709551615", "repetition_cap")):
                    self.fail(worker, command, code)
                result = worker.query("untimed_batch 6 857123 -1")
                require(result.get("ok") is False, "Negative repetition accepted")
                self.report["synthetic_checks"].append(
                    f"{label}: default timing refusal and malformed/fixed-domain/repetition checks")
        self.synthetic(references)
        require(self.report["full_size_cases"] == 60 and self.report["untimed_batch_checks"] == 60
                and self.report["cross_build_pairs"] == 30, "Incomplete fixed full-size suite")
        artifact_bytes = sum(Path(path).stat().st_size for path in self.report["fixtures"])
        require(artifact_bytes <= MAX_ARTIFACTS, "Canonical artifact cap exceeded")
        for path, expected in self.report["fixtures"].items():
            require(sha256(path) == expected, f"Fixture changed after verification: {path}")
        self.verify_provenance()
        self.report.update(status="passed", artifact_bytes=artifact_bytes,
                           provenance_pre_post_unchanged=True,
                           sources_checked=self.provenance,
                           full_size_oracle_preparations=60,
                           worker_reported_small_oracle_passes=60,
                           small_oracle_coverage={
                               "source": "check_small in manifest-hashed worker.cpp",
                               "per_fresh_prepare": "8-vertex tree and edge-deleted forest, each at all 8 roots; empty and singleton; complete vertices and directed messages",
                               "matrix_additional": "independent termwise product oracle, both identity sides, associativity, noncommutativity witness, maximum-entry product widening",
                               "measured_or_inferred_runtime_case_count": False})

    def synthetic(self, references):
        for duration in (0, 99_999_999, 100_000_000, 2_000_000_000, 2_000_000_001):
            with Worker(self, "candidate", "--synthetic-clock-ns", str(duration)) as worker:
                require(worker.query("prepare 6 857123")["ok"], "Synthetic prepare failed")
                if duration <= MAX_ELAPSED_NS:
                    self.batch(worker, references[6, 857123], 1, duration, command="batch")
                else:
                    response = self.fail(worker, "batch 6 857123 1", "batch_duration_cap")
                    require(response.get("elapsed_ns") == duration,
                            "Failed duration not retained")
        self.report["synthetic_checks"].append(
            "Synthetic elapsed 0/99,999,999/100,000,000/2,000,000,000 accepted; 2,000,000,001 rejected")
        for label in ("baseline", "candidate"):
            with Worker(self, label, "--synthetic-clock-ns", "100000000") as worker:
                self.fail(worker, "batch 1 857123 1", "not_prepared")
                for cell in (1, 6):
                    require(worker.query(f"prepare {cell} 857123")["ok"], "Count-cap prepare failed")
                    self.batch(worker, references[cell, 857123], MAX_REPS, 100_000_000, command="batch")
                    for count in (0, MAX_REPS + 1, 18446744073709551615):
                        self.fail(worker, f"batch {cell} 857123 {count}", "repetition_cap")
                self.report["synthetic_checks"].append(
                    f"{label}: pair/matrix count 128 full lifecycle passed; zero/>128 rejected; unprepared refused")
        for fault, code in (("bad_result", "result_mismatch"), ("storage", "storage_cap"),
                            ("rss", "rss_cap")):
            with Worker(self, "candidate", "--synthetic-clock-ns", "100000000",
                        "--fault", fault) as worker:
                require(worker.query("prepare 6 857123")["ok"], "Fault prepare failed")
                self.fail(worker, "batch 6 857123 129", "repetition_cap")
                failure = self.fail(worker, "batch 6 857123 1", code)
                # A pre-allocation storage failure need not have an elapsed observation.
                if "elapsed_ns" in failure:
                    require(failure["elapsed_ns"] == 100_000_000, "Wrong synthetic failure elapsed")
                self.report["synthetic_checks"].append(
                    f"Injected {fault} rejected as {code}; repetition cap precedence verified")
        self.option_rejection(("--fault", "bad_result"))
        self.option_rejection(("--synthetic-clock-ns", "-1"))
        self.option_rejection(("--synthetic-clock-ns", "100", "--fault", "unknown"))
        self.option_rejection(("--enable-timing", "--synthetic-clock-ns", "1"), parse_conflict_only=True)
        self.report["synthetic_checks"].append(
            "Fault without synthetic clock, invalid synthetic duration, unknown fault, and conflicting real+synthetic parse-only options refused with exit 2")
        for file_field in ("input_file", "output_file"):
            with tempfile.TemporaryDirectory(prefix="rerooting-fixture-negative-") as root:
                with Worker(self, "candidate", fixtures_root=root) as worker:
                    prepared = worker.query("prepare 6 857123")
                    require(prepared["ok"], "Fixture immutability setup failed")
                path = Path(prepared[file_field])
                corrupted = bytearray(path.read_bytes())
                corrupted[-1] ^= 1
                path.write_bytes(corrupted)
                expected_corruption = hashlib.sha256(corrupted).hexdigest()
                with Worker(self, "candidate", fixtures_root=root) as worker:
                    self.fail(worker, "prepare 6 857123", "fixture_mismatch")
                require(sha256(path) == expected_corruption,
                        "Mismatching existing fixture was overwritten")
                self.log({"event": "fixture_immutability_rejection", "field": file_field,
                          "corrupted_sha256_preserved": expected_corruption})
        self.report["synthetic_checks"].append(
            "Existing input and output fixture corruption independently refused without overwriting bytes")

    def option_rejection(self, options, *, parse_conflict_only=False):
        # EOF is supplied immediately: an accidentally accepted invocation must still
        # exit nonzero, and cannot wait indefinitely for a request or read a real clock.
        with Worker(self, "candidate", *options, parse_conflict_only=parse_conflict_only) as worker:
            worker.p.stdin.close()
            deadline = min(self.deadline, time.monotonic() + 5.0)
            while worker.p.poll() is None:
                worker._drain_ready()
                worker.selector.select(min(0.02, worker._remaining(deadline)))
            worker._drain_ready()
            require(worker.p.returncode == 2, ("Invalid options accepted/wrong exit", options, worker.p.returncode))
            self.log({"event": "option_rejected", "options": options,
                      "returncode": worker.p.returncode,
                      "stdout": worker.stdout.decode("utf-8", errors="replace"),
                      "stderr": worker.stderr.decode("utf-8", errors="replace")})
            worker._release()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=HERE / "build-manifest.compiled.json")
    parser.add_argument("--output", type=Path, default=HERE / "worker-validation.json")
    parser.add_argument("--deadline-monotonic", type=float,
                        default=float(os.environ.get("REROOTING_CAMPAIGN_DEADLINE_MONOTONIC", "inf")))
    args = parser.parse_args()
    args.output = args.output.resolve()
    require(not args.output.exists(), f"Refuse to overwrite validation report: {args.output}")
    log_path = args.output.with_suffix(".raw.jsonl")
    require(not log_path.exists(), f"Refuse to overwrite raw validation log: {log_path}")
    harness = None
    with log_path.open("x") as raw:
        try:
            harness = Harness(args, raw)
            harness.run()
        except BaseException as error:
            if harness is not None:
                harness.report["status"] = "failed"
                harness.report["failure"] = {"type": type(error).__name__, "message": str(error)}
                harness.log({"event": "validation_failure", **harness.report["failure"]})
                try:
                    harness.verify_provenance()
                    harness.report["provenance_pre_post_unchanged"] = True
                except BaseException as provenance_error:
                    harness.report["provenance_pre_post_unchanged"] = False
                    harness.report["provenance_failure"] = str(provenance_error)
            else:
                failure = {"status": "failed", "real_timing_executed": False,
                           "failure": {"type": type(error).__name__, "message": str(error)},
                           "full_size_cases": 0, "raw_log": str(log_path)}
                raw.write(json.dumps({"event": "initialization_failure", **failure}) + "\n")
                raw.flush()
                with args.output.open("x") as target:
                    json.dump(failure, target, indent=2)
                    target.write("\n")
            raise
        finally:
            if harness is not None:
                for worker in harness.workers:
                    worker.abort()
                with args.output.open("x") as target:
                    json.dump(harness.report, target, indent=2, sort_keys=True)
                    target.write("\n")
    print("PASS: 60 complete full-size prepares, 30 logical-then-byte cross-build pairs, "
          "60 untimed batches, synthetic lifecycle/cap/failure checks. No real timings.")


if __name__ == "__main__":
    main()
