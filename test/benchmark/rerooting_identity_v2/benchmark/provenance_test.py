#!/usr/bin/env python3
"""Pure synthetic provenance/deadline tests: no compiler or real campaign runs."""
import argparse
import contextlib
import copy
import io
import json
from pathlib import Path
import signal
import subprocess
import tempfile
import unittest
from unittest.mock import Mock, patch

import build
import run_experiment as gate
import seal_manifest


class VirtualExecutables:
    def __init__(self, paths, aliases=None):
        self.paths = set(paths)
        self.aliases = aliases or {}
        self.stack = contextlib.ExitStack()

    def __enter__(self):
        self.stack.enter_context(patch.object(Path, 'is_file', lambda p: str(p) in self.paths))
        self.stack.enter_context(patch.object(Path, 'resolve', lambda p, **kwargs: Path(self.aliases.get(str(p), str(p)))))
        self.stack.enter_context(patch.object(build.os, 'access', lambda p, mode: str(p) in self.paths))
        return self

    def __exit__(self, *args):
        self.stack.close()


class BackendTests(unittest.TestCase):
    def test_both_installed_layouts(self):
        for layout in ('lib', 'libexec'):
            path = Path(f'/usr/{layout}/gcc/x86_64-linux-gnu/15/cc1plus')
            with self.subTest(layout=layout), VirtualExecutables([str(path)]):
                actual = build.resolve_program(str(path), [], backend=True)
                self.assertEqual(actual, path)
                build.verify_backend_trace(actual, f' {path} -quiet worker.cpp\n', [])

    def test_relative_backend_search_resolution(self):
        root = Path('/usr/libexec/gcc/x86_64-linux-gnu/15')
        with VirtualExecutables([str(root / 'cc1plus')]):
            actual = build.resolve_program('cc1plus', [root], backend=True)
            build.verify_backend_trace(actual, ' cc1plus -quiet worker.cpp\n', [root])

    def test_canonical_alias_same_backend(self):
        path = '/usr/libexec/gcc/x86_64-linux-gnu/15/cc1plus'
        alias = '/usr/lib/gcc/x86_64-linux-gnu/15/cc1plus'
        with VirtualExecutables([path, alias], {alias: path}):
            self.assertEqual(build.resolve_program('cc1plus', [Path(alias).parent, Path(path).parent], backend=True), Path(path))

    def test_relative_ambiguous_backend_refused(self):
        a = '/usr/lib/gcc/x86_64-linux-gnu/15/cc1plus'
        b = '/usr/libexec/gcc/x86_64-linux-gnu/15/cc1plus'
        with VirtualExecutables([a, b]), self.assertRaises(build.PreparationError):
            build.resolve_program('cc1plus', [Path(a).parent, Path(b).parent], backend=True)

    def test_missing_backend_refused(self):
        with VirtualExecutables([]), self.assertRaises(build.PreparationError):
            build.resolve_program('cc1plus', [], backend=True)

    def test_inconsistent_trace_refused(self):
        a = '/usr/lib/gcc/x86_64-linux-gnu/15/cc1plus'
        b = '/usr/libexec/gcc/x86_64-linux-gnu/15/cc1plus'
        with VirtualExecutables([a, b]), self.assertRaises(build.PreparationError):
            build.verify_backend_trace(Path(a), f' {b} -quiet\n', [])

    def test_trace_missing_refused(self):
        with self.assertRaises(build.PreparationError):
            build.verify_backend_trace(Path('/usr/lib/gcc/x/15/cc1plus'), 'nothing', [])

    def test_backend_does_not_fall_back_to_path(self):
        with VirtualExecutables(['/tmp/cc1plus']), self.assertRaises(build.PreparationError):
            build.resolve_program('cc1plus', [], path_env='/tmp', backend=True)

    def test_relocated_only_local_preparation(self):
        path = build.LOCAL_CXX.parent.parent / 'root/usr/libexec/gcc/x86_64-linux-gnu/15/cc1plus'
        with VirtualExecutables([str(path)]):
            with self.assertRaises(build.PreparationError):
                build.resolve_program(str(path), [], backend=True)
            self.assertEqual(build.resolve_program(str(path), [], backend=True, local_preparation=True), path)

    def test_unapproved_layout_refused_even_local(self):
        with VirtualExecutables(['/tmp/cc1plus']), self.assertRaises(build.PreparationError):
            build.resolve_program('/tmp/cc1plus', [], backend=True, local_preparation=True)

    def test_target_prefixed_assembler_trace(self):
        path = '/usr/bin/x86_64-linux-gnu-as'
        with VirtualExecutables([path]):
            found = build.trace_executables(' ' + path + ' --64 -o worker.o worker.s', 'as', [])
            self.assertEqual(found, {Path(path)})

    def test_search_directories(self):
        self.assertEqual(build.program_search_dirs('install: /x\nprograms: =/a:/b\nlibraries: =/c\n'), [Path('/a'), Path('/b')])
        with self.assertRaises(build.PreparationError):
            build.program_search_dirs('libraries: /a')


class GateTests(unittest.TestCase):
    def test_unauthorized_gate_no_probe_or_process(self):
        with patch.object(gate, 'run_future') as run, contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(gate.main([]), 2)
            run.assert_not_called()

    def test_flag_alone_insufficient(self):
        with patch.object(gate, 'run_future') as run, contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(gate.main(['--authorize-real-timing']), 2)
            run.assert_not_called()

    def test_full_deadline_budget_starts_before_work(self):
        now = [100.0]
        budget = build.Budget(660, clock=lambda: now[0])
        self.assertEqual(budget.deadline, 760)
        now[0] = 758.5
        self.assertEqual(budget.remaining(), 0.5)
        now[0] = 759
        with self.assertRaises(build.PreparationError):
            budget.remaining()

    def test_inherited_deadline_never_extended(self):
        budget = build.Budget(660, deadline=125, clock=lambda: 100)
        self.assertEqual(budget.deadline, 125)
        for deadline in (100, 99, float('nan'), float('-inf')):
            with self.subTest(deadline=deadline), self.assertRaises(build.PreparationError):
                build.Budget(deadline=deadline, clock=lambda: 100)

    def test_process_group_cleanup_fake_processes_only(self):
        proc = Mock(pid=123)
        with patch.object(build, 'descendants', return_value={124, 125}), \
             patch.object(build.os, 'getpgrp', return_value=1), \
             patch.object(build.os, 'getpgid', side_effect=lambda pid: pid), \
             patch.object(build.os, 'killpg') as groups, patch.object(build.os, 'kill') as pids:
            build.kill_tree(proc)
            self.assertEqual({call.args for call in groups.call_args_list}, {(123, signal.SIGKILL), (124, signal.SIGKILL), (125, signal.SIGKILL)})
            self.assertEqual(pids.call_count, 3)
            proc.wait.assert_called_once_with(timeout=0.5)

    def test_cleanup_uncertain_failure_is_not_success(self):
        proc = Mock(pid=123)
        proc.wait.side_effect = subprocess.TimeoutExpired('fake', .5)
        with patch.object(build, 'descendants', return_value=set()), \
             patch.object(build.os, 'getpgrp', return_value=1), patch.object(build.os, 'getpgid', return_value=123), \
             patch.object(build.os, 'killpg'), patch.object(build.os, 'kill'), \
             self.assertRaises(build.PreparationError):
            build.kill_tree(proc)

    def test_full_supervisor_only_fake_process_success(self):
        child = Mock(pid=123, exitcode=0)
        child.is_alive.return_value = False
        context = Mock()
        context.Process.return_value = child
        factory = Mock(return_value=context)
        times = iter((100.0, 100.0, 101.0))
        with patch.object(gate, 'enable_subreaper'), patch.object(gate, 'descendants', return_value=set()), \
             patch.object(gate, 'supervised_outcome', return_value='passed'):
            gate.supervise_future(argparse.Namespace(), clock=lambda: next(times), context_factory=factory)
        factory.assert_called_once_with('fork')
        child.start.assert_called_once_with()
        child.join.assert_called_once_with(659.0)
        child.close.assert_called_once_with()

    def test_full_supervisor_timeout_kills_fake_tree(self):
        child = Mock(pid=123, exitcode=None)
        child.is_alive.side_effect = [True, False]
        context = Mock()
        context.Process.return_value = child
        factory = Mock(return_value=context)
        with patch.object(gate, 'enable_subreaper'), patch.object(gate, 'kill_tree') as kill, \
             self.assertRaises(build.PreparationError):
            gate.supervise_future(argparse.Namespace(), clock=lambda: 100.0, context_factory=factory)
        self.assertEqual(kill.call_count, 1)
        self.assertEqual(kill.call_args.args[0].pid, 123)
        child.close.assert_called_once_with()

    def good_environment(self):
        env = {'GITHUB_ACTIONS': 'true', 'RUNNER_ENVIRONMENT': 'github-hosted', 'RUNNER_OS': 'Linux',
               'RUNNER_ARCH': 'X64', 'ImageOS': 'ubuntu26', 'ImageVersion': 'synthetic'}
        event = {'repository': {'private': False, 'visibility': 'public', 'full_name': 'example/repo'}}
        return env, 'ID=ubuntu\nVERSION_ID="26.04"\n', event, 'x86_64'

    def test_standard_environment(self):
        self.assertTrue(gate.verify_standard_runner(*self.good_environment())['public_repository_verified'])

    def test_self_hosted_wrong_image_private_repo_refused(self):
        for key, value in [('RUNNER_ENVIRONMENT', 'self-hosted'), ('ImageOS', 'ubuntu24'), ('RUNNER_ARCH', 'ARM64')]:
            env, os_release, event, machine = self.good_environment()
            env[key] = value
            with self.subTest(key=key), self.assertRaises(build.PreparationError):
                gate.verify_standard_runner(env, os_release, event, machine)
        env, os_release, event, machine = self.good_environment()
        event['repository']['private'] = True
        with self.assertRaises(build.PreparationError):
            gate.verify_standard_runner(env, os_release, event, machine)

    def workflow(self):
        return gate.workflow_template()

    def check_workflow(self, data):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'workflow.json'
            path.write_text(json.dumps(data))
            return gate.verify_workflow(path, build.sha(path))

    def test_reviewed_narrow_workflow(self):
        self.check_workflow(self.workflow())

    def test_larger_runner_matrix_install_mutation_refused(self):
        for key, value in [('runs-on', 'ubuntu-26.04-16core'), ('strategy', {'matrix': {'x': [1]}}), ('timeout-minutes', 13)]:
            workflow = self.workflow()
            workflow['jobs']['screen'][key] = value
            with self.subTest(key=key), self.assertRaises(build.PreparationError):
                self.check_workflow(workflow)
        workflow = self.workflow()
        workflow['jobs']['screen']['steps'].append({'run': 'sudo apt install g++-15'})
        with self.assertRaises(build.PreparationError):
            self.check_workflow(workflow)

    def test_workflow_hash_change_refused(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'workflow.json'
            path.write_text('{}')
            with self.assertRaises(build.PreparationError):
                gate.verify_workflow(path, '0' * 64)

    def test_optional_metadata_unknown(self):
        result = gate.read_optional(Path('/definitely-not-a-real-file'))
        self.assertEqual(result['status'], 'unknown')

    def test_dependency_parser_escaped_space(self):
        with tempfile.TemporaryDirectory() as root:
            header = Path(root) / 'space header.hpp'
            header.write_text('test')
            dep = Path(root) / 'test.d'
            dep.write_text('object: ' + str(header).replace(' ', '\\ ') + '\n')
            self.assertEqual(build.parse_dependencies(dep), {header})

    def test_changed_artifact_refused(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'code.py'
            path.write_text('first')
            frozen = build.inventory([path])
            path.write_text('second')
            with self.assertRaises(build.PreparationError):
                build.verify_map(frozen)


class OutcomePropagationTests(unittest.TestCase):
    """End to end through CLI/child/result file, using fake transports/processes.

    'campaign' below is only a collector state-machine label. FakeWorld uses
    FakeClock/FakeWorker; no worker binary, compiler or OS process is launched.
    """
    @classmethod
    def setUpClass(cls):
        from collector_test import FakeWorld
        cls.reports = {}
        for blocked in (False, True):
            world = FakeWorld('campaign')
            world.clock.value = 100_000_000_000
            world.collector.inherited_deadline_ns = 460_000_000_000
            world.collector.metadata.update(real_timing_authorized_flag=True, manifest_sha256='0' * 64)
            world.wall_hook = lambda event, response: 1000
            if blocked:
                world.elapsed_hook = lambda event, elapsed: elapsed * 11 // 10 if event['build'] == 'candidate' and event['cell'] == 5 else elapsed
            report = world.collector.run()
            expected = 'blocked' if blocked else 'passed'
            assert report['status'] == expected
            cls.reports[expected] = json.loads(json.dumps(report))

    def propagate(self, report, *, returncode=None, stdout_override=None, stderr_override=None,
                  watchdog=False, hash_failure_at=None, supervisor_exit_override=None):
        report = copy.deepcopy(report)
        observed = {'commands': [], 'campaign_check': None, 'manifest_checks': 0}
        with tempfile.TemporaryDirectory() as directory, contextlib.ExitStack() as stack:
            root = Path(directory)
            workflow = root / 'workflow.json'
            workflow.write_text('{}')
            event = root / 'event.json'
            event.write_text('{}')

            class FakeRunner:
                def __init__(self, evidence, budget, *, env, own_group):
                    self.env, self.records = dict(env), []

                def run(self, argv, *, label, timeout=None, check=True):
                    observed['commands'].append(label)
                    if label == 'seal':
                        (root / 'build-manifest.json').write_text(json.dumps({'compiler': {'local_preparation': False}}))
                    code = 0
                    if label == 'campaign':
                        observed['campaign_check'] = check
                        if watchdog:
                            raise build.PreparationError('Synthetic external campaign watchdog timeout')
                        code = returncode if returncode is not None else 0 if report['status'] == 'passed' else 2
                    self.records.append({'argv': list(map(str, argv)), 'returncode': code})
                    if label == 'campaign':
                        if 'metadata' in report:
                            report['metadata']['manifest_sha256'] = build.sha(root / 'build-manifest.json')
                        stdout = json.dumps(report) if stdout_override is None else stdout_override
                        stderr = '' if stderr_override is None else stderr_override
                        return stdout, stderr
                    return '', 'OK\n' if label in ('provenance_test', 'collector_test') else ''

            class FakeChild:
                pid = 123
                def __init__(self, target, args):
                    self.target, self.args, self.exitcode = target, args, None
                def start(self):
                    try:
                        self.target(*self.args)
                    except SystemExit as exc:
                        self.exitcode = exc.code
                    if supervisor_exit_override is not None:
                        self.exitcode = supervisor_exit_override
                def join(self, timeout):
                    pass
                def is_alive(self):
                    return False
                def close(self):
                    pass

            context = Mock()
            context.Process.side_effect = lambda target, args: FakeChild(target, args)
            actual_supervisor = gate.supervise_future
            actual_run_future = gate.run_future
            def verify_fake_manifest(manifest):
                observed['manifest_checks'] += 1
                if observed['manifest_checks'] == hash_failure_at:
                    raise build.PreparationError('Synthetic provenance mismatch')
            for name, value in [('HERE', root), ('ROOT', root), ('EXPECTED', {}), ('Runner', FakeRunner)]:
                stack.enter_context(patch.object(gate, name, value))
            stack.enter_context(patch.object(gate, 'verify_workflow', return_value={}))
            stack.enter_context(patch.object(gate, 'verify_standard_runner', return_value={}))
            stack.enter_context(patch.object(gate, 'capture_metadata', return_value={}))
            stack.enter_context(patch.object(gate, 'verify_manifest', side_effect=verify_fake_manifest))
            stack.enter_context(patch.object(gate, 'enable_subreaper'))
            stack.enter_context(patch.object(gate, 'descendants', return_value=set()))
            stack.enter_context(patch.object(gate.os, 'sched_getaffinity', return_value={2}))
            stack.enter_context(patch.object(gate.os, 'sched_setaffinity'))
            stack.enter_context(patch.dict(gate.os.environ, {'GITHUB_EVENT_PATH': str(event)}))
            stack.enter_context(patch.object(gate, 'run_future', side_effect=lambda args, full_deadline: actual_run_future(args, clock=lambda: 100.0, full_deadline=full_deadline)))
            stack.enter_context(patch.object(gate, 'supervise_future', side_effect=lambda args: actual_supervisor(args, clock=lambda: 100.0, context_factory=lambda method: context)))
            output = io.StringIO()
            with contextlib.redirect_stderr(output):
                exitcode = gate.main(['--authorize-real-timing', '--workflow', str(workflow),
                                      '--reviewed-workflow-sha256', '0' * 64])
            saved = json.loads((root / 'experiment' / 'experiment-result.json').read_text())
            return exitcode, saved, output.getvalue(), observed

    def test_complete_valid_blocked_survives_result_supervisor_and_cli(self):
        code, result, output, observed = self.propagate(self.reports['blocked'])
        self.assertEqual((code, result['status'], result['collector_result']['status']), (2, 'blocked', 'blocked'))
        self.assertIn('EXPERIMENT BLOCKED:', output)
        self.assertNotIn('EXPERIMENT INCONCLUSIVE:', output)
        self.assertFalse(observed['campaign_check'])
        self.assertEqual(observed['commands'].count('campaign'), 1)
        self.assertEqual(result['post_hash_verification'], 'passed')

    def test_complete_passed_survives_result_supervisor_and_cli(self):
        code, result, output, _ = self.propagate(self.reports['passed'])
        self.assertEqual((code, result['status']), (0, 'passed'))
        self.assertNotIn('INCONCLUSIVE', output)

    def test_noise_provenance_and_watchdog_reports_stay_inconclusive(self):
        for failure in ('aa_guard_failed', 'source_hash_changed', 'watchdog', 'campaign_budget'):
            report = copy.deepcopy(self.reports['blocked'])
            report.update(status='inconclusive', performance_evidence=False,
                          failure={'code': failure, 'message': 'synthetic failure'})
            with self.subTest(failure=failure):
                code, result, output, _ = self.propagate(report)
                self.assertEqual((code, result['status']), (2, 'inconclusive'))
                self.assertIn('EXPERIMENT INCONCLUSIVE:', output)
                self.assertNotIn('EXPERIMENT BLOCKED:', output)

    def test_external_timeout_stays_inconclusive(self):
        code, result, output, _ = self.propagate(self.reports['blocked'], watchdog=True)
        self.assertEqual((code, result['status']), (2, 'inconclusive'))
        self.assertIn('watchdog timeout', result['failure']['message'])
        self.assertNotIn('EXPERIMENT BLOCKED:', output)

    def test_final_provenance_failure_overrides_blocked(self):
        code, result, output, _ = self.propagate(self.reports['blocked'], hash_failure_at=3)
        self.assertEqual((code, result['status'], result['post_hash_verification']), (2, 'inconclusive', 'failed'))
        self.assertNotIn('EXPERIMENT BLOCKED:', output)

    def test_collector_exit_status_contradictions_are_inconclusive(self):
        for status, code in [('blocked', 0), ('passed', 2), ('blocked', 1), ('passed', -9)]:
            with self.subTest(status=status, code=code):
                actual, result, output, _ = self.propagate(self.reports[status], returncode=code)
                self.assertEqual((actual, result['status']), (2, 'inconclusive'))
                self.assertNotIn('EXPERIMENT BLOCKED:', output)

    def test_supervisor_child_exit_contradiction_is_inconclusive(self):
        code, result, output, _ = self.propagate(self.reports['blocked'], supervisor_exit_override=0)
        self.assertEqual(code, 2)
        self.assertIn('exit/status contradiction', output)
        self.assertNotIn('EXPERIMENT BLOCKED:', output)

    def test_malformed_duplicate_nonfinite_extra_output_refused(self):
        malformed = ['not JSON', '{}', '[]', '{"status":"blocked","status":"passed"}',
                     '{"status":"blocked","ratio":NaN}', json.dumps(self.reports['blocked']) + '\nextra']
        for text in malformed:
            with self.subTest(text=text[:40]):
                code, result, output, _ = self.propagate(self.reports['blocked'], stdout_override=text)
                self.assertEqual((code, result['status']), (2, 'inconclusive'))
                self.assertNotIn('EXPERIMENT BLOCKED:', output)
        code, result, _, _ = self.propagate(self.reports['blocked'], stderr_override='unexpected warning')
        self.assertEqual((code, result['status']), (2, 'inconclusive'))

    def test_incomplete_or_contradictory_performance_results_refused(self):
        mutations = [lambda r: r['summaries'].pop('aa_end'),
                     lambda r: r['summaries']['ab'].pop('10'),
                     lambda r: r['summaries']['ab']['5']['ratios_exact'].pop(),
                     lambda r: r['counts'].update({'1': True}),
                     lambda r: r.update(counts_sha256='f' * 64),
                     lambda r: r.update(status='passed'),
                     lambda r: r['summaries']['aa_begin']['1'].update(passed=False),
                     lambda r: r['summaries']['ab']['5'].update(passed=True),
                     lambda r: r['summaries']['aa_begin']['1']['ratios'].clear(),
                     lambda r: r['summaries']['aa_begin']['1']['ratios'].__setitem__(0, True),
                     lambda r: r.update(protocol_sha256='0' * 64),
                     lambda r: r.update(campaign_elapsed_ns=360_000_000_001),
                     lambda r: r['resource_accounting'].update(combined_rss_bytes=0),
                     lambda r: r.update(measurement=1),
                     lambda r: r['summaries']['ab']['5'].update(unexpected=True),
                     lambda r: r['summaries']['ab']['5'].update(very_short_batch=1),
                     lambda r: r['summaries']['ab']['5']['b_elapsed_ns'].__setitem__(0, 1)]
        for number, mutate in enumerate(mutations):
            report = copy.deepcopy(self.reports['blocked'])
            mutate(report)
            with self.subTest(mutation=number):
                code, result, output, _ = self.propagate(report)
                self.assertEqual((code, result['status']), (2, 'inconclusive'))
                self.assertNotIn('EXPERIMENT BLOCKED:', output)


if __name__ == '__main__':
    unittest.main(verbosity=2)
