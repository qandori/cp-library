#!/usr/bin/env python3
"""Future-only, fail-closed standard Ubuntu 26.04 experiment launcher.

THIS FILE IS NOT LAUNCH AUTHORIZATION. --authorize-real-timing plus a separately
reviewed workflow is required. Local preparation must use build.py instead.
The separately reviewed log-only workflow uses one standard ubuntu-26.04 job,
read-only contents permissions, a 14-minute job ceiling, pinned one-minute
checkout, 30-second preparation, one direct invocation, and 60-second transport.
The 660-second experiment and 360-second collector bounds are unchanged.
REMOTE-INTEGRATION.md records the pre-data outer scheduler amendment. This
launcher installs nothing, never retries, and never creates or submits a workflow.
"""
from __future__ import annotations

import argparse
import ctypes
from fractions import Fraction
import hashlib
import multiprocessing
import subprocess
import json
import math
import os
from pathlib import Path
import platform
import re
import shlex
import sys
import time

from build import (HERE, ROOT, EXPECTED, MAX_ARTIFACT_BYTES, Budget, Runner,
                   PreparationError, artifact_bytes, inventory, sha,
                   verify_manifest, verify_map, write_json_new, descendants, kill_tree)

FULL_SECONDS = 660
CAMPAIGN_SECONDS = 360


def workflow_template() -> dict:
    # Exact, immutable four-step log-only integration. No general shell allowlist.
    return json.loads(r'''{
  "name": "Rerooting identity one-shot screen v2",
  "on": {
    "push": {
      "branches": [
        "codex/rerooting-identity-screen-v2-20261019"
      ],
      "paths": [
        "test/benchmark/rerooting_identity_v2/launch.json"
      ]
    }
  },
  "permissions": {
    "contents": "read"
  },
  "concurrency": {
    "group": "rerooting-identity-screen-v2-20261019",
    "cancel-in-progress": false
  },
  "jobs": {
    "screen": {
      "if": "github.event_name == 'push' && github.repository == 'qandori/cp-library' && github.ref == 'refs/heads/codex/rerooting-identity-screen-v2-20261019' && github.event.repository.private == false && github.event.repository.visibility == 'public' && github.event.created == false && github.event.deleted == false && github.event.forced == false && github.run_number == 1 && github.run_attempt == 1",
      "runs-on": "ubuntu-26.04",
      "timeout-minutes": 14,
      "steps": [
        {
          "name": "Check out exact launch head",
          "uses": "actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683",
          "with": {
            "fetch-depth": 0,
            "persist-credentials": false,
            "ref": "${{ github.sha }}"
          },
          "timeout-minutes": 1
        },
        {
          "name": "Bind stage and prepare isolated sources",
          "id": "prepare",
          "timeout-minutes": 1,
          "shell": "bash",
          "run": "python3 -B test/benchmark/rerooting_identity_v2/remote_adapter.py prepare"
        },
        {
          "name": "Run the single fixed experiment",
          "timeout-minutes": 11,
          "shell": "bash",
          "run": "python3 \"$RUNNER_TEMP/rerooting-identity-screen-v2/benchmark/run_experiment.py\" --authorize-real-timing --workflow \"$GITHUB_WORKSPACE/.github/workflows/rerooting-identity-screen-v2.yml\" --reviewed-workflow-sha256 \"${{ steps.prepare.outputs.workflow_sha256 }}\""
        },
        {
          "name": "Preserve complete evidence in logs",
          "if": "always()",
          "timeout-minutes": 1,
          "shell": "bash",
          "run": "python3 -B test/benchmark/rerooting_identity_v2/remote_adapter.py transport"
        }
      ]
    }
  }
}
''')

def verify_workflow(path: Path, expected_sha: str) -> dict:
    if not re.fullmatch('[0-9a-f]{64}', expected_sha or '') or sha(path) != expected_sha:
        raise PreparationError('Workflow bytes do not match separately reviewed SHA-256')
    def unique(items):
        result = {}
        for key, value in items:
            if key in result:
                raise PreparationError('Duplicate workflow key: ' + key)
            result[key] = value
        return result
    try:
        workflow = json.loads(path.read_text(), object_pairs_hook=unique)
    except ValueError as exc:
        raise PreparationError('Future workflow must use the reviewed JSON-compatible YAML format') from exc
    # Equality covers every field, step, command, action pin, condition and cap.
    if json.dumps(workflow, sort_keys=True, allow_nan=False) != json.dumps(workflow_template(), sort_keys=True, allow_nan=False):
        raise PreparationError('Workflow differs from the exact reviewed one-shot log-only template')
    return workflow


def verify_standard_runner(env: dict, os_release: str, event: dict, machine: str) -> dict:
    required = {'GITHUB_ACTIONS': 'true', 'RUNNER_ENVIRONMENT': 'github-hosted',
                'RUNNER_OS': 'Linux', 'RUNNER_ARCH': 'X64', 'ImageOS': 'ubuntu26'}
    for key, value in required.items():
        if env.get(key) != value:
            raise PreparationError('Not the approved standard runner: ' + key)
    fields = {}
    for line in os_release.splitlines():
        if '=' in line:
            k, v = line.split('=', 1)
            fields[k] = v.strip('"')
    if fields.get('ID') != 'ubuntu' or fields.get('VERSION_ID') != '26.04' or machine != 'x86_64':
        raise PreparationError('Ubuntu 26.04 x86-64 OS identity is required')
    repository = event.get('repository', {})
    if repository.get('private') is not False or repository.get('visibility') != 'public':
        raise PreparationError('The future experiment must run in a public repository')
    if not env.get('ImageVersion'):
        raise PreparationError('GitHub image version is missing')
    return {'environment': {key: env[key] for key in [*required, 'ImageVersion']},
            'os_release': fields, 'repository': repository.get('full_name'),
            'public_repository_verified': True, 'machine': machine}


def read_optional(path: Path, max_bytes=65536):
    try:
        with path.open('rb') as handle:
            data = handle.read(max_bytes + 1)
        if len(data) > max_bytes:
            return {'status': 'unknown', 'reason': 'capture exceeds bounded metadata field limit'}
        return {'status': 'known', 'value': data.decode(errors='replace')}
    except OSError as exc:
        return {'status': 'unknown', 'reason': type(exc).__name__}


def capture_metadata() -> dict:
    paths = ['/etc/os-release', '/proc/version', '/proc/cpuinfo', '/proc/meminfo',
             '/proc/loadavg', '/proc/self/status', '/proc/self/cgroup',
             '/proc/pressure/cpu', '/proc/pressure/memory', '/proc/pressure/io']
    values = {path: read_optional(Path(path)) for path in paths}
    cgroup = values['/proc/self/cgroup']
    if cgroup['status'] == 'known':
        for line in cgroup['value'].splitlines():
            if line.startswith('0::'):
                root = Path('/sys/fs/cgroup')
                relative = line[3:].lstrip('/')
                actual = (root / relative).resolve()
                if actual.is_relative_to(root):
                    for name in ('cpu.max', 'cpu.stat', 'cpuset.cpus.effective', 'memory.current',
                                 'memory.peak', 'memory.max', 'cpu.pressure', 'memory.pressure', 'io.pressure'):
                        values[str(actual / name)] = read_optional(actual / name)
    affinity = sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None
    return {'uname': list(platform.uname()), 'logical_cpus': os.cpu_count(),
            'allowed_affinity': affinity, 'counters': values,
            'optional_unknowns_are_not_inferred': True}


def require_outcome(condition, message):
    if not condition:
        raise PreparationError('Invalid collector outcome: ' + message)


def strict_outcome_json(text):
    from collector import strict_json
    try:
        value = strict_json(text)
    except (ValueError, TypeError) as exc:
        raise PreparationError('Malformed collector outcome JSON') from exc
    require_outcome(type(value) is dict, 'result must be a JSON object')
    return value


def exact_json_equal(left, right):
    if type(left) is not type(right):
        return False
    if type(right) is dict:
        return set(left) == set(right) and all(exact_json_equal(left[k], v) for k, v in right.items())
    if type(right) is list:
        return len(left) == len(right) and all(exact_json_equal(a, b) for a, b in zip(left, right))
    return left == right


def validate_collector_outcome(report, returncode, *, manifest_sha256):
    """Validate terminal evidence without changing or weakening collector gates."""
    from collector import (AA_CELLS, PRIMARY_CELLS, CONTROL_CELLS, PROTOCOL_SHA256,
                           MAX_REPS, MAX_BATCH_NS, CAMPAIGN_NS, MAX_RSS, MAX_COLLECTOR_RSS,
                           MAX_COMBINED_RSS, MAX_ARTIFACTS, summarize_ratios)
    require_outcome(type(report) is dict and type(returncode) is int, 'result/exit types')
    status = report.get('status')
    require_outcome(status in ('passed', 'blocked', 'inconclusive'), 'unknown terminal status')
    require_outcome(returncode == (0 if status == 'passed' else 2), 'exit/status contradiction')
    if status == 'inconclusive':
        failure = report.get('failure')
        require_outcome(type(failure) is dict and type(failure.get('code')) is str
                        and bool(failure['code']) and type(failure.get('message')) is str,
                        'inconclusive result needs a structured failure')
        require_outcome(report.get('performance_evidence', False) is False,
                        'inconclusive result cannot be performance evidence')
        return status
    required_fields = {'status', 'mode', 'performance_evidence', 'measurement',
                       'campaign_budget_starts_before_prepare', 'inherited_campaign_deadline_ns',
                       'prior_preparation_inherited_ns', 'resource_accounting', 'protocol_sha256',
                       'metadata', 'counts', 'counts_sha256', 'summaries', 'failure',
                       'postfailure_source_check', 'campaign_elapsed_ns'}
    require_outcome(set(report) == required_fields, 'complete report fields differ from fixed schema')
    require_outcome(type(manifest_sha256) is str and re.fullmatch('[0-9a-f]{64}', manifest_sha256),
                    'missing sealed manifest identity')
    require_outcome(report.get('mode') == 'campaign' and report.get('performance_evidence') is True,
                    'complete performance evidence must be a real campaign result')
    require_outcome(report.get('protocol_sha256') == PROTOCOL_SHA256, 'protocol identity mismatch')
    require_outcome(report['measurement'] == 'warm-call throughput with retained trees; caller-side result destruction excluded',
                    'measurement definition differs from fixed protocol')
    require_outcome(report.get('failure') is None and report.get('postfailure_source_check') is None,
                    'complete result contains failure evidence')
    metadata = report.get('metadata')
    require_outcome(type(metadata) is dict and metadata.get('manifest_sha256') == manifest_sha256
                    and metadata.get('real_timing_authorized_flag') is True,
                    'manifest or authorization identity mismatch')
    require_outcome(report.get('campaign_budget_starts_before_prepare') is True,
                    'campaign budget excludes preparation')
    elapsed, prior = report.get('campaign_elapsed_ns'), report.get('prior_preparation_inherited_ns')
    require_outcome(type(elapsed) is int and elapsed >= 0 and type(prior) is int and prior >= 0
                    and elapsed + prior <= CAMPAIGN_NS, 'invalid or exceeded campaign budget')
    resources = report.get('resource_accounting')
    require_outcome(type(resources) is dict and set(resources) ==
                    {'collector_rss_bytes', 'worker_rss_bytes', 'combined_rss_bytes', 'artifact_bytes'},
                    'invalid resource accounting')
    workers = resources['worker_rss_bytes']
    require_outcome(type(workers) is dict and set(workers) == {'baseline', 'candidate'}
                    and all(type(n) is int and 0 < n <= MAX_RSS for n in workers.values()),
                    'invalid worker RSS accounting')
    for field, limit in [('collector_rss_bytes', MAX_COLLECTOR_RSS),
                         ('combined_rss_bytes', MAX_COMBINED_RSS), ('artifact_bytes', MAX_ARTIFACTS)]:
        require_outcome(type(resources[field]) is int and 0 <= resources[field] <= limit,
                        'invalid resource cap: ' + field)
    require_outcome(resources['combined_rss_bytes'] == resources['collector_rss_bytes'] + sum(workers.values()),
                    'combined RSS accounting contradiction')
    require_outcome(type(report['inherited_campaign_deadline_ns']) is int
                    and report['inherited_campaign_deadline_ns'] > 0, 'missing shared campaign deadline')
    counts = report.get('counts')
    keys = {str(cell) for cell in range(1, 11)}
    require_outcome(type(counts) is dict and set(counts) == keys, 'incomplete repetition table')
    require_outcome(all(type(n) is int and 1 <= n <= MAX_REPS and n & (n - 1) == 0
                        for n in counts.values()), 'invalid frozen repetitions')
    expected_hash = hashlib.sha256(json.dumps(counts, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    require_outcome(report.get('counts_sha256') == expected_hash, 'frozen repetition hash mismatch')
    summaries = report.get('summaries')
    phase_cells = {'aa_begin': AA_CELLS, 'ab': tuple(range(1, 11)), 'aa_end': AA_CELLS}
    require_outcome(type(summaries) is dict and set(summaries) == set(phase_cells), 'missing/extra phases')
    ab_passes = []
    for phase, cells in phase_cells.items():
        values = summaries[phase]
        require_outcome(type(values) is dict and set(values) == {str(cell) for cell in cells},
                        'missing/extra cells in ' + phase)
        for cell in cells:
            summary = values[str(cell)]
            require_outcome(type(summary) is dict, 'cell summary is not an object')
            expected_keys = {'gate', 'median_ratio', 'median_ratio_exact', 'qualifying_pairs', 'passed',
                             'ratios', 'ratios_exact', 'b_elapsed_ns', 'b_per_call_ns',
                             'very_short_batch', 'very_short_note'}
            if phase == 'ab':
                expected_keys |= {'regression', 'candidate_elapsed_ns', 'candidate_per_call_ns'}
            require_outcome(set(summary) == expected_keys, 'cell summary fields differ from fixed schema')
            raw_ratios = summary.get('ratios_exact')
            require_outcome(type(raw_ratios) is list and len(raw_ratios) == 9
                            and all(type(pair) is list and len(pair) == 2
                                    and all(type(x) is int and x > 0 for x in pair) for pair in raw_ratios),
                            'each cell requires nine exact positive ratios')
            ratios = [Fraction(*pair) for pair in raw_ratios]
            gate = 'aa' if phase != 'ab' else 'primary' if cell in PRIMARY_CELLS else 'control' if cell in CONTROL_CELLS else 'diagnostic'
            expected = summarize_ratios(ratios, gate)
            if phase == 'ab':
                regression = summarize_ratios(ratios, 'control')
                require_outcome(exact_json_equal(summary.get('regression'), regression), 'regression summary contradiction')
                expected['passed'] = bool(expected['passed'] and regression['passed'])
            for key, value in expected.items():
                require_outcome(exact_json_equal(summary.get(key), value),
                                'ratio/gate contradiction for ' + phase + '/' + str(cell) + '/' + key)
            durations = summary.get('b_elapsed_ns')
            require_outcome(type(durations) is list and len(durations) == 9
                            and all(type(n) is int and 0 < n <= MAX_BATCH_NS for n in durations),
                            'invalid or incomplete batch durations')
            require_outcome(all((Fraction(duration) / ratio).denominator == 1
                                and 0 < Fraction(duration) / ratio <= MAX_BATCH_NS
                                for duration, ratio in zip(durations, ratios)),
                            'paired baseline durations contradict exact ratios or duration cap')
            require_outcome(summary.get('very_short_batch') is any(n < 1_000_000 for n in durations)
                            and summary.get('very_short_note') == 'Batch <1 ms: limited evidence, informational only; no candidate recalibration.',
                            'batch diagnostics contradict recorded durations')
            require_outcome(exact_json_equal(summary.get('b_per_call_ns'), [n / counts[str(cell)] for n in durations]),
                            'per-call durations contradict frozen counts')
            if phase != 'ab':
                require_outcome(expected['passed'] is True, 'A/A noise failure cannot be complete evidence')
            else:
                require_outcome(exact_json_equal(summary.get('candidate_elapsed_ns'), durations)
                                and exact_json_equal(summary.get('candidate_per_call_ns'), summary['b_per_call_ns']),
                                'candidate duration contradiction')
                ab_passes.append(expected['passed'])
    expected_status = 'passed' if all(ab_passes) else 'blocked'
    require_outcome(status == expected_status, 'terminal status contradicts complete performance gates')
    return status


def decode_collector_outcome(stdout, stderr, returncode, *, manifest_sha256):
    # Normal collector results are one stdout JSON object. Setup errors are one
    # stderr JSON object. Extra text, duplicate keys and non-finite data fail.
    require_outcome(bool(stdout.strip()) != bool(stderr.strip()), 'exactly one result stream is required')
    report = strict_outcome_json(stdout if stdout.strip() else stderr)
    if not stdout.strip():
        require_outcome(report.get('status') == 'inconclusive', 'complete result must be on stdout')
    validate_collector_outcome(report, returncode, manifest_sha256=manifest_sha256)
    return report


def supervised_outcome(path, returncode):
    result = strict_outcome_json(path.read_text())
    status = result.get('status')
    require_outcome(status in ('passed', 'blocked', 'inconclusive'), 'invalid experiment status')
    require_outcome(type(returncode) is int and returncode == (0 if status == 'passed' else 2),
                    'experiment child exit/status contradiction')
    if status != 'inconclusive':
        require_outcome(result.get('post_hash_verification') == 'passed'
                        and result.get('full_deadline_not_exceeded') is True
                        and type(result.get('storage_bytes')) is int
                        and 0 <= result['storage_bytes'] <= MAX_ARTIFACT_BYTES
                        and not any(key in result for key in ('failure', 'hash_failure', 'storage_failure')),
                        'complete experiment failed final validation')
        collector = result.get('collector_result')
        require_outcome(validate_collector_outcome(collector, result.get('collector_returncode'),
                        manifest_sha256=result.get('manifest_sha256')) == status,
                        'experiment and collector status disagree')
    return status


def run_future(args, *, clock=time.monotonic, full_deadline=None):
    # Establish the full deadline before environment/compiler probes or build.
    budget = Budget(FULL_SECONDS, clock=clock, deadline=full_deadline)
    workflow = Path(args.workflow).resolve(strict=True)
    reviewed = verify_workflow(workflow, args.reviewed_workflow_sha256)
    event_path = Path(os.environ.get('GITHUB_EVENT_PATH', ''))
    event = json.loads(event_path.read_text())
    runner_proof = verify_standard_runner(os.environ, Path('/etc/os-release').read_text(), event, platform.machine())
    source_snapshot = inventory([ROOT / name for name in EXPECTED])
    verify_map({str(ROOT / name): value for name, value in EXPECTED.items()})
    interpreted = inventory(HERE.glob('*.py'))
    evidence = HERE / 'experiment'
    evidence.mkdir(exist_ok=False)
    env = dict(os.environ)
    env.update({'REROOTING_VERIFIED_STANDARD_RUNNER': '1', 'REROOTING_PARENT_PROCESS_GROUP': '1',
                'PYTHONDONTWRITEBYTECODE': '1'})
    runner = Runner(evidence, budget, env=env, own_group=True)
    manifest = None
    result = {'status': 'inconclusive', 'real_campaign_started': False, 'workflow_sha256': sha(workflow),
              'runner_proof': runner_proof, 'full_deadline_seconds': FULL_SECONDS,
              'collector_deadline_seconds': CAMPAIGN_SECONDS, 'no_installs': True,
              'no_retries': True, 'before': capture_metadata()}
    try:
        runner.run([sys.executable, HERE / 'build.py', '--deadline-monotonic', str(budget.deadline)], label='build')
        for module, filename in [('provenance_test', 'provenance-tests.log'), ('collector_test', 'collector-tests.log')]:
            out, err = runner.run([sys.executable, '-m', 'unittest', '-v', module], label=module)
            with (HERE / filename).open('x') as handle:
                handle.write(out + err)
        # This shared deadline starts BEFORE all full-size reference generation.
        campaign_deadline = min(budget.deadline - 1.0, clock() + CAMPAIGN_SECONDS)
        env['REROOTING_CAMPAIGN_DEADLINE_MONOTONIC'] = str(campaign_deadline)
        runner.env.update(env)
        out, err = runner.run([sys.executable, HERE / 'validate_worker.py',
                               '--manifest', HERE / 'build-manifest.compiled.json',
                               '--output', HERE / 'worker-validation.json',
                               '--deadline-monotonic', str(campaign_deadline)], label='worker-validation',
                              timeout=max(0.001, campaign_deadline - clock()))
        with (HERE / 'worker-validation.log').open('x') as handle:
            handle.write(out + err)
        runner.run([sys.executable, HERE / 'seal_manifest.py'], label='seal',
                   timeout=max(0.001, campaign_deadline - clock()))
        manifest_path = HERE / 'build-manifest.json'
        manifest_hash = sha(manifest_path)
        manifest = json.loads(manifest_path.read_text())
        verify_manifest(manifest)
        if manifest['compiler'].get('local_preparation') is not False:
            raise PreparationError('Relocated local-preparation compiler is not a runner compiler')
        affinity = sorted(os.sched_getaffinity(0))
        if not affinity:
            raise PreparationError('No usable CPU affinity')
        os.sched_setaffinity(0, {affinity[0]})
        result['pinned_cpu'] = affinity[0]
        result['real_campaign_started'] = True
        out, err = runner.run([sys.executable, HERE / 'collector.py', 'campaign', '--manifest', manifest_path,
                    '--output', evidence / 'raw.jsonl', '--authorize-real-timing'], label='campaign',
                   timeout=max(0.001, campaign_deadline - clock()), check=False)
        collector_returncode = runner.records[-1]['returncode']
        collector_result = decode_collector_outcome(out, err, collector_returncode, manifest_sha256=manifest_hash)
        result.update(collector_result=collector_result, collector_returncode=collector_returncode,
                      manifest_sha256=manifest_hash)
        if sha(manifest_path) != manifest_hash:
            raise PreparationError('Sealed manifest changed during campaign')
        verify_manifest(manifest)
        result['status'] = collector_result['status']
        result['note'] = ('Complete valid screen rejected by the unchanged performance gates.'
                          if result['status'] == 'blocked' else 'Validated collector terminal outcome.')
    except BaseException as exc:
        result['status'] = 'inconclusive'
        result['failure'] = {'type': type(exc).__name__, 'message': str(exc)}
        raise
    finally:
        # Final metadata/hash collection is part of the same full wall budget.
        result['after'] = capture_metadata()
        try:
            verify_map(source_snapshot)
            verify_map(interpreted)
            if manifest is not None:
                verify_manifest(manifest)
            result['post_hash_verification'] = 'passed'
        except (OSError, PreparationError) as exc:
            result['post_hash_verification'] = 'failed'
            result['status'] = 'inconclusive'
            result['hash_failure'] = str(exc)
        result['commands'] = runner.records
        result['storage_bytes'] = artifact_bytes(HERE)
        if result['storage_bytes'] > MAX_ARTIFACT_BYTES:
            result['status'] = 'inconclusive'
            result['storage_failure'] = 'Complete artifacts exceeded 256 MiB'
        result['full_deadline_not_exceeded'] = clock() < budget.deadline
        if not result['full_deadline_not_exceeded']:
            result['status'] = 'inconclusive'
        write_json_new(evidence / 'experiment-result.json', result)
    if result['status'] == 'inconclusive':
        raise PreparationError('Collector or final validation was inconclusive; consult preserved failure evidence')
    return result


def enable_subreaper():
    # Linux PR_SET_CHILD_SUBREAPER: detached workers remain reachable for cleanup.
    if ctypes.CDLL(None, use_errno=True).prctl(36, 1, 0, 0, 0) != 0:
        raise PreparationError('Could not establish bounded descendant cleanup')


class ChildAdapter:
    def __init__(self, child):
        self.child, self.pid = child, child.pid

    def wait(self, timeout):
        self.child.join(timeout)
        if self.child.is_alive():
            raise subprocess.TimeoutExpired('experiment process', timeout)
        return self.child.exitcode


def experiment_child(args, deadline):
    try:
        result = run_future(args, full_deadline=deadline)
    except (PreparationError, OSError, ValueError, KeyError) as exc:
        print('EXPERIMENT INCONCLUSIVE: ' + str(exc), file=sys.stderr)
        raise SystemExit(2) from exc
    raise SystemExit(0 if result['status'] == 'passed' else 2)


def supervise_future(args, *, clock=time.monotonic, context_factory=multiprocessing.get_context):
    # The parent owns the absolute bound even if the child hangs in a filesystem
    # read, hashing, metadata collection, Python cleanup, or a detached worker.
    started = clock()
    deadline = started + FULL_SECONDS
    enable_subreaper()
    child = context_factory('fork').Process(target=experiment_child, args=(args, deadline))
    child.start()
    try:
        child.join(max(0.0, deadline - clock() - 1.0))
        if child.is_alive():
            kill_tree(ChildAdapter(child))
            raise PreparationError('Full 660-second experiment watchdog expired; process tree killed')
        leftovers = descendants(os.getpid())
        if leftovers:
            # The subreaper owns only this isolated launcher's experiment children.
            for pid in leftovers:
                try:
                    os.kill(pid, 9)
                except ProcessLookupError:
                    pass
            raise PreparationError('Experiment left descendant processes; killed and invalidated')
        status = supervised_outcome(HERE / 'experiment' / 'experiment-result.json', child.exitcode)
        if clock() >= deadline:
            raise PreparationError('Full experiment deadline exceeded')
        return status
    except BaseException:
        if child.is_alive():
            kill_tree(ChildAdapter(child))
        raise
    finally:
        child.close()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--authorize-real-timing', action='store_true')
    parser.add_argument('--workflow', type=Path)
    parser.add_argument('--reviewed-workflow-sha256')
    args = parser.parse_args(argv)
    # Intentionally precedes environment reads, compiler probes and any process.
    if not args.authorize_real_timing:
        print('REFUSED: real execution requires explicit --authorize-real-timing and separately reviewed standard-runner workflow.', file=sys.stderr)
        return 2
    if args.workflow is None or not args.reviewed_workflow_sha256:
        print('REFUSED: reviewed workflow path and SHA-256 are required.', file=sys.stderr)
        return 2
    try:
        status = supervise_future(args)
    except (PreparationError, OSError, ValueError, KeyError) as exc:
        print('EXPERIMENT INCONCLUSIVE: ' + str(exc), file=sys.stderr)
        return 2
    if status == 'blocked':
        print('EXPERIMENT BLOCKED: complete valid screen failed the performance gates.', file=sys.stderr)
        return 2
    if status == 'inconclusive':
        print('EXPERIMENT INCONCLUSIVE: consult the preserved failure evidence.', file=sys.stderr)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main())
