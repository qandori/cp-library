#!/usr/bin/env python3
"""Local-only, fail-closed collector for the frozen rerooting identity-elision protocol v1.

This module does not enable timing on import. `validate` never reads worker
timings; `synthetic` is not performance evidence; `campaign` requires the explicit
--authorize-real-timing flag. No installation, networking or remote writes occur.

Manifest schema 1 requires sealed=true, manifest_kind=sealed-full-fixtures,
canonical absolute artifact_root and fixture_roots, exact protocol identity and
compiler flags, resolved driver/backend/tool hashes, baseline/candidate binaries,
and nonempty SHA256 maps: sources, baseline_source, candidate_source, objects,
fixtures, closure_headers, toolchain, interpreted_sources, preparation_evidence.
The fixture inventory is exactly 120 input/output files for 10 cells x 3 seeds x
2 builds. Raw logs must live below artifact_root and are never overwritten.

The manifest is a frozen build record, not a proof that binaries were compiled
from the declared source. The build procedure must establish that separately.
Every listed artifact and the manifest itself are checked before and after the
run. `Clock`, worker transports, and the log sink are injectable for testing.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import selectors
import signal
import resource
import subprocess
import sys
import time
from dataclasses import dataclass
from fractions import Fraction
from typing import Any, Callable, Protocol


PROTOCOL_SHA256 = "434d67065f3a7a3de061732c589a38af8f64a3197252ac144ae4b5956219e1ca"
BASELINE_COMMIT = "9f05f01500656c65e394a8aa11c3f5afa4d9495a"
BASELINE_HEADER_SHA256 = "f7b2af0d128eb794a1342acc328becb82d1fd49075daa6db273d09dda308b828"
CANDIDATE_HEADER_SHA256 = "63ff812240127afd3313ac0001652cf01173c66d212063813f8d47fc3a27f5d4"
CSR_SHA256 = "2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7"
UNIT_SHA256 = "122d62eb83c84ee2f0771a52ebe6ffb3b4a7f7406051df7b5326128dcbd861fc"
SEEDS = (857123, 857124, 857125)
AA_CELLS = (1, 2, 6, 7)
PRIMARY_CELLS = (6, 8)
CONTROL_CELLS = tuple(range(1, 11))
NS = 1_000_000_000
MIN_BATCH_NS = NS // 10
MAX_BATCH_NS = 2 * NS
WATCHDOG_NS = 5 * NS
CAMPAIGN_NS = 360 * NS
MAX_REPS = 128
MAX_STORAGE = 512 * 1024 * 1024
MAX_RSS = 1024 * 1024 * 1024
MAX_COLLECTOR_RSS = 256 * 1024 * 1024
MAX_COMBINED_RSS = 2304 * 1024 * 1024
MAX_ARTIFACTS = 256 * 1024 * 1024
SHORT_BATCH_NS = 1_000_000
MAX_RESPONSE_BYTES = 64 * 1024
FLAGS = ["-std=c++23", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror",
         "-fno-lto", "-march=x86-64", "-mtune=generic"]


@dataclass(frozen=True)
class Cell:
    id: int
    type: str
    shape: int
    n: int
    initial_reps: int = 1

    @property
    def m(self) -> int:
        return self.n - 1

    @property
    def output_length(self) -> int:
        return self.n + 2 * self.m

    @property
    def scalar_count(self) -> int:
        return self.output_length * (2 if self.type == "pair" else 16)

    @property
    def element_bytes(self) -> int:
        return 16 if self.type == "pair" else 64

    @property
    def storage_per_result(self) -> int:
        return 256 + 2 * (8 * self.m + self.output_length * self.element_bytes)


CELLS = tuple(Cell(i, "pair" if i <= 5 else "matrix", (i - 1) % 5,
                   32768 if i <= 5 else 8192) for i in range(1, 11))
CELL_BY_ID = {c.id: c for c in CELLS}


class ScreenError(Exception):
    def __init__(self, code: str, message: str):
        self.code = code
        super().__init__(message)


class Clock(Protocol):
    def now_ns(self) -> int: ...


class RealClock:
    def now_ns(self) -> int:
        return time.monotonic_ns()


class Transport(Protocol):
    def request(self, command: str, timeout_ns: int) -> str: ...
    def close(self) -> None: ...


def strict_json(raw: str) -> Any:
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError("duplicate JSON key: " + key)
            result[key] = value
        return result

    def bad_constant(value):
        raise ValueError("non-finite JSON number: " + value)

    return json.loads(raw, object_pairs_hook=pairs, parse_constant=bad_constant)


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


class Manifest:
    def __init__(self, path: Path):
        self.path = path.resolve()
        try:
            self.sha256 = digest(self.path)
            self.data = strict_json(self.path.read_text())
            if type(self.data.get("schema_version")) is not int or self.data["schema_version"] != 1:
                raise ValueError("schema_version must be 1")
            if self.data["protocol"]["sha256"] != PROTOCOL_SHA256:
                raise ValueError("protocol identity is not frozen rerooting protocol v1")
            compiler = self.data["compiler"]
            if not isinstance(compiler["version"], str) or not re.search(r"(?<![0-9.])15\.2\.0(?![0-9.])", compiler["version"]):
                raise ValueError("frozen compiler must identify GCC 15.2")
            flags = compiler["flags"]
            expected_flags = FLAGS
            if flags != expected_flags:
                raise ValueError("compiler flags do not match reviewed generic non-LTO build")
            for field in ("no_lto", "separate_translation_units", "external_kernel_symbols_and_call_sites_verified"):
                if self.data[field] is not True:
                    raise ValueError(field + " must be true")
            if self.data["real_timing_executed"] is not False:
                raise ValueError("build record must not claim timing execution")
            if not isinstance(self.data["commands"], list) or not self.data["commands"]:
                raise ValueError("build commands must be recorded")
            if self.data.get("sealed") is not True or self.data.get("manifest_kind") != "sealed-full-fixtures":
                raise ValueError("collector requires sealed-full-fixtures manifest")
            self.artifact_root = canonical_absolute(self.data["artifact_root"])
            self.fixture_roots = {b: canonical_absolute(self.data["fixture_roots"][b])
                                  for b in ("baseline", "candidate")}
            if not self.path.is_relative_to(self.artifact_root):
                raise ValueError("sealed manifest must reside below artifact_root")
            if self.fixture_roots["baseline"] == self.fixture_roots["candidate"]:
                raise ValueError("build fixture roots must be distinct")
            if any(not p.is_relative_to(self.artifact_root) for p in self.fixture_roots.values()):
                raise ValueError("fixture roots must reside below artifact_root")
            self.artifacts = [("protocol", self.data["protocol"]), ("compiler", compiler),
                              ("compiler backend", compiler["backend"])]
            if not isinstance(compiler["tools"], dict) or not compiler["tools"]:
                raise ValueError("resolved compiler tools must be recorded")
            self.artifacts.extend(("compiler tool " + name, item) for name, item in compiler["tools"].items())
            for field in ("sources", "baseline_source", "candidate_source", "fixtures", "objects",
                          "closure_headers", "toolchain", "interpreted_sources", "preparation_evidence"):
                entry = self.data[field]
                if not isinstance(entry, dict) or not entry:
                    raise ValueError(field + " must be a nonempty path/hash mapping")
                self.artifacts.extend((field, {"path": p, "sha256": h}) for p, h in entry.items())
            if self.data.get("baseline_commit") != BASELINE_COMMIT:
                raise ValueError("baseline commit does not match frozen protocol")
            if not {BASELINE_HEADER_SHA256, CSR_SHA256}.issubset(self.data["baseline_source"].values()):
                raise ValueError("baseline header/CSR identities missing from source scope")
            if not {CANDIDATE_HEADER_SHA256, CSR_SHA256, UNIT_SHA256}.issubset(self.data["candidate_source"].values()):
                raise ValueError("candidate header/CSR/permanent unit identities missing from source scope")
            for build in ("baseline", "candidate"):
                self.artifacts.append((build + " binary", self.data["binaries"][build]))
            for label, item in self.artifacts:
                canonical_absolute(item["path"])
                if not re.fullmatch(r"[0-9a-f]{64}", item["sha256"]):
                    raise ValueError(label + " hash must be lowercase SHA-256")
            expected_fixtures = {str(self.fixture_roots[build] / fixture_name(cell.id, seed, kind))
                                 for build in ("baseline", "candidate") for cell in CELLS
                                 for seed in SEEDS for kind in ("input", "output")}
            if set(self.data["fixtures"]) != expected_fixtures:
                raise ValueError("sealed fixtures must contain exactly all 120 canonical fixture paths")
            self.verify()
        except (OSError, ValueError, TypeError, KeyError, AttributeError) as exc:
            raise ScreenError("manifest_invalid", str(exc)) from exc

    def resolve(self, item: dict) -> Path:
        return Path(item["path"]).resolve()

    def binary(self, build: str) -> Path:
        return self.resolve(self.data["binaries"][build])

    def fixture(self, build: str, cell: int, seed: int, kind: str, path: str) -> Path:
        expected = self.fixture_roots[build] / fixture_name(cell, seed, kind)
        try:
            actual = canonical_absolute(path)
        except (TypeError, ValueError, OSError) as exc:
            raise ScreenError("fixture_path", str(exc)) from exc
        if actual != expected or path != str(expected) or path not in self.data["fixtures"]:
            raise ScreenError("fixture_path", "worker fixture path does not match sealed inventory")
        if digest(actual) != self.data["fixtures"][path]:
            raise ScreenError("source_hash_changed", "canonical fixture differs from sealed SHA256: " + path)
        return actual

    def verify(self) -> None:
        try:
            if digest(self.path) != self.sha256:
                raise ScreenError("source_hash_changed", "build manifest changed")
            for label, item in self.artifacts:
                path = canonical_absolute(item["path"])
                if digest(path) != item["sha256"]:
                    raise ScreenError("source_hash_changed", f"{label} hash mismatch: {path}")
        except (OSError, ValueError) as exc:
            raise ScreenError("source_hash_changed", str(exc)) from exc


def canonical_absolute(value: str) -> Path:
    if not isinstance(value, str) or not Path(value).is_absolute():
        raise ValueError("path must be absolute")
    path = Path(value)
    if path.resolve() != path:
        raise ValueError("path must be canonical, without symlinks or traversal")
    return path


def fixture_name(cell: int, seed: int, kind: str) -> str:
    return f"cell-{cell:02d}-seed-{seed}.{kind}.bin"


def fnv64_file(path: Path) -> str:
    value = 14695981039346656037
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            for byte in chunk:
                value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return f"{value:016x}"


def identical_files(a: Path, b: Path) -> bool:
    if a.stat().st_size != b.stat().st_size:
        return False
    with a.open("rb") as left, b.open("rb") as right:
        while True:
            chunk = left.read(1024 * 1024)
            if chunk != right.read(1024 * 1024):
                return False
            if not chunk:
                return True


class ProcessTransport:
    """Persistent worker; every request has a bounded external watchdog.

    Pipes are nonblocking. Each worker owns a session/process group; failure and
    normal cleanup kill that whole group, including descendants, without retry.
    """

    def __init__(self, argv: list[str], stderr_path: Path, clock: Clock):
        self.clock = clock
        self.closed = False
        self.stderr = stderr_path.open("xb")
        try:
            self.process = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                            stderr=self.stderr, bufsize=0, start_new_session=True)
        except BaseException:
            self.stderr.close()
            raise
        self.selector = selectors.DefaultSelector()
        assert self.process.stdout is not None and self.process.stdin is not None
        os.set_blocking(self.process.stdout.fileno(), False)
        os.set_blocking(self.process.stdin.fileno(), False)
        self.selector.register(self.process.stdout, selectors.EVENT_READ)
        self.buffer = bytearray()

    def request(self, command: str, timeout_ns: int) -> str:
        if timeout_ns <= 0 or timeout_ns > WATCHDOG_NS:
            raise ScreenError("watchdog", "invalid external request deadline")
        if self.buffer or self.selector.select(0):
            raise ScreenError("malformed_response", "unsolicited or duplicate worker response")
        deadline = self.clock.now_ns() + timeout_ns
        payload = (command + "\n").encode("ascii")
        sent = 0
        try:
            assert self.process.stdin is not None
            with selectors.DefaultSelector() as writable:
                writable.register(self.process.stdin, selectors.EVENT_WRITE)
                while sent < len(payload):
                    remaining = deadline - self.clock.now_ns()
                    if remaining <= 0:
                        raise ScreenError("watchdog", "worker stdin deadline expired")
                    if not writable.select(remaining / NS):
                        continue
                    try:
                        sent += os.write(self.process.stdin.fileno(), payload[sent:])
                    except BlockingIOError:
                        continue
        except (BrokenPipeError, OSError) as exc:
            raise ScreenError("worker_io", str(exc)) from exc
        while True:
            remaining = deadline - self.clock.now_ns()
            if remaining <= 0:
                raise ScreenError("watchdog", "worker response deadline expired; partial=" + repr(bytes(self.buffer)))
            if b"\n" in self.buffer:
                line, _, rest = self.buffer.partition(b"\n")
                if rest:
                    raise ScreenError("malformed_response", "unsolicited or duplicate worker response")
                self.buffer.clear()
                try:
                    return line.decode("utf-8", errors="strict")
                except UnicodeDecodeError as exc:
                    raise ScreenError("malformed_response", "invalid UTF-8: " + repr(bytes(line))) from exc
            if not self.selector.select(remaining / NS):
                continue
            try:
                chunk = os.read(self.process.stdout.fileno(), 65536)
            except BlockingIOError:
                continue
            if not chunk:
                raise ScreenError("worker_eof", "worker closed stdout; partial=" + repr(bytes(self.buffer)))
            self.buffer.extend(chunk)
            if len(self.buffer) > MAX_RESPONSE_BYTES:
                raise ScreenError("malformed_response", "response exceeds size bound")

    def close(self) -> None:
        if self.closed:
            return
        self.closed = True
        try:
            # Do this even if the parent already exited: descendants must not survive.
            try:
                os.killpg(self.process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            self.process.wait(timeout=1)
        finally:
            self.selector.close()
            if self.process.stdin:
                self.process.stdin.close()
            if self.process.stdout:
                self.process.stdout.close()
            self.stderr.close()


class ResourceMonitor:
    """Outside-timer accounting; peak values remain accounted after worker exit."""

    def __init__(self, root: Path, workers: dict[str, Transport]):
        self.root, self.workers = root, workers
        self.peaks = {"baseline": 0, "candidate": 0}

    def __call__(self) -> dict:
        peak = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        collector = int(peak if sys.platform == "darwin" else peak * 1024)
        for build, worker in self.workers.items():
            process = getattr(worker, "process", None)
            if process is None:
                raise ScreenError("rss_unavailable", "external worker process identity missing")
            status = Path(f"/proc/{process.pid}/status")
            if getattr(worker, "closed", False):
                continue
            try:
                text = status.read_text()
            except OSError as exc:
                raise ScreenError("rss_unavailable", "cannot read live worker peak RSS") from exc
            match = re.search(r"^VmHWM:\s+(\d+) kB$", text, re.MULTILINE)
            if not match:
                raise ScreenError("rss_unavailable", "live worker peak RSS unavailable")
            self.peaks[build] = max(self.peaks[build], int(match[1]) * 1024)
        if not self.root.is_dir():
            raise ScreenError("artifact_path", "artifact root is missing")
        total = 0
        def walk_error(exc):
            raise ScreenError("artifact_path", "artifact inventory is unreadable: " + str(exc))
        for directory, dirs, files in os.walk(self.root, followlinks=False, onerror=walk_error):
            for name in dirs + files:
                path = Path(directory) / name
                if path.is_symlink():
                    raise ScreenError("artifact_path", "artifact symlinks are not allowed")
                if name in files:
                    if not path.is_file():
                        raise ScreenError("artifact_path", "nonregular artifact is not allowed")
                    total += path.stat().st_size
                    if total > MAX_ARTIFACTS:
                        raise ScreenError("artifact_cap", "artifact total exceeds 256 MiB")
        return {"collector_rss_bytes": collector, "worker_rss_bytes": dict(self.peaks),
                "artifact_bytes": total}


class JsonlLog:
    def __init__(self, path: Path):
        self.file = path.open("x", encoding="utf-8")

    def __call__(self, record: dict) -> None:
        self.file.write(json.dumps(record, sort_keys=True, separators=(",", ":"), allow_nan=False) + "\n")
        self.file.flush()

    def close(self) -> None:
        self.file.close()


def integer(value: Any, name: str, minimum: int = 0, maximum: int | None = None) -> int:
    if type(value) is not int or value < minimum or (maximum is not None and value > maximum):
        raise ScreenError("malformed_response", f"invalid {name}: {value!r}")
    return value


def hash64(value: Any, name: str) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{16}", value):
        raise ScreenError("malformed_response", f"invalid {name}: {value!r}")
    return value


def summarize_ratios(ratios: list[Fraction], gate: str) -> dict:
    if len(ratios) != 9 or any(x <= 0 for x in ratios):
        raise ScreenError("missing_pairs", "exactly nine positive paired ratios are required")
    median = sorted(ratios)[4]
    if gate == "aa":
        count = sum(Fraction(100, 105) <= x <= Fraction(105, 100) for x in ratios)
        passed = Fraction(100, 103) <= median <= Fraction(103, 100) and count >= 7
    elif gate == "primary":
        count = sum(x < 1 for x in ratios)
        passed = median <= Fraction(100, 105) and count >= 7
    elif gate == "control":
        count = sum(x <= Fraction(105, 100) for x in ratios)
        passed = median <= Fraction(103, 100) and count >= 7
    elif gate == "diagnostic":
        count, passed = None, None
    else:
        raise ValueError("unknown gate")
    return {"gate": gate, "median_ratio": float(median),
            "median_ratio_exact": [median.numerator, median.denominator],
            "qualifying_pairs": count, "passed": passed,
            "ratios": [float(x) for x in ratios],
            "ratios_exact": [[x.numerator, x.denominator] for x in ratios]}


class Collector:
    def __init__(self, workers: dict[str, Transport], clock: Clock,
                 sink: Callable[[dict], None], *, mode: str,
                 verify_sources: Callable[[], None],
                 verify_fixture: Callable[[str, int, int, str, str], Path],
                 resource_snapshot: Callable[[], dict], metadata: dict | None = None,
                 deadline_ns: int | None = None):
        if mode not in ("validate", "synthetic", "campaign"):
            raise ValueError("invalid mode")
        if set(workers) != {"baseline", "candidate"}:
            raise ValueError("exactly baseline and candidate workers required")
        self.workers, self.clock, self.sink, self.mode = workers, clock, sink, mode
        self.verify_sources = verify_sources
        self.verify_fixture = verify_fixture
        self.resource_snapshot = resource_snapshot
        self.inherited_deadline_ns = deadline_ns
        self.deadline_ns: int | None = None
        self.worker_peaks = {"baseline": 0, "candidate": 0}
        self.clock_reads = {"baseline": 0, "candidate": 0}
        self.workers_closed = False
        self.resources: dict = {}
        self.metadata = metadata or {}
        self.started: int | None = None
        self.last_clock: int | None = None
        self.sequence = 0
        self.source_check_failure: dict | None = None
        self.prepared: dict[tuple[str, int, int], dict] = {}
        self.counts: dict[int, int] = {}
        self.counts_hash: str | None = None
        self.summaries: dict[str, dict[int, dict]] = {}
        self.failure: dict | None = None

    def now(self) -> int:
        current = self.clock.now_ns()
        if self.last_clock is not None and current < self.last_clock:
            raise ScreenError("clock_invalid", "monotonic clock moved backwards")
        self.last_clock = current
        return current

    def budget(self) -> int:
        assert self.started is not None
        deadline = self.deadline_ns if self.deadline_ns is not None else self.started + CAMPAIGN_NS
        remaining = deadline - self.now()
        if remaining < 0:
            raise ScreenError("campaign_budget", "360 s campaign wall budget exceeded")
        return remaining

    def check_resources(self) -> None:
        snapshot = self.resource_snapshot()
        own = integer(snapshot.get("collector_rss_bytes"), "collector_rss_bytes")
        artifacts = integer(snapshot.get("artifact_bytes"), "artifact_bytes")
        if own > MAX_COLLECTOR_RSS:
            raise ScreenError("collector_rss_cap", "collector peak RSS exceeds 256 MiB")
        for build in ("baseline", "candidate"):
            peak = integer(snapshot.get("worker_rss_bytes", {}).get(build), "worker_rss_bytes")
            self.worker_peaks[build] = max(peak, self.worker_peaks[build])
            if self.worker_peaks[build] > MAX_RSS:
                raise ScreenError("rss_cap", "worker peak RSS exceeds 1024 MiB")
        if own + sum(self.worker_peaks.values()) > MAX_COMBINED_RSS:
            raise ScreenError("combined_rss_cap", "combined accounted peak RSS exceeds 2304 MiB")
        if artifacts > MAX_ARTIFACTS:
            raise ScreenError("artifact_cap", "artifact total exceeds 256 MiB")
        self.resources = {"collector_rss_bytes": own, "worker_rss_bytes": dict(self.worker_peaks),
                          "combined_rss_bytes": own + sum(self.worker_peaks.values()),
                          "artifact_bytes": artifacts}

    def emit(self, event: str, **payload) -> None:
        self.sequence += 1
        self.sink({"sequence": self.sequence, "event": event, "mode": self.mode,
                   "synthetic": self.mode == "synthetic", **payload})
        self.check_resources()

    def request(self, build: str, command: str, **context) -> dict:
        remaining = self.budget()
        if remaining <= 0:
            raise ScreenError("campaign_budget", "no campaign time remains for another operation")
        timeout = min(WATCHDOG_NS, remaining)
        before = self.now()
        raw = None
        response = None
        error = None
        try:
            raw = self.workers[build].request(command, timeout)
            if not isinstance(raw, str):
                raise ScreenError("malformed_response", "transport must return raw text")
            if len(raw.encode("utf-8")) > MAX_RESPONSE_BYTES:
                raise ScreenError("malformed_response", "response exceeds size bound")
            elapsed = self.now() - before
            if elapsed > timeout:
                code = "watchdog" if remaining >= WATCHDOG_NS else "campaign_budget"
                raise ScreenError(code, "worker response exceeded external deadline")
            try:
                response = strict_json(raw)
            except (ValueError, TypeError) as exc:
                raise ScreenError("malformed_response", str(exc)) from exc
            if not isinstance(response, dict) or type(response.get("ok")) is not bool:
                raise ScreenError("malformed_response", "response must be object with Boolean ok")
            if not response["ok"]:
                raise ScreenError("worker_error", str(response.get("error_code")) + ": " + str(response.get("error")))
            return response
        except ScreenError as exc:
            if exc.code == "watchdog" and remaining < WATCHDOG_NS:
                exc = ScreenError("campaign_budget", "campaign deadline while awaiting worker: " + str(exc))
            error = {"code": exc.code, "message": str(exc)}
            raise exc
        except Exception as exc:
            error = {"code": "worker_transport", "message": str(exc)}
            raise ScreenError("worker_transport", str(exc)) from exc
        finally:
            self.emit("response", build=build, command=command, raw=raw, response=response,
                      error=error, wall_started_ns=before,
                      wall_elapsed_ns=self.now() - before, **context)
            self.budget()

    def prepare(self) -> None:
        for cell in CELLS:
            for seed in SEEDS:
                paths = {}
                for build in ("baseline", "candidate"):
                    r = self.request(build, f"prepare {cell.id} {seed}", phase="prepare", cell=cell.id, seed=seed)
                    expected = {"cell": cell.id, "seed": seed, "type": cell.type,
                                "shape": cell.shape, "n": cell.n, "m": cell.m,
                                "output_length": cell.output_length, "scalar_count": cell.scalar_count,
                                "storage_per_result": cell.storage_per_result}
                    for key, value in expected.items():
                        if type(r.get(key)) is not type(value) or r[key] != value:
                            raise ScreenError("prepare_mismatch", f"{build} cell {cell.id} {key} mismatch")
                    if r.get("small_oracle_passed") is not True:
                        raise ScreenError("oracle_failed", "independent small-tree oracle has not passed")
                    if type(r.get("clock_reads")) is not int or r["clock_reads"] != 0:
                        raise ScreenError("clock_mode_mismatch", "reference preparation read the worker clock")
                    self.worker_peaks[build] = max(self.worker_peaks[build], integer(r.get("peak_rss_bytes"), "peak_rss_bytes"))
                    self.check_resources()
                    integer(r.get("root"), "root", 0, cell.n - 1)
                    saved = integer(r.get("saved_merges"), "saved_merges", 0, cell.n - 2)
                    known_saved = {0: cell.n - 2, 1: 0, 2: cell.n // 2 - 1, 3: cell.n // 2 - 1}
                    if cell.shape in known_saved and saved != known_saved[cell.shape]:
                        raise ScreenError("prepare_mismatch", "independent topology merge reduction mismatch")
                    if "elapsed_ns" in r or "result" in r:
                        raise ScreenError("prepare_mismatch", "prepare must expose neither elapsed timing nor inline output")
                    for kind in ("input", "output"):
                        name = kind + "_hash"
                        r[name] = hash64(r.get(name), name)
                        path = self.verify_fixture(build, cell.id, seed, kind, r.get(kind + "_file"))
                        if fnv64_file(path) != r[name]:
                            raise ScreenError("output_mismatch" if kind == "output" else "input_mismatch",
                                              "canonical file FNV64 differs from prepare metadata")
                        paths[build, kind] = path
                        self.budget()
                    self.prepared[build, cell.id, seed] = r
                a, b = (self.prepared[x, cell.id, seed] for x in ("baseline", "candidate"))
                metadata = lambda r: {k: v for k, v in r.items() if k not in ("input_file", "output_file", "peak_rss_bytes")}
                if metadata(a) != metadata(b):
                    raise ScreenError("output_mismatch", f"baseline/candidate metadata differ for cell {cell.id}, seed {seed}")
                for kind in ("input", "output"):
                    if not identical_files(paths["baseline", kind], paths["candidate", kind]):
                        raise ScreenError("output_mismatch" if kind == "output" else "input_mismatch",
                                          f"baseline/candidate canonical {kind} bytes differ")
                self.emit("full_fixture_comparison", cell=cell.id, seed=seed,
                          input_bytes=paths["baseline", "input"].stat().st_size,
                          output_bytes=paths["baseline", "output"].stat().st_size,
                          logical_values_bound_to_sealed_validation=cell.output_length,
                          scalar_count=cell.scalar_count, full_bytes_equal=True)
        self.emit("prepared", cells=10, seeds=list(SEEDS), builds=2)
        self.budget()

    def check_count(self, cell: int, seed: int, build: str, reps: int) -> int:
        if type(reps) is not int or reps < 1 or reps > MAX_REPS:
            raise ScreenError("repetition_cap", "batch repetitions exceed fixed cap")
        charge = self.prepared[build, cell, seed]["storage_per_result"] * reps
        if charge > MAX_STORAGE:
            raise ScreenError("storage_cap", "retained output charge exceeds 512 MiB before allocation")
        return charge

    def batch(self, build: str, cell: int, seed: int, reps: int, *, phase: str,
              role: str, pair: int | None = None, label: str | None = None,
              untimed: bool = False) -> dict:
        charge = self.check_count(cell, seed, build, reps)
        command = "untimed_batch" if untimed else "batch"
        r = self.request(build, f"{command} {cell} {seed} {reps}", phase=phase, role=role,
                         cell=cell, seed=seed, repetitions=reps, pair=pair, label=label)
        for key, value in (("cell", cell), ("seed", seed), ("repetitions", reps), ("full_outputs_verified", reps),
                           ("logical_values_verified", reps * CELL_BY_ID[cell].output_length)):
            if type(r.get(key)) is not int or r[key] != value:
                raise ScreenError("batch_mismatch", key + " does not match requested batch")
        if type(r.get("synthetic_clock")) is not bool:
            raise ScreenError("malformed_response", "missing synthetic_clock Boolean")
        if r["synthetic_clock"] != (self.mode == "synthetic"):
            raise ScreenError("clock_mode_mismatch", "worker clock mode does not match campaign")
        expected_reads = self.clock_reads[build] + (2 if self.mode == "campaign" and not untimed else 0)
        if type(r.get("clock_reads")) is not int or r["clock_reads"] != expected_reads:
            raise ScreenError("clock_mode_mismatch", "worker clock read counter disagrees with authorized mode")
        self.clock_reads[build] = expected_reads
        storage = integer(r.get("storage_bytes"), "storage_bytes")
        if storage != charge or storage > MAX_STORAGE:
            raise ScreenError("storage_cap", "observed output storage inconsistent with charge or cap")
        self.worker_peaks[build] = max(self.worker_peaks[build], integer(r.get("peak_rss_bytes"), "peak_rss_bytes"))
        self.check_resources()
        if hash64(r.get("output_hash"), "output_hash") != self.prepared[build, cell, seed]["output_hash"]:
            raise ScreenError("output_mismatch", "post-timer full output hash mismatch")
        hash64(r.get("batch_hash"), "batch_hash")
        if untimed:
            if "elapsed_ns" not in r or r["elapsed_ns"] is not None:
                raise ScreenError("clock_mode_mismatch", "untimed validation unexpectedly reported timing")
        else:
            elapsed = integer(r.get("elapsed_ns"), "elapsed_ns", 1)
            if elapsed > MAX_BATCH_NS:
                raise ScreenError("batch_duration_cap", "warmup or timed batch exceeds 2 s")
        self.budget()
        return r

    def measured(self, build: str, cell: int, seed: int, reps: int, *, phase: str,
                 pair: int | None = None, label: str | None = None) -> dict:
        # Warmups are untimed-statistics, not clock-free: their durations enforce
        # the same hard 2 s cap and are kept in raw logs, never in ratio summaries.
        for _ in range(2):
            self.batch(build, cell, seed, reps, phase=phase, role="warmup", pair=pair, label=label)
        return self.batch(build, cell, seed, reps, phase=phase, role="measured", pair=pair, label=label)

    def calibrate(self) -> None:
        for cell in CELLS:
            reps = cell.initial_reps
            while True:
                # Reject an impossible count for any seed before this complete
                # three-seed step. No partial or lower-count favorable retries.
                for seed in SEEDS:
                    for build in ("baseline", "candidate"):
                        self.check_count(cell.id, seed, build, reps)
                durations = [self.measured("baseline", cell.id, seed, reps,
                                           phase="calibration")["elapsed_ns"] for seed in SEEDS]
                self.emit("calibration_step", cell=cell.id, repetitions=reps,
                          elapsed_ns=durations, floor_reached=all(x >= MIN_BATCH_NS for x in durations))
                if all(x >= MIN_BATCH_NS for x in durations):
                    self.counts[cell.id] = reps
                    break
                reps *= 2
        frozen = {str(k): v for k, v in self.counts.items()}
        self.counts_hash = hashlib.sha256(json.dumps(frozen, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
        self.emit("counts_frozen", repetitions=frozen, sha256=self.counts_hash)
        self.budget()

    def phase(self, name: str, cells: tuple[int, ...], aa: bool) -> None:
        if set(self.counts) != set(CELL_BY_ID) or not self.counts_hash:
            raise ScreenError("counts_not_frozen", "all ten counts must be frozen before paired phases")
        self.summaries[name] = {}
        for cell in cells:
            ratios, candidate_durations = [], []
            for pair in range(9):
                seed = SEEDS[pair % 3]
                order = ("A", "B") if pair % 2 == 0 else ("B", "A")
                values = {}
                for label in order:
                    build = "baseline" if aa or label == "A" else "candidate"
                    values[label] = self.measured(build, cell, seed, self.counts[cell],
                                                  phase=name, pair=pair, label=label)["elapsed_ns"]
                ratio = Fraction(values["B"], values["A"])
                ratios.append(ratio)
                candidate_durations.append(values["B"])
                self.emit("pair", phase=name, cell=cell, pair=pair, seed=seed, order="".join(order),
                          repetitions=self.counts[cell], a_elapsed_ns=values["A"], b_elapsed_ns=values["B"],
                          ratio=float(ratio), ratio_exact=[ratio.numerator, ratio.denominator])
            gate = "aa" if aa else "primary" if cell in PRIMARY_CELLS else "control" if cell in CONTROL_CELLS else "diagnostic"
            summary = summarize_ratios(ratios, gate)
            if not aa:
                summary["regression"] = summarize_ratios(ratios, "control")
                summary["passed"] = bool(summary["passed"] and summary["regression"]["passed"])
            summary.update({"b_elapsed_ns": candidate_durations,
                            "b_per_call_ns": [x / self.counts[cell] for x in candidate_durations],
                            "very_short_batch": any(x < SHORT_BATCH_NS for x in candidate_durations),
                            "very_short_note": "Batch <1 ms: limited evidence, informational only; no candidate recalibration."})
            if not aa:
                summary["candidate_elapsed_ns"] = list(candidate_durations)
                summary["candidate_per_call_ns"] = list(summary["b_per_call_ns"])
            self.summaries[name][cell] = summary
            self.emit("cell_summary", phase=name, cell=cell, **summary)
            self.budget()
            if aa and not summary["passed"]:
                raise ScreenError("aa_guard_failed", f"{name} guard failed at cell {cell}")

    def report(self, status: str) -> dict:
        return {"status": status, "mode": self.mode,
                "performance_evidence": self.mode == "campaign" and status in ("passed", "blocked"),
                "measurement": "warm-call throughput with retained trees; caller-side result destruction excluded",
                "campaign_budget_starts_before_prepare": True,
                "inherited_campaign_deadline_ns": self.inherited_deadline_ns,
                "prior_preparation_inherited_ns": (max(0, self.started - (self.inherited_deadline_ns - CAMPAIGN_NS))
                                                   if self.started is not None and self.inherited_deadline_ns is not None else 0),
                "resource_accounting": self.resources,
                "protocol_sha256": PROTOCOL_SHA256, "metadata": self.metadata,
                "counts": self.counts, "counts_sha256": self.counts_hash,
                "summaries": self.summaries, "failure": self.failure,
                "postfailure_source_check": self.source_check_failure,
                # Use a raw diagnostic reading here so a clock-regression error
                # can itself be reported, rather than throwing a second error.
                "campaign_elapsed_ns": self.clock.now_ns() - self.started if self.started is not None else None}

    def close_workers(self) -> None:
        if self.workers_closed:
            return
        self.workers_closed = True
        errors = []
        for build, worker in self.workers.items():
            try:
                worker.close()
            except Exception as exc:
                errors.append(f"{build}: {exc}")
        if errors:
            raise ScreenError("cleanup_failed", "; ".join(errors))

    def run(self) -> dict:
        try:
            self.started = self.now()
            self.deadline_ns = self.started + CAMPAIGN_NS
            if self.inherited_deadline_ns is not None:
                self.deadline_ns = min(self.deadline_ns, self.inherited_deadline_ns)
            self.verify_sources()
            self.budget()
            self.emit("start", protocol_sha256=PROTOCOL_SHA256, metadata=self.metadata,
                      caps={"repetitions": MAX_REPS, "storage_bytes": MAX_STORAGE, "rss_bytes": MAX_RSS,
                            "collector_rss_bytes": MAX_COLLECTOR_RSS, "combined_rss_bytes": MAX_COMBINED_RSS,
                            "artifact_bytes": MAX_ARTIFACTS,
                            "batch_ns": MAX_BATCH_NS, "watchdog_ns": WATCHDOG_NS, "campaign_ns": CAMPAIGN_NS})
            self.prepare()
            if self.mode == "validate":
                for cell in CELLS:
                    for seed in SEEDS:
                        for build in ("baseline", "candidate"):
                            self.batch(build, cell.id, seed, 1, phase="validation", role="untimed", untimed=True)
                status = "validated_untimed"
            else:
                self.calibrate()
                self.phase("aa_begin", AA_CELLS, aa=True)
                self.phase("ab", tuple(CELL_BY_ID), aa=False)
                self.phase("aa_end", AA_CELLS, aa=True)
                if (set(self.summaries["ab"]) != set(CELL_BY_ID)
                        or set(self.summaries["aa_begin"]) != set(AA_CELLS)
                        or set(self.summaries["aa_end"]) != set(AA_CELLS)):
                    raise ScreenError("missing_cells", "campaign is incomplete")
                passed = all(self.summaries["ab"][c]["passed"] for c in PRIMARY_CELLS + CONTROL_CELLS)
                status = "passed" if passed else "blocked"
                if self.mode == "synthetic":
                    status = "synthetic_" + status
            self.close_workers()
            self.verify_sources()
            self.budget()
            result = self.report(status)
            self.emit("outcome", report=result)
            # Includes final raw-log flushing and all source/output postwork.
            self.budget()
            result["campaign_elapsed_ns"] = self.now() - self.started
            return result
        except Exception as exc:
            code = exc.code if isinstance(exc, ScreenError) else "collector_error"
            self.failure = {"code": code, "message": str(exc)}
            # A batch/guard failure remains primary. Also preserve any concurrent
            # source change, without retrying a failed batch or hiding its error.
            try:
                self.close_workers()
            except Exception as cleanup_exc:
                self.failure["cleanup_error"] = str(cleanup_exc)
            if self.started is not None:
                try:
                    self.verify_sources()
                    self.source_check_failure = {"ok": True}
                except Exception as source_exc:
                    self.source_check_failure = {"ok": False,
                                                 "code": getattr(source_exc, "code", "source_check_error"),
                                                 "message": str(source_exc)}
            result = self.report("inconclusive")
            try:
                self.emit("abort", report=result)
            except Exception as logging_exc:
                # Disk/log failure is fatal even if it prevents recording itself.
                # The caller still receives an inconclusive diagnostic on stdout.
                result["failure"]["abort_log_error"] = str(logging_exc)
            return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("validate", "synthetic", "campaign"))
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="new JSONL evidence file; never overwritten")
    parser.add_argument("--authorize-real-timing", action="store_true")
    parser.add_argument("--cpu", type=int, help="predeclared single CPU; default selects lowest available CPU before any sample")
    parser.add_argument("--synthetic-clock-ns", type=int, default=MIN_BATCH_NS)
    args = parser.parse_args(argv)
    if args.mode == "campaign" and not args.authorize_real_timing:
        parser.error("real campaign refused without --authorize-real-timing and reviewed manifest")
    if args.mode != "campaign" and args.authorize_real_timing:
        parser.error("--authorize-real-timing is only valid with campaign")
    if not 1 <= args.synthetic_clock_ns <= MAX_BATCH_NS:
        parser.error("--synthetic-clock-ns must be in [1, 2000000000]")
    workers = {}
    sink = None
    try:
        # Retain rejected manifests and setup failures too. The explicit timing
        # authorization gate above runs before opening any output or worker.
        sink = JsonlLog(args.output)
        manifest = Manifest(args.manifest)
        output = canonical_absolute(str(args.output))
        if not output.is_relative_to(manifest.artifact_root):
            raise ScreenError("artifact_path", "raw evidence must reside below artifact_root")
        inherited_deadline_ns = None
        if args.mode == "campaign" and "REROOTING_CAMPAIGN_DEADLINE_MONOTONIC" in os.environ:
            from decimal import Decimal, InvalidOperation
            try:
                value = Decimal(os.environ["REROOTING_CAMPAIGN_DEADLINE_MONOTONIC"])
                if not value.is_finite() or value <= 0:
                    raise ValueError("deadline must be positive finite monotonic seconds")
                inherited_deadline_ns = int(value * NS)
            except (InvalidOperation, ValueError, OverflowError) as exc:
                raise ScreenError("campaign_budget", "invalid inherited campaign deadline") from exc
        cpu = args.cpu
        if hasattr(os, "sched_getaffinity"):
            lowest = min(os.sched_getaffinity(0))
            if cpu is not None and cpu != lowest:
                raise ScreenError("cpu_affinity", "fixed protocol requires lowest allowed CPU")
            cpu = lowest
        if cpu is not None:
            if type(cpu) is not int or cpu < 0:
                raise ScreenError("cpu_affinity", "cpu must be a nonnegative integer")
            if hasattr(os, "sched_setaffinity"):
                os.sched_setaffinity(0, {cpu})
                if os.sched_getaffinity(0) != {cpu}:
                    raise ScreenError("cpu_affinity", "requested single-CPU affinity was not applied")
            elif args.mode == "campaign":
                raise ScreenError("cpu_affinity", "manifest requested CPU affinity unsupported on this platform")
        elif args.mode == "campaign" and hasattr(os, "sched_setaffinity"):
            raise ScreenError("cpu_affinity", "real campaign requires one predeclared CPU when supported")
        clock = RealClock()
        extra = ["--enable-timing"] if args.mode == "campaign" else ["--synthetic-clock-ns", str(args.synthetic_clock_ns)] if args.mode == "synthetic" else []
        for build in ("baseline", "candidate"):
            workers[build] = ProcessTransport([str(manifest.binary(build)), "--fixtures-root", str(manifest.fixture_roots[build]), *extra],
                                              Path(str(args.output) + "." + build + ".stderr"), clock)
        metadata = {"manifest": str(manifest.path), "manifest_sha256": manifest.sha256,
                    "compiler": manifest.data["compiler"], "cpu_selected": cpu,
                    "cpu_requested": args.cpu, "cpu_affinity_supported": hasattr(os, "sched_setaffinity"),
                    "platform": platform.platform(),
                    "python": sys.version, "synthetic_clock_ns": args.synthetic_clock_ns if args.mode == "synthetic" else None,
                    "real_timing_authorized_flag": args.authorize_real_timing,
                    "source_and_binary_artifacts": manifest.artifacts}
        result = Collector(workers, clock, sink, mode=args.mode,
                           verify_sources=manifest.verify, verify_fixture=manifest.fixture,
                           resource_snapshot=ResourceMonitor(manifest.artifact_root, workers),
                           metadata=metadata, deadline_ns=inherited_deadline_ns).run()
        print(json.dumps(result, sort_keys=True, allow_nan=False))
        return 0 if result["status"] in ("passed", "synthetic_passed", "validated_untimed") else 2
    except (ScreenError, OSError) as exc:
        result = {"status": "inconclusive", "failure": {"code": getattr(exc, "code", "setup_error"), "message": str(exc)}}
        if sink:
            try:
                sink({"event": "setup_failure", **result})
            except OSError as log_exc:
                result["failure"]["logging_error"] = str(log_exc)
        print(json.dumps(result, sort_keys=True), file=sys.stderr)
        return 2
    finally:
        for worker in workers.values():
            worker.close()
        if sink:
            sink.close()


if __name__ == "__main__":
    raise SystemExit(main())
