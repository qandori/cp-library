#!/usr/bin/env python3
"""Synthetic-only collector tests. No worker binary or real timing is invoked."""

from __future__ import annotations

import contextlib
import copy
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import Mock, patch
from fractions import Fraction

import collector as c


class FakeClock:
    def __init__(self):
        self.value = 0

    def now_ns(self):
        return self.value

    def advance(self, ns):
        self.value += ns


class FakeWorld:
    def __init__(self, mode="synthetic"):
        self.mode = mode
        self.clock = FakeClock()
        self.records = []
        self.calls = []
        self.response_hook = None
        self.elapsed_hook = None
        self.wall_hook = None
        self.sink_hook = None
        self.verifications = 0
        self.verify_hook = None
        self.temp = tempfile.TemporaryDirectory()
        unittest.addModuleCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.fixture_hashes = {}
        self.fixture_paths = {}
        for build in ("baseline", "candidate"):
            (self.root / build).mkdir()
            for cell in c.CELLS:
                for seed in c.SEEDS:
                    for kind in ("input", "output"):
                        path = self.root / build / c.fixture_name(cell.id, seed, kind)
                        path.write_bytes(f"fake canonical {kind} {cell.id} {seed}".encode())
                        self.fixture_paths[build, cell.id, seed, kind] = path
                        self.fixture_hashes[str(path)] = c.digest(path)
        self.resources = {"collector_rss_bytes": 8 * 1024 * 1024,
                          "worker_rss_bytes": {"baseline": 10 * 1024 * 1024, "candidate": 10 * 1024 * 1024},
                          "artifact_bytes": 1000}
        self.workers = {name: FakeWorker(self, name) for name in ("baseline", "candidate")}
        self.collector = c.Collector(self.workers, self.clock, self.sink, mode=mode,
                                     verify_sources=self.verify, verify_fixture=self.verify_fixture,
                                     resource_snapshot=lambda: self.resources, metadata={"fake_only": True})

    def verify_fixture(self, build, cell, seed, kind, path):
        expected = self.fixture_paths[build, cell, seed, kind]
        if path != str(expected) or str(Path(path).resolve()) != path:
            raise c.ScreenError("fixture_path", "worker fixture outside sealed inventory")
        if c.digest(expected) != self.fixture_hashes[path]:
            raise c.ScreenError("source_hash_changed", "sealed fixture changed")
        return expected

    def sink(self, record):
        # JSON roundtrip reflects durable logging, rather than retaining mutable
        # response objects that later validations could alter.
        self.records.append(json.loads(json.dumps(record)))
        if self.sink_hook:
            self.sink_hook(record)

    def verify(self):
        self.verifications += 1
        if self.verify_hook:
            self.verify_hook(self.verifications)

    def prepare_only(self):
        self.collector.started = self.clock.now_ns()
        self.collector.prepare()
        return self.collector

    def response_records(self):
        return [x for x in self.records if x["event"] == "response"]

    def timed_calls(self):
        return [x for x in self.calls if x["command"] == "batch"]


class FakeWorker:
    def __init__(self, world, build):
        self.world, self.build = world, build
        self.closed = 0
        self.clock_reads = 0

    def request(self, command, timeout_ns):
        tokens = command.split()
        cmd, cell_id, seed = tokens[0], int(tokens[1]), int(tokens[2])
        cell = c.CELL_BY_ID[cell_id]
        reps = int(tokens[3]) if len(tokens) == 4 else None
        event = {"build": self.build, "command": cmd, "cell": cell_id, "seed": seed,
                 "reps": reps, "timeout_ns": timeout_ns}
        self.world.calls.append(event)
        charge = cell.storage_per_result
        output_path = self.world.fixture_paths[self.build, cell_id, seed, "output"]
        output_hash = c.fnv64_file(output_path)
        if cmd == "prepare":
            input_path = self.world.fixture_paths[self.build, cell_id, seed, "input"]
            saved = {0: cell.n - 2, 1: 0, 2: cell.n // 2 - 1, 3: cell.n // 2 - 1, 4: 0}[cell.shape]
            r = {"ok": True, "cell": cell_id, "seed": seed,
                 "type": cell.type, "shape": cell.shape, "n": cell.n, "m": cell.m,
                 "root": 0, "saved_merges": saved, "output_length": cell.output_length,
                 "scalar_count": cell.scalar_count, "input_hash": c.fnv64_file(input_path),
                 "output_hash": output_hash, "input_file": str(input_path), "output_file": str(output_path),
                 "storage_per_result": charge, "small_oracle_passed": True,
                 "peak_rss_bytes": 10 * 1024 * 1024, "clock_reads": 0}
        else:
            elapsed = c.MIN_BATCH_NS * reps // cell.initial_reps
            if self.build == "candidate" and cell_id in c.PRIMARY_CELLS:
                elapsed = elapsed * 4 // 5
            if self.world.elapsed_hook:
                elapsed = self.world.elapsed_hook(event, elapsed)
            if self.world.mode == "campaign" and cmd == "batch":
                self.clock_reads += 2
            r = {"ok": True, "cell": cell_id, "seed": seed, "repetitions": reps,
                 "clock_reads": self.clock_reads,
                 "elapsed_ns": elapsed if cmd == "batch" else None,
                 "synthetic_clock": self.world.mode == "synthetic", "storage_bytes": charge * reps,
                 "peak_rss_bytes": 10 * 1024 * 1024, "output_hash": output_hash,
                 "batch_hash": "1234567890abcdef", "full_outputs_verified": reps,
                 "logical_values_verified": reps * cell.output_length}
        if self.world.wall_hook:
            self.world.clock.advance(self.world.wall_hook(event, r))
        if self.world.response_hook:
            r = self.world.response_hook(event, r)
        return r if isinstance(r, str) else json.dumps(r)

    def close(self):
        self.closed += 1


class ScheduleTests(unittest.TestCase):
    def test_full_schedule_and_frozen_counts(self):
        w = FakeWorld()
        result = w.collector.run()
        self.assertEqual(result["status"], "synthetic_passed")
        self.assertFalse(result["performance_evidence"])
        self.assertEqual(w.verifications, 2)
        self.assertEqual(result["counts"], {x.id: x.initial_reps for x in c.CELLS})
        self.assertEqual(len(result["counts_sha256"]), 64)
        self.assertEqual([x["event"] for x in w.records].count("counts_frozen"), 1)
        responses = w.response_records()
        self.assertEqual(sum(x["phase"] == "prepare" for x in responses), 60)
        calibration = [x for x in responses if x["phase"] == "calibration"]
        self.assertEqual(len(calibration), 10 * 3 * 3)
        self.assertTrue(all(x["build"] == "baseline" for x in calibration))
        for k, cell in enumerate(c.CELLS):
            block = calibration[k * 9:(k + 1) * 9]
            self.assertEqual([x["seed"] for x in block], [s for s in c.SEEDS for _ in range(3)])
        for phase, cells in (("aa_begin", c.AA_CELLS), ("ab", tuple(range(1, 11))), ("aa_end", c.AA_CELLS)):
            pairs = [x for x in w.records if x["event"] == "pair" and x["phase"] == phase]
            self.assertEqual(len(pairs), len(cells) * 9)
            self.assertEqual([x["cell"] for x in pairs], [cell for cell in cells for _ in range(9)])
            for k, pair in enumerate(pairs):
                p = k % 9
                self.assertEqual(pair["pair"], p)
                self.assertEqual(pair["seed"], c.SEEDS[p % 3])
                self.assertEqual(pair["order"], "AB" if p % 2 == 0 else "BA")
            batches = [x for x in responses if x["phase"] == phase]
            self.assertEqual(len(batches), len(cells) * 9 * 2 * 3)
            if phase != "ab":
                self.assertTrue(all(x["build"] == "baseline" for x in batches))
        timed = [x for x in responses if x["command"].startswith("batch ")]
        self.assertEqual(len(timed), 1062)
        for i in range(0, len(timed), 3):
            group = timed[i:i + 3]
            self.assertEqual([x["role"] for x in group], ["warmup", "warmup", "measured"])
            for key in ("command", "build", "cell", "seed", "repetitions", "phase", "pair", "label"):
                self.assertEqual(len({x[key] for x in group}), 1)
        for x in w.workers.values():
            self.assertEqual(x.closed, 1)

    def test_floor_100ms_inclusive_and_all_three_seeds_repeated_on_doubling(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: t - 1 if e["build"] == "baseline" and e["cell"] == 1 and e["seed"] == c.SEEDS[1] and e["reps"] == 1 else t
        r = w.collector.run()
        self.assertEqual(r["status"], "synthetic_passed")
        self.assertEqual(r["counts"][1], 2)
        self.assertTrue(all(r["counts"][x.id] == x.initial_reps for x in c.CELLS[1:]))
        steps = [x for x in w.records if x["event"] == "calibration_step" and x["cell"] == 1]
        self.assertEqual([x["repetitions"] for x in steps], [1, 2])
        batches = [x for x in w.response_records() if x["phase"] == "calibration" and x["cell"] == 1]
        self.assertEqual([x["seed"] for x in batches], [s for _ in range(2) for s in c.SEEDS for _ in range(3)])
        self.assertTrue(all(x["build"] == "baseline" for x in batches))

    def test_candidate_short_batch_is_informational_not_recalibration(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: 999_999 if e["build"] == "candidate" and e["cell"] in (1, 4) else t
        r = w.collector.run()
        self.assertEqual(r["status"], "synthetic_passed")
        for cell in (1, 4):
            self.assertTrue(r["summaries"]["ab"][cell]["very_short_batch"])
            self.assertEqual(r["summaries"]["ab"][cell]["b_elapsed_ns"], [999_999] * 9)
            self.assertEqual(r["counts"][cell], 1)
        self.assertFalse(any(x["build"] == "candidate" for x in w.response_records() if x["phase"] == "calibration"))

    def test_untimed_validation_has_no_batch_or_calibration(self):
        w = FakeWorld("validate")
        r = w.collector.run()
        self.assertEqual(r["status"], "validated_untimed")
        self.assertFalse(r["performance_evidence"])
        self.assertEqual(len(w.calls), 120)
        self.assertEqual(len(w.timed_calls()), 0)
        self.assertEqual(sum(x["command"] == "untimed_batch" and x["reps"] == 1 for x in w.calls), 60)


class BoundaryTests(unittest.TestCase):
    def batch(self, w, mutate=None, reps=1):
        collector = w.prepare_only()
        if mutate:
            w.response_hook = lambda e, r: mutate(r)
        return collector.batch("baseline", 1, c.SEEDS[0], reps, phase="test", role="measured")

    def test_2s_inclusive(self):
        w = FakeWorld()
        self.assertEqual(self.batch(w, lambda r: {**r, "elapsed_ns": c.MAX_BATCH_NS})["elapsed_ns"], c.MAX_BATCH_NS)
        with self.assertRaisesRegex(c.ScreenError, "exceeds 2 s"):
            self.batch(FakeWorld(), lambda r: {**r, "elapsed_ns": c.MAX_BATCH_NS + 1})

    def test_warmup_2s_failure_stops_without_measured_retry(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: c.MAX_BATCH_NS + 1
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "batch_duration_cap")
        self.assertEqual(len(w.timed_calls()), 1)
        self.assertEqual(w.response_records()[-1]["role"], "warmup")

    def test_5s_external_watchdog_inclusive_response_and_overrun(self):
        w = FakeWorld()
        w.wall_hook = lambda e, r: c.WATCHDOG_NS if e["command"] == "batch" else 0
        self.batch(w)
        w = FakeWorld()
        w.wall_hook = lambda e, r: c.WATCHDOG_NS + 1 if e["command"] == "batch" else 0
        with self.assertRaises(c.ScreenError) as caught:
            self.batch(w)
        self.assertEqual(caught.exception.code, "watchdog")
        self.assertIsNotNone(w.response_records()[-1]["raw"])

    def test_transport_watchdog_error_preserved_and_no_retry(self):
        w = FakeWorld()
        def fail(e, r):
            if e["command"] == "batch":
                raise c.ScreenError("watchdog", "stuck; partial={")
            return r
        w.response_hook = fail
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "watchdog")
        self.assertEqual(len(w.timed_calls()), 1)
        self.assertIn("partial={", w.response_records()[-1]["error"]["message"])

    def test_360s_inclusive_final_logging_then_exceeded(self):
        for elapsed, status in ((c.CAMPAIGN_NS, "synthetic_passed"), (c.CAMPAIGN_NS + 1, "inconclusive")):
            w = FakeWorld()
            w.sink_hook = lambda record, world=w, ns=elapsed: world.clock.advance(ns) if record["event"] == "outcome" else None
            result = w.collector.run()
            self.assertEqual(result["status"], status)
            if status == "inconclusive":
                self.assertEqual(result["failure"]["code"], "campaign_budget")
                self.assertEqual(w.records[-1]["event"], "abort")

    def test_campaign_budget_counts_warmup_wall_cost(self):
        w = FakeWorld()
        # 3 s per response is below external 5 s, but eventually exhausts the
        # campaign even though worker-reported synthetic elapsed is 100 ms.
        w.wall_hook = lambda e, r: 3 * c.NS if e["command"] == "batch" else 0
        r = w.collector.run()
        self.assertEqual(r["status"], "inconclusive")
        self.assertEqual(r["failure"]["code"], "campaign_budget")
        self.assertEqual(len(w.timed_calls()), 120)
        self.assertEqual(r["campaign_elapsed_ns"], c.CAMPAIGN_NS)
        self.assertGreater(sum(x["role"] == "warmup" for x in w.response_records() if x["phase"] != "prepare"), 70)

    def test_campaign_budget_counts_prepare_and_postwork(self):
        w = FakeWorld()
        w.wall_hook = lambda e, r: c.WATCHDOG_NS + 1 if e["command"] == "prepare" else 0
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "watchdog")
        self.assertEqual(len(w.timed_calls()), 0)
        w = FakeWorld()
        w.verify_hook = lambda count: w.clock.advance(c.CAMPAIGN_NS + 1) if count == 2 else None
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "campaign_budget")
        self.assertEqual(len(r["summaries"]["aa_end"]), 4)

    def test_remaining_campaign_deadline_is_tighter_than_watchdog(self):
        w = FakeWorld()
        collector = w.prepare_only()
        w.clock.advance(c.CAMPAIGN_NS - c.NS)
        def fail(e, r):
            self.assertEqual(e["timeout_ns"], c.NS)
            raise c.ScreenError("watchdog", "transport reached supplied deadline")
        w.response_hook = fail
        with self.assertRaises(c.ScreenError) as caught:
            collector.batch("baseline", 1, c.SEEDS[0], 1, phase="deadline", role="warmup")
        self.assertEqual(caught.exception.code, "campaign_budget")

    def test_repetition_and_planned_memory_caps(self):
        w = FakeWorld()
        collector = w.prepare_only()
        collector.prepared["baseline", 1, c.SEEDS[0]]["storage_per_result"] = 1
        self.assertEqual(collector.check_count(1, c.SEEDS[0], "baseline", c.MAX_REPS), c.MAX_REPS)
        with self.assertRaises(c.ScreenError) as caught:
            collector.check_count(1, c.SEEDS[0], "baseline", c.MAX_REPS + 1)
        self.assertEqual(caught.exception.code, "repetition_cap")
        collector.prepared["baseline", 1, c.SEEDS[0]]["storage_per_result"] = c.MAX_STORAGE // c.MAX_REPS
        self.assertEqual(collector.check_count(1, c.SEEDS[0], "baseline", c.MAX_REPS), c.MAX_STORAGE)
        collector.prepared["baseline", 1, c.SEEDS[0]]["storage_per_result"] = c.MAX_STORAGE // c.MAX_REPS + 1
        with self.assertRaises(c.ScreenError) as caught:
            collector.check_count(1, c.SEEDS[0], "baseline", c.MAX_REPS)
        self.assertEqual(caught.exception.code, "storage_cap")
        self.assertEqual(len(w.timed_calls()), 0)

    def test_observed_storage_and_rss_boundaries(self):
        w = FakeWorld()
        collector = w.prepare_only()
        collector.prepared["baseline", 1, c.SEEDS[0]]["storage_per_result"] = c.MAX_STORAGE
        w.response_hook = lambda e, r: {**r, "storage_bytes": c.MAX_STORAGE, "peak_rss_bytes": c.MAX_RSS}
        collector.batch("baseline", 1, c.SEEDS[0], 1, phase="boundary", role="measured")
        for field, value, code in (("storage_bytes", c.MAX_STORAGE + 1, "storage_cap"),
                                   ("storage_bytes", 0, "storage_cap"),
                                   ("storage_bytes", c.CELLS[0].storage_per_result + 1, "storage_cap"),
                                   ("peak_rss_bytes", c.MAX_RSS + 1, "rss_cap")):
            with self.subTest(field=field, value=value), self.assertRaises(c.ScreenError) as caught:
                self.batch(FakeWorld(), lambda r: {**r, field: value})
            self.assertEqual(caught.exception.code, code)

    def test_floor_unreachable_under_storage_cap_no_downward_retry(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: 1
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "repetition_cap")
        steps = [x["repetitions"] for x in w.records if x["event"] == "calibration_step"]
        self.assertEqual(steps, [1, 2, 4, 8, 16, 32, 64, 128])
        self.assertEqual(len(w.timed_calls()), 9 * len(steps))
        self.assertTrue(all(x["build"] == "baseline" and x["cell"] == 1 for x in w.timed_calls()))


class FailureTests(unittest.TestCase):
    def test_malformed_responses_raw_retained_no_retry(self):
        for raw in ("{", '{"ok":true,"ok":false}', '{"ok":NaN}', "[]", '{"ok":1}'):
            with self.subTest(raw=raw):
                w = FakeWorld()
                w.response_hook = lambda e, r: raw
                result = w.collector.run()
                self.assertEqual(result["failure"]["code"], "malformed_response")
                self.assertEqual(len(w.calls), 1)
                self.assertEqual(w.response_records()[0]["raw"], raw)

    def test_prepare_metadata_and_paths(self):
        for field, value, code in (("shape", 9, "prepare_mismatch"),
                                   ("saved_merges", 0, "prepare_mismatch"),
                                   ("scalar_count", 1, "prepare_mismatch"),
                                   ("output_length", True, "prepare_mismatch"),
                                   ("type", "int64_t", "prepare_mismatch"),
                                   ("root", -1, "malformed_response"),
                                   ("storage_per_result", 1, "prepare_mismatch"),
                                   ("input_hash", "not-hex", "malformed_response"),
                                   ("output_hash", "1", "malformed_response"),
                                   ("output_file", "/tmp/not-sealed.bin", "fixture_path"),
                                   ("elapsed_ns", None, "prepare_mismatch")):
            with self.subTest(field=field):
                w = FakeWorld()
                w.response_hook = lambda e, r: {**r, field: value}
                result = w.collector.run()
                self.assertEqual(result["failure"]["code"], code)
                self.assertEqual(len(w.calls), 1)

    def test_baseline_candidate_full_output_mismatch_even_with_matching_hash(self):
        w = FakeWorld()
        path = w.fixture_paths["candidate", 1, c.SEEDS[0], "output"]
        path.write_bytes(b"different exact full output")
        w.fixture_hashes[str(path)] = c.digest(path)
        # Deliberately stub the weak hash to simulate a collision. Exact bytes
        # must still be compared, not accepted from equality of hash metadata.
        with patch.object(c, "fnv64_file", return_value="0123456789abcdef"):
            r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "output_mismatch")
        self.assertEqual(len(w.calls), 2)

    def test_changed_sealed_fixture_stops_before_timing(self):
        w = FakeWorld()
        path = w.fixture_paths["baseline", 1, c.SEEDS[0], "input"]
        path.write_bytes(b"changed after seal")
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "source_hash_changed")
        self.assertEqual(len(w.timed_calls()), 0)

    def test_post_timer_error_and_full_count_mismatch(self):
        for replacement, code in (({"ok": False, "error_code": "post_timer_mismatch", "error": "oracle differs"}, "worker_error"),
                                  ({"full_outputs_verified": 15}, "batch_mismatch"),
                                  ({"output_hash": "0000000000000001"}, "output_mismatch"),
                                  ({"synthetic_clock": False}, "clock_mode_mismatch"),
                                  ({"elapsed_ns": True}, "malformed_response"),
                                  ({"elapsed_ns": 0}, "malformed_response")):
            with self.subTest(replacement=replacement):
                w = FakeWorld()
                w.response_hook = lambda e, r: {**r, **replacement} if e["command"] == "batch" else r
                result = w.collector.run()
                self.assertEqual(result["failure"]["code"], code)
                self.assertEqual(len(w.timed_calls()), 1)

    def test_aa_begin_and_end_failure_are_terminal(self):
        for phase in ("aa_begin", "aa_end"):
            with self.subTest(phase=phase):
                w = FakeWorld()
                def elapsed(e, default):
                    previous = w.response_records()[-1]
                    # Measured responses follow two warmups with the same phase
                    # and label, so the transport need not know collector internals.
                    if previous.get("phase") == phase and previous.get("label") == "B" and e["cell"] == 1:
                        return default * 11 // 10
                    return default
                w.elapsed_hook = elapsed
                result = w.collector.run()
                self.assertEqual(result["failure"]["code"], "aa_guard_failed")
                self.assertEqual(len(result["summaries"][phase]), 1)
                self.assertEqual(len(result["summaries"].get("ab", {})), 0 if phase == "aa_begin" else 10)
                self.assertEqual(result["status"], "inconclusive")

    def test_control_failure_blocks_but_retains_complete_drift_guard(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: t * 11 // 10 if e["build"] == "candidate" and e["cell"] == 5 else t
        result = w.collector.run()
        self.assertEqual(result["status"], "synthetic_blocked")
        self.assertFalse(result["summaries"]["ab"][5]["passed"])
        self.assertEqual(len(result["summaries"]["aa_end"]), 4)
        self.assertEqual(len(w.timed_calls()), 1062)

    def test_source_failure_before_start_and_after_postwork(self):
        for when in (1, 2):
            w = FakeWorld()
            def verify(count):
                if count == when:
                    raise c.ScreenError("source_hash_changed", "changed source")
            w.verify_hook = verify
            r = w.collector.run()
            self.assertEqual(r["failure"]["code"], "source_hash_changed")
            self.assertEqual(len(w.calls), 0 if when == 1 else 1122)

    def test_source_verification_on_failed_batch_preserves_both_failures(self):
        w = FakeWorld()
        w.elapsed_hook = lambda e, t: c.MAX_BATCH_NS + 1
        def verify(count):
            if count == 2:
                raise c.ScreenError("source_hash_changed", "changed during failed warmup")
        w.verify_hook = verify
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "batch_duration_cap")
        self.assertEqual(r["postfailure_source_check"]["code"], "source_hash_changed")
        self.assertEqual(len(w.timed_calls()), 1)

    def test_real_mode_rejects_synthetic_worker(self):
        w = FakeWorld("campaign")
        w.response_hook = lambda e, r: {**r, "synthetic_clock": True} if e["command"] == "batch" else r
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "clock_mode_mismatch")

    def test_monotonic_clock_regression_is_inconclusive(self):
        w = FakeWorld()
        w.wall_hook = lambda e, r: -1
        r = w.collector.run()
        self.assertEqual(r["status"], "inconclusive")
        self.assertEqual(r["failure"]["code"], "clock_invalid")

    def test_logging_failure_cannot_report_success(self):
        w = FakeWorld()
        def fail(record):
            if record["event"] in ("outcome", "abort"):
                raise OSError("simulated disk full")
        w.sink_hook = fail
        r = w.collector.run()
        self.assertEqual(r["status"], "inconclusive")
        self.assertIn("disk full", r["failure"]["abort_log_error"])

    def test_missing_cell_is_inconclusive_without_imputation(self):
        w = FakeWorld()
        phase = w.collector.phase
        def missing(name, cells, aa):
            phase(name, cells, aa)
            if name == "ab":
                del w.collector.summaries[name][10]
        w.collector.phase = missing
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "missing_cells")
        self.assertEqual(len(r["summaries"]["ab"]), 9)

    def test_process_transport_close_is_bounded_idempotent_and_kills_group(self):
        transport = object.__new__(c.ProcessTransport)
        transport.closed = False
        transport.process = Mock()
        transport.process.pid = 123456
        transport.selector = Mock()
        transport.stderr = io.BytesIO()
        transport.process.stdin = io.BytesIO()
        transport.process.stdout = io.BytesIO()
        with patch.object(c.os, "killpg") as kill:
            transport.close()
            transport.close()
            kill.assert_called_once_with(123456, c.signal.SIGKILL)
        transport.process.wait.assert_called_once_with(timeout=1)
        self.assertTrue(transport.stderr.closed)
        self.assertTrue(transport.process.stdin.closed)
        self.assertTrue(transport.process.stdout.closed)


class RerootingSpecificTests(unittest.TestCase):
    def test_fixed_cells_shapes_caps_and_all_regression_gates(self):
        self.assertEqual(c.AA_CELLS, (1, 2, 6, 7))
        self.assertEqual(c.PRIMARY_CELLS, (6, 8))
        self.assertEqual(c.CONTROL_CELLS, tuple(range(1, 11)))
        self.assertEqual(c.CAMPAIGN_NS, 360 * c.NS)
        self.assertEqual(c.MAX_REPS, 128)
        self.assertEqual(c.MAX_STORAGE, 512 << 20)
        self.assertEqual(c.MAX_RSS, 1024 << 20)
        self.assertEqual(c.MAX_COMBINED_RSS, 2304 << 20)
        self.assertEqual(c.MAX_COLLECTOR_RSS, 256 << 20)
        self.assertEqual(c.MAX_ARTIFACTS, 256 << 20)
        self.assertEqual(c.MIN_BATCH_NS, 100_000_000)
        self.assertEqual(c.MAX_BATCH_NS, 2 * c.NS)
        self.assertEqual(c.WATCHDOG_NS, 5 * c.NS)
        self.assertEqual([x.initial_reps for x in c.CELLS], [1] * 10)
        self.assertEqual([x.shape for x in c.CELLS], list(range(5)) * 2)
        for cell in c.CELLS:
            self.assertEqual(cell.storage_per_result, 256 + 2 * (8 * cell.m + cell.output_length * cell.element_bytes))
        w = FakeWorld()
        r = w.collector.run()
        for cell in range(1, 11):
            self.assertEqual(r["summaries"]["ab"][cell]["regression"]["gate"], "control")
        responses = [x for x in w.response_records() if x["phase"] in ("aa_begin", "ab", "aa_end")]
        self.assertEqual(sum(x["role"] == "measured" for x in responses), 324)
        self.assertEqual(sum(x["role"] == "warmup" for x in responses), 648)
        self.assertTrue(all(x["timeout_ns"] <= c.WATCHDOG_NS for x in w.calls))

    def test_every_prepare_request_has_five_second_watchdog(self):
        w = FakeWorld()
        w.prepare_only()
        self.assertEqual({x["timeout_ns"] for x in w.calls}, {c.WATCHDOG_NS})

    def test_independent_oracle_and_zero_clock_reads_required(self):
        for field, value, code in (("small_oracle_passed", False, "oracle_failed"),
                                   ("clock_reads", 1, "clock_mode_mismatch"),
                                   ("peak_rss_bytes", c.MAX_RSS + 1, "rss_cap")):
            w = FakeWorld()
            w.response_hook = lambda e, r: {**r, field: value}
            result = w.collector.run()
            self.assertEqual(result["failure"]["code"], code)
            self.assertEqual(len(w.timed_calls()), 0)

    def test_runtime_clock_reads_and_logical_count_required(self):
        for field, value, code in (("clock_reads", 1, "clock_mode_mismatch"),
                                   ("logical_values_verified", 1, "batch_mismatch"),
                                   ("full_outputs_verified", 0, "batch_mismatch")):
            w = FakeWorld()
            w.response_hook = lambda e, r: {**r, field: value} if e["command"] == "batch" else r
            result = w.collector.run()
            self.assertEqual(result["failure"]["code"], code)
            self.assertEqual(len(w.timed_calls()), 1)

    def test_build_rss_may_differ_without_output_mismatch(self):
        w = FakeWorld("validate")
        w.response_hook = lambda e, r: {**r, "peak_rss_bytes": 20 * 1024 * 1024} if e["build"] == "candidate" else r
        self.assertEqual(w.collector.run()["status"], "validated_untimed")

    def test_collector_artifact_and_external_worker_caps(self):
        for field, value, code in (("collector_rss_bytes", c.MAX_COLLECTOR_RSS + 1, "collector_rss_cap"),
                                   ("artifact_bytes", c.MAX_ARTIFACTS + 1, "artifact_cap"),
                                   ("worker_rss_bytes", {"baseline": c.MAX_RSS + 1, "candidate": 0}, "rss_cap")):
            w = FakeWorld()
            w.resources[field] = value
            r = w.collector.run()
            self.assertEqual(r["failure"]["code"], code)
            self.assertEqual(w.calls, [])
            self.assertTrue(all(x.closed == 1 for x in w.workers.values()))

    def test_all_resource_boundaries_inclusive_and_combined_enforced(self):
        w = FakeWorld()
        w.resources = {"collector_rss_bytes": c.MAX_COLLECTOR_RSS,
                       "worker_rss_bytes": {"baseline": c.MAX_RSS, "candidate": c.MAX_RSS},
                       "artifact_bytes": c.MAX_ARTIFACTS}
        w.collector.check_resources()
        self.assertEqual(w.collector.resources["combined_rss_bytes"], c.MAX_COMBINED_RSS)
        with patch.object(c, "MAX_COMBINED_RSS", c.MAX_COMBINED_RSS - 1), self.assertRaises(c.ScreenError) as err:
            w.collector.check_resources()
        self.assertEqual(err.exception.code, "combined_rss_cap")

    def test_combined_cap_keeps_both_historical_worker_peaks(self):
        w = FakeWorld()
        w.resources = {"collector_rss_bytes": c.MAX_COLLECTOR_RSS,
                       "worker_rss_bytes": {"baseline": c.MAX_RSS, "candidate": 0},
                       "artifact_bytes": c.MAX_ARTIFACTS}
        w.collector.check_resources()
        w.resources["worker_rss_bytes"] = {"baseline": 0, "candidate": c.MAX_RSS}
        w.collector.check_resources()
        self.assertEqual(w.collector.resources["worker_rss_bytes"],
                         {"baseline": c.MAX_RSS, "candidate": c.MAX_RSS})
        self.assertEqual(w.collector.resources["combined_rss_bytes"], 2304 << 20)
        with patch.object(c, "MAX_COMBINED_RSS", (2304 << 20) - 1), self.assertRaises(c.ScreenError) as err:
            w.collector.check_resources()
        self.assertEqual(err.exception.code, "combined_rss_cap")

    def test_artifact_growth_during_logging_invalidates_outcome(self):
        w = FakeWorld()
        def grow(record):
            if record["event"] == "outcome":
                w.resources["artifact_bytes"] = c.MAX_ARTIFACTS + 1
        w.sink_hook = grow
        r = w.collector.run()
        self.assertEqual(r["status"], "inconclusive")
        self.assertEqual(r["failure"]["code"], "artifact_cap")

    def test_real_artifact_counter_sparse_overrun_and_symlink(self):
        with tempfile.TemporaryDirectory() as root:
            monitor = c.ResourceMonitor(Path(root), {})
            f = Path(root) / "sparse"
            with f.open("wb") as stream:
                stream.truncate(c.MAX_ARTIFACTS + 1)
            with self.assertRaises(c.ScreenError) as err:
                monitor()
            self.assertEqual(err.exception.code, "artifact_cap")
            f.unlink()
            f.symlink_to("/tmp")
            with self.assertRaises(c.ScreenError) as err:
                monitor()
            self.assertEqual(err.exception.code, "artifact_path")

    def test_inherited_deadline_includes_prior_preparation(self):
        w = FakeWorld()
        w.clock.advance(350 * c.NS)
        w.collector.inherited_deadline_ns = 360 * c.NS
        w.wall_hook = lambda e, r: 2 * c.NS
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "campaign_budget")
        self.assertEqual(len(w.calls), 5)
        self.assertEqual(len(w.timed_calls()), 0)

    def test_cleanup_precedes_final_hash_check_and_counts_toward_budget(self):
        w = FakeWorld()
        def check(count):
            if count == 2:
                self.assertTrue(all(x.closed == 1 for x in w.workers.values()))
        w.verify_hook = check
        original = w.workers["baseline"].close
        def slow_close():
            original()
            w.clock.advance(c.CAMPAIGN_NS + 1)
        w.workers["baseline"].close = slow_close
        r = w.collector.run()
        self.assertEqual(r["failure"]["code"], "campaign_budget")
        self.assertTrue(all(x.closed == 1 for x in w.workers.values()))

    def test_cleanup_failure_still_closes_other_worker_and_checks_sources(self):
        w = FakeWorld()
        def fail_close():
            w.workers["baseline"].closed += 1
            raise RuntimeError("simulated bounded wait failure")
        w.workers["baseline"].close = fail_close
        result = w.collector.run()
        self.assertEqual(result["failure"]["code"], "cleanup_failed")
        self.assertEqual(w.workers["candidate"].closed, 1)
        self.assertGreaterEqual(w.verifications, 2)

    def test_missing_duplicate_and_oversized_response_fail_closed(self):
        for raw in ('{"ok":true}\n{"ok":true}', "", "x" * (c.MAX_RESPONSE_BYTES + 1)):
            w = FakeWorld()
            w.response_hook = lambda e, r: raw
            r = w.collector.run()
            self.assertEqual(r["failure"]["code"], "malformed_response")
            self.assertEqual(len(w.calls), 1)

    def test_fnv64_standard_vectors_and_exact_stream_comparison(self):
        with tempfile.TemporaryDirectory() as root:
            a, b = Path(root) / "a", Path(root) / "b"
            a.write_bytes(b"")
            self.assertEqual(c.fnv64_file(a), "cbf29ce484222325")
            a.write_bytes(b"a")
            self.assertEqual(c.fnv64_file(a), "af63dc4c8601ec8c")
            b.write_bytes(b"b")
            self.assertFalse(c.identical_files(a, b))
            b.write_bytes(b"a")
            self.assertTrue(c.identical_files(a, b))


class GateTests(unittest.TestCase):
    def test_median_is_paired_median_and_keeps_outliers(self):
        ratios = [Fraction(1, 2)] * 4 + [Fraction(9, 10)] * 3 + [Fraction(100)] * 2
        r = c.summarize_ratios(ratios, "primary")
        self.assertEqual(r["median_ratio_exact"], [9, 10])
        self.assertEqual(r["qualifying_pairs"], 7)
        self.assertTrue(r["passed"])
        self.assertEqual(len(r["ratios"]), 9)

    def test_primary_exact_boundary_and_strict_seven_under_one(self):
        self.assertTrue(c.summarize_ratios([Fraction(100, 105)] * 7 + [Fraction(1)] * 2, "primary")["passed"])
        self.assertFalse(c.summarize_ratios([Fraction(100, 105)] * 6 + [Fraction(1)] * 3, "primary")["passed"])
        self.assertFalse(c.summarize_ratios([Fraction(100, 105) + Fraction(1, 10**12)] * 9, "primary")["passed"])

    def test_control_inclusive_bounds(self):
        good = [Fraction(103, 100)] * 5 + [Fraction(105, 100)] * 2 + [Fraction(2)] * 2
        self.assertTrue(c.summarize_ratios(good, "control")["passed"])
        self.assertFalse(c.summarize_ratios([Fraction(103, 100)] * 5 + [Fraction(105, 100)] + [Fraction(2)] * 3, "control")["passed"])
        self.assertFalse(c.summarize_ratios([Fraction(103, 100) + Fraction(1, 10**12)] * 9, "control")["passed"])

    def test_aa_both_exact_boundaries(self):
        for median in (Fraction(100, 103), Fraction(103, 100)):
            self.assertTrue(c.summarize_ratios([median] * 7 + [Fraction(100, 105), Fraction(105, 100)], "aa")["passed"])
        for median in (Fraction(100, 103) - Fraction(1, 10**12), Fraction(103, 100) + Fraction(1, 10**12)):
            self.assertFalse(c.summarize_ratios([median] * 9, "aa")["passed"])
        self.assertFalse(c.summarize_ratios([Fraction(1)] * 6 + [Fraction(2)] * 3, "aa")["passed"])

    def test_missing_pair_and_diagnostic(self):
        with self.assertRaises(c.ScreenError):
            c.summarize_ratios([Fraction(1)] * 8, "primary")
        self.assertIsNone(c.summarize_ratios([Fraction(10)] * 9, "diagnostic")["passed"])


class ManifestTests(unittest.TestCase):
    def fixture(self, root):
        root = Path(root)
        protocol = root / "protocol.md"
        protocol.write_bytes((Path(__file__).resolve().parent.parent / "prospective-benchmark.md").read_bytes())
        compiler = root / "compiler"
        compiler.write_text("fake compiler, never executed")
        common, baseline, candidate = root / "common.cpp", root / "baseline.hpp", root / "candidate.hpp"
        for path in (common, baseline, candidate):
            path.write_text(str(path))
        source_root = Path(__file__).resolve().parent.parent
        baseline.write_bytes((source_root / "base/tree/rerooting.hpp").read_bytes())
        candidate.write_bytes((source_root / "candidate/tree/rerooting.hpp").read_bytes())
        csr, unit = root / "csr.hpp", root / "unit.cpp"
        csr.write_bytes((source_root / "base/components/undirected_csr.hpp").read_bytes())
        unit.write_bytes((source_root / "candidate/test/unit/rerooting.cpp").read_bytes())
        binaries = {}
        for name in ("baseline", "candidate"):
            path = root / name
            path.write_text("fake binary, never executed")
            binaries[name] = {"path": str(path), "sha256": c.digest(path)}
        data = {"schema_version": 1, "protocol": {"path": str(protocol), "sha256": c.digest(protocol)},
                "compiler": {"path": str(compiler), "sha256": c.digest(compiler), "version": "g++ GCC 15.2.0",
                             "flags": c.FLAGS,
                             "backend": {"path": str(compiler), "sha256": c.digest(compiler)},
                             "tools": {"assembler": {"path": str(compiler), "sha256": c.digest(compiler)}}},
                "sources": {str(common): c.digest(common)}, "baseline_source": {str(baseline): c.digest(baseline), str(csr): c.digest(csr)},
                "candidate_source": {str(candidate): c.digest(candidate), str(csr): c.digest(csr), str(unit): c.digest(unit)},
                "baseline_commit": c.BASELINE_COMMIT, "binaries": binaries,
                "commands": [["fake", "not executed"]], "no_lto": True, "separate_translation_units": True,
                "external_kernel_symbols_and_call_sites_verified": True, "real_timing_executed": False}
        for field in ("objects", "closure_headers", "toolchain", "interpreted_sources", "preparation_evidence"):
            artifact = root / (field + ".fake")
            artifact.write_text("fake never executed " + field)
            data[field] = {str(artifact): c.digest(artifact)}
        data["fixture_roots"] = {b: str(root / (b + "-fixtures")) for b in ("baseline", "candidate")}
        data["fixtures"] = {}
        for build in ("baseline", "candidate"):
            folder = Path(data["fixture_roots"][build])
            folder.mkdir()
            for cell in c.CELLS:
                for seed in c.SEEDS:
                    for kind in ("input", "output"):
                        fixture = folder / c.fixture_name(cell.id, seed, kind)
                        fixture.write_bytes(f"{cell.id} {seed} {kind}".encode())
                        data["fixtures"][str(fixture)] = c.digest(fixture)
        data.update(sealed=True, manifest_kind="sealed-full-fixtures", artifact_root=str(root))
        path = root / "manifest.json"
        path.write_text(json.dumps(data))
        return path, data

    def test_all_artifact_hashes_and_manifest_checked(self):
        for target in ("common.cpp", "baseline.hpp", "candidate.hpp", "baseline", "candidate", "compiler", "protocol.md", "manifest.json", "objects.fake", "closure_headers.fake", "toolchain.fake", "interpreted_sources.fake", "preparation_evidence.fake", "baseline-fixtures/cell-01-seed-857123.input.bin"):
            with self.subTest(target=target), tempfile.TemporaryDirectory() as root:
                path, data = self.fixture(root)
                manifest = c.Manifest(path)
                manifest.verify()
                with (Path(root) / target).open("a") as f:
                    f.write("changed")
                with self.assertRaises(c.ScreenError) as caught:
                    manifest.verify()
                self.assertEqual(caught.exception.code, "source_hash_changed")

    def test_wrong_protocol_hash_and_missing_sources_fail_closed(self):
        with tempfile.TemporaryDirectory() as root:
            path, data = self.fixture(root)
            for change in (("protocol", {"path": str(Path(root) / "protocol.md"), "sha256": "0" * 64}),
                           ("baseline_source", {}), ("no_lto", False)):
                modified = copy.deepcopy(data)
                modified[change[0]] = change[1]
                path.write_text(json.dumps(modified))
                with self.assertRaises(c.ScreenError):
                    c.Manifest(path)

    def test_sealed_inventory_and_all_extra_hashes_are_required(self):
        with tempfile.TemporaryDirectory() as root:
            path, data = self.fixture(root)
            for field in ("fixtures", "objects", "closure_headers", "toolchain", "interpreted_sources", "preparation_evidence"):
                changed = copy.deepcopy(data)
                changed[field] = {}
                path.write_text(json.dumps(changed))
                with self.assertRaises(c.ScreenError):
                    c.Manifest(path)
            for field, value in (("sealed", False), ("manifest_kind", "compiled-provisional")):
                changed = copy.deepcopy(data)
                changed[field] = value
                path.write_text(json.dumps(changed))
                with self.assertRaises(c.ScreenError):
                    c.Manifest(path)
            changed = copy.deepcopy(data)
            changed["fixtures"].pop(next(iter(changed["fixtures"])))
            path.write_text(json.dumps(changed))
            with self.assertRaises(c.ScreenError):
                c.Manifest(path)

    def test_fixture_path_binding_and_digest_verification(self):
        with tempfile.TemporaryDirectory() as root:
            path, data = self.fixture(root)
            m = c.Manifest(path)
            fixture = Path(data["fixture_roots"]["baseline"]) / c.fixture_name(1, c.SEEDS[0], "input")
            self.assertEqual(m.fixture("baseline", 1, c.SEEDS[0], "input", str(fixture)), fixture)
            for wrong in (str(fixture.parent / ".." / fixture.parent.name / fixture.name),
                          str(Path(data["fixture_roots"]["candidate"]) / fixture.name), "relative.bin"):
                with self.assertRaises(c.ScreenError):
                    m.fixture("baseline", 1, c.SEEDS[0], "input", wrong)
            fixture.write_bytes(b"corrupt")
            with self.assertRaises(c.ScreenError) as err:
                m.fixture("baseline", 1, c.SEEDS[0], "input", str(fixture))
            self.assertEqual(err.exception.code, "source_hash_changed")

    def test_real_campaign_requires_explicit_flag_before_any_work(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as caught:
            c.main(["campaign", "--manifest", "/does/not/exist", "--output", "/does/not/exist"])
        self.assertEqual(caught.exception.code, 2)

    def test_rejected_manifest_is_retained_in_new_raw_log(self):
        with tempfile.TemporaryDirectory() as root:
            path, data = self.fixture(root)
            data["protocol"]["sha256"] = "0" * 64
            path.write_text(json.dumps(data))
            output = Path(root) / "raw.jsonl"
            with contextlib.redirect_stderr(io.StringIO()):
                status = c.main(["validate", "--manifest", str(path), "--output", str(output)])
            self.assertEqual(status, 2)
            record = json.loads(output.read_text())
            self.assertEqual(record["event"], "setup_failure")
            self.assertEqual(record["failure"]["code"], "manifest_invalid")


if __name__ == "__main__":
    unittest.main(verbosity=2)
