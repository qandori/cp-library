#!/usr/bin/env python3
"""Frozen, untimed build and provenance collection. Importing executes nothing.

Local preparation is explicitly selected; the future runner path never accepts
this relocated compiler. Diagnostic/preprocessor commands do not time wrappers.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
PROTOCOL_SHA256 = '434d67065f3a7a3de061732c589a38af8f64a3197252ac144ae4b5956219e1ca'
FLAGS = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-fno-lto', '-march=x86-64', '-mtune=generic']
LOCAL_CXX = Path('/workspace/shared/toolchains/gcc-15.2/bin/g++-15')
EXPECTED = {
    'base/tree/rerooting.hpp': 'f7b2af0d128eb794a1342acc328becb82d1fd49075daa6db273d09dda308b828',
    'candidate/tree/rerooting.hpp': '63ff812240127afd3313ac0001652cf01173c66d212063813f8d47fc3a27f5d4',
    'base/components/undirected_csr.hpp': '2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7',
    'candidate/components/undirected_csr.hpp': '2e53a2ce4fbaf18b7f30808cade253977b6cfe422eb3a199b1c79384a0093ec7',
    'candidate/test/unit/rerooting.cpp': '122d62eb83c84ee2f0771a52ebe6ffb3b4a7f7406051df7b5326128dcbd861fc',
    'prospective-benchmark.md': PROTOCOL_SHA256,
}
MAP_FIELDS = ('sources', 'baseline_source', 'candidate_source', 'objects', 'closure_headers',
              'toolchain', 'interpreted_sources', 'preparation_evidence', 'fixtures')
MAX_ARTIFACT_BYTES = 256 * 1024 * 1024


class PreparationError(RuntimeError):
    pass


def sha(path: Path | str) -> str:
    h = hashlib.sha256()
    with Path(path).open('rb') as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def item(path: Path | str) -> dict:
    path = Path(path).resolve(strict=True)
    return {'path': str(path), 'sha256': sha(path)}


def inventory(paths) -> dict[str, str]:
    return {str(Path(p).resolve(strict=True)): sha(p) for p in sorted(set(map(Path, paths)))}


def verify_map(mapping: dict) -> None:
    for filename, expected in mapping.items():
        if not Path(filename).is_absolute() or not re.fullmatch('[0-9a-f]{64}', expected):
            raise PreparationError('Invalid manifest path/hash: ' + filename)
        if sha(filename) != expected:
            raise PreparationError('Artifact changed: ' + filename)


def verify_manifest(data: dict) -> None:
    for field in MAP_FIELDS:
        verify_map(data[field])
    for entry in [data['protocol'], data['compiler'], data['compiler']['backend'],
                  *data['compiler']['tools'].values(), *data['binaries'].values()]:
        verify_map({entry['path']: entry['sha256']})


def write_json_new(path: Path, data: dict) -> None:
    with path.open('x') as handle:
        json.dump(data, handle, indent=2, sort_keys=True)
        handle.write('\n')


class Budget:
    """A wall watchdog only; never a wrapper/performance measurement clock."""
    def __init__(self, seconds=660, *, deadline=None, clock=time.monotonic):
        self.clock = clock
        now = clock()
        self.started = now
        if deadline is not None and (not math.isfinite(deadline) or deadline <= now):
            raise PreparationError('Inherited deadline is invalid or exhausted')
        self.deadline = min(now + seconds, deadline) if deadline is not None else now + seconds
        if not math.isfinite(self.deadline) or self.deadline <= now:
            raise PreparationError('Deadline already exhausted or invalid')

    def remaining(self, reserve=1.0):
        left = self.deadline - self.clock() - reserve
        if left <= 0:
            raise PreparationError('Full experiment deadline exhausted')
        return left


def descendants(pid: int) -> set[int]:
    """Linux descendants, including detached child process groups."""
    parents = {}
    for entry in Path('/proc').iterdir():
        if not entry.name.isdigit():
            continue
        try:
            fields = (entry / 'stat').read_text().rsplit(')', 1)[1].split()
            parents[int(entry.name)] = int(fields[1])
        except (OSError, ValueError, IndexError):
            continue
    result, frontier = set(), {pid}
    while frontier:
        found = {child for child, parent in parents.items() if parent in frontier} - result
        result |= found
        frontier = found
    return result


def kill_tree(process, *, own_group=True):
    # No grace sleep: SIGKILL immediately bounds cleanup, including detached workers.
    pids = descendants(process.pid) | {process.pid}
    for pid in sorted(pids, reverse=True):
        try:
            if own_group:
                pgid = os.getpgid(pid)
                if pgid != os.getpgrp():
                    os.killpg(pgid, signal.SIGKILL)
            os.kill(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    try:
        process.wait(timeout=0.5)
    except subprocess.TimeoutExpired as exc:
        raise PreparationError('Process cleanup did not complete within 0.5 seconds') from exc


class Runner:
    def __init__(self, root: Path, budget: Budget, *, env=None, own_group=None):
        self.root, self.budget = root, budget
        self.env = dict(os.environ if env is None else env)
        self.commands = []
        self.records = []
        self.counter = 0
        self.own_group = own_group

    def run(self, args, *, label=None, timeout=None, check=True):
        args = list(map(str, args))
        self.budget.remaining()
        self.counter += 1
        label = label or f'command-{self.counter:03d}'
        stdout = self.root / (label + '.stdout')
        stderr = self.root / (label + '.stderr')
        self.commands.append(args)
        print(shlex.join(args), flush=True)
        own_group = self.own_group if self.own_group is not None else self.env.get('REROOTING_PARENT_PROCESS_GROUP') != '1'
        with stdout.open('xb') as out, stderr.open('xb') as err:
            proc = subprocess.Popen(args, stdout=out, stderr=err, cwd=HERE, env=self.env,
                                    start_new_session=own_group)
            hard_end = self.budget.clock() + min(self.budget.remaining(), timeout or float('inf'))
            try:
                while proc.poll() is None:
                    if self.budget.clock() >= hard_end:
                        raise PreparationError('Subprocess deadline: ' + label)
                    if stdout.stat().st_size + stderr.stat().st_size > 8 * 1024 * 1024:
                        raise PreparationError('Subprocess output exceeds 8 MiB: ' + label)
                    if artifact_bytes(HERE) > MAX_ARTIFACT_BYTES:
                        raise PreparationError('Total evidence exceeds 256 MiB')
                    time.sleep(min(0.025, max(0.001, hard_end - self.budget.clock())))
                code = proc.wait()
            except BaseException:
                kill_tree(proc, own_group=own_group)
                raise
        self.records.append({'argv': args, 'returncode': code, 'stdout': str(stdout), 'stderr': str(stderr)})
        if check and code:
            raise PreparationError(f'{label} failed ({code}); see {stderr}')
        return stdout.read_text(errors='replace'), stderr.read_text(errors='replace')


def artifact_bytes(root: Path) -> int:
    return sum(p.stat().st_size for p in root.rglob('*') if p.is_file())


def program_search_dirs(output: str) -> list[Path]:
    lines = [line for line in output.splitlines() if line.startswith('programs:')]
    if len(lines) != 1:
        raise PreparationError('Missing/ambiguous compiler program search directories')
    return [Path(x) for x in lines[0].split(':', 1)[1].strip().lstrip('=').split(os.pathsep) if x]


def resolve_program(reported: str, directories: list[Path], *, path_env=None,
                    local_preparation=False, backend=False) -> Path:
    reported = reported.strip()
    if not reported or '\n' in reported:
        raise PreparationError('Missing/ambiguous program identity')
    source = Path(reported)
    candidates = [source] if source.is_absolute() else [d / source for d in directories]
    if not backend and path_env:
        candidates += [Path(d) / source for d in path_env.split(os.pathsep) if d]
    found = {p.resolve() for p in candidates if p.is_file() and os.access(p, os.X_OK)}
    if len(found) != 1:
        raise PreparationError('Missing/ambiguous executable: ' + reported)
    resolved = found.pop()
    if backend:
        accepted = [Path('/usr/lib/gcc'), Path('/usr/libexec/gcc')]
        if local_preparation:
            accepted += [LOCAL_CXX.parent.parent / 'root/usr/lib/gcc',
                         LOCAL_CXX.parent.parent / 'root/usr/libexec/gcc']
        if not any(resolved.is_relative_to(p) for p in accepted):
            raise PreparationError('Unapproved C++ backend layout: ' + str(resolved))
    return resolved


def trace_executables(trace: str, name: str, directories: list[Path], **kwargs) -> set[Path]:
    found = set()
    for line in trace.splitlines():
        try:
            words = shlex.split(line)
        except ValueError:
            continue
        if words and (Path(words[0]).name == name or
                      (name in ('as', 'ld') and Path(words[0]).name.endswith('-' + name))):
            found.add(resolve_program(words[0], directories, **kwargs))
    return found


def verify_backend_trace(backend: Path, trace: str, directories: list[Path], *, local_preparation=False):
    found = trace_executables(trace, 'cc1plus', directories, backend=True,
                              local_preparation=local_preparation)
    if found != {backend.resolve()}:
        raise PreparationError('Verbose compile trace does not match resolved cc1plus')


def parse_dependencies(path: Path) -> set[Path]:
    text = path.read_text().replace('\\\n', '')
    if ':' not in text:
        raise PreparationError('Malformed dependency file: ' + str(path))
    paths = {Path(x).resolve(strict=True) for x in shlex.split(text.split(':', 1)[1])}
    if not paths:
        raise PreparationError('Empty dependency closure')
    return paths


def discover_compiler(runner: Runner, local_preparation: bool) -> Path:
    if local_preparation:
        candidates = [LOCAL_CXX]
    else:
        candidates = [Path(p) for name in ('g++-15', 'g++') if (p := shutil.which(name))]
    for cxx in candidates:
        before = sha(cxx)
        version, _ = runner.run([cxx, '-dumpfullversion'], label='compiler-version-' + cxx.name)
        if sha(cxx) != before:
            raise PreparationError('Compiler driver changed during version probe')
        if version.strip() == '15.2.0':
            if not local_preparation and not cxx.resolve().is_relative_to('/usr'):
                raise PreparationError('Future experiment requires existing system compiler')
            return cxx.resolve(strict=True)
    raise PreparationError('Installed GCC 15.2.0 is required; no installation or substitute permitted')


def build(local_preparation=False, deadline=None, reuse_worker_object=False):
    scoped = {str((ROOT / p).resolve()): h for p, h in EXPECTED.items()}
    verify_map(scoped)
    out = HERE / 'build'
    out.mkdir(exist_ok=reuse_worker_object)
    if reuse_worker_object and not local_preparation:
        raise PreparationError('Recovery is local untimed preparation only, never a future experiment retry')
    recovery = None
    if reuse_worker_object:
        recovery = json.loads((out / 'first-parse-failure.json').read_text())
        bound = {p: h for p, h in recovery['artifacts_after_failure'].items() if not p.endswith('/build.py')}
        verify_map(bound)
    log_root = out / 'resume' if reuse_worker_object else out
    if reuse_worker_object:
        log_root.mkdir(exist_ok=False)
    budget = Budget(deadline=deadline)
    runner = Runner(log_root, budget)
    sources = inventory([HERE / n for n in ('worker.cpp', 'kernel.cpp', 'kernel.hpp')])
    interpreted = inventory(HERE.glob('*.py'))
    write_json_new(log_root / 'build-sources-before.json', {'scoped': scoped, 'sources': sources, 'interpreted_sources': interpreted})
    cxx = discover_compiler(runner, local_preparation)
    metadata = {}
    for key, arg in [('version', '--version'), ('dumpmachine', '-dumpmachine'),
                     ('dumpfullversion', '-dumpfullversion'), ('search_dirs', '-print-search-dirs'),
                     ('specs', '-dumpspecs')]:
        metadata[key], _ = runner.run([cxx, arg], label='compiler-' + key)
    if metadata['dumpfullversion'].strip() != '15.2.0' or not metadata['dumpmachine'].strip().startswith('x86_64-'):
        raise PreparationError('Frozen compiler target/version mismatch')
    dirs = program_search_dirs(metadata['search_dirs'])
    tools = {}
    for name in ('cc1plus', 'collect2', 'as', 'ld'):
        reported, _ = runner.run([cxx, '-print-prog-name=' + name], label='resolve-' + name)
        tools[name] = resolve_program(reported, dirs, path_env=runner.env.get('PATH'),
                                      backend=name == 'cc1plus', local_preparation=local_preparation)
    toolchain = inventory([cxx, *tools.values()])
    _, driver_trace = runner.run([cxx, '-v'], label='driver-verbose-identity')
    actual_drivers = {Path(line.split('=', 1)[1]).resolve(strict=True)
                      for line in driver_trace.splitlines() if line.startswith('COLLECT_GCC=')}
    if len(actual_drivers) != 1:
        raise PreparationError('Missing/ambiguous actual compiler driver identity')
    toolchain.update(inventory(actual_drivers))
    closure = {}
    objects = []
    units = [('worker', HERE / 'worker.cpp', [])]
    units += [('kernel-' + label, HERE / 'kernel.cpp', ['-I', str(ROOT / directory)])
              for label, directory in [('baseline', 'base'), ('candidate', 'candidate')]]
    traces = []
    for name, source, includes in units:
        deps = log_root / (name + '.before.d')
        runner.run([cxx, *FLAGS, *includes, '-M', '-MF', deps, '-MT', name, source], label=name + '-dependencies')
        before = inventory(parse_dependencies(deps))
        closure.update(before)
        obj = out / (name + '.o')
        actual_deps = out / (name + '.d')
        compile_command = [cxx, *FLAGS, *includes, '-v', '-MD', '-MF', actual_deps,
                           '-c', source, '-o', obj]
        write_json_new(log_root / (name + '-inputs.json'),
                       {'source_and_header_hashes_before_compile_or_verified_reuse': before,
                        'command': list(map(str, compile_command)), 'reused': reuse_worker_object and name == 'worker'})
        if reuse_worker_object and name == 'worker':
            trace = (out / 'worker-compile.stderr').read_text()
            runner.commands.append(list(map(str, compile_command)))
            runner.records.append({'argv': list(map(str, compile_command)), 'returncode': 0,
                                   'reused_once_compiled_object': str(obj),
                                   'retained_original_trace': str(out / 'worker-compile.stderr')})
        else:
            _, trace = runner.run(compile_command, label=name + '-compile')
        verify_backend_trace(tools['cc1plus'], trace, dirs, local_preparation=local_preparation)
        actual_as = trace_executables(trace, 'as', dirs, path_env=runner.env.get('PATH'))
        if actual_as != {tools['as']}:
            raise PreparationError('Assembler trace identity differs from resolved assembler')
        if inventory(parse_dependencies(actual_deps)) != before:
            raise PreparationError('Header closure changed during compilation')
        verify_map(before)
        for line in trace.splitlines():
            if line.startswith('COLLECT_GCC='):
                actual_driver = Path(line.split('=', 1)[1]).resolve(strict=True)
                if actual_driver not in actual_drivers:
                    raise PreparationError('Actual compiler driver changed between probe and compile')
        traces.append(trace)
        objects.append(obj)
    binaries = {}
    for label in ('baseline', 'candidate'):
        binary = out / ('worker-' + label)
        linked, trace = runner.run([cxx, *FLAGS, '-v', out / 'worker.o', out / ('kernel-' + label + '.o'),
                                    '-Wl,-t', '-o', binary], label=label + '-link')
        if trace_executables(trace, 'collect2', dirs) != {tools['collect2']}:
            raise PreparationError('Link trace collect2 identity mismatch')
        # GNU ld's -t reports every object/archive/linker script actually consumed.
        libraries = [Path(line.strip()) for line in linked.splitlines() if line.startswith('/') and Path(line.strip()).is_file()]
        if not libraries:
            raise PreparationError('Link input trace is empty')
        toolchain.update(inventory(libraries))
        nm = shutil.which('nm')
        objdump = shutil.which('objdump')
        if not nm or not objdump:
            raise PreparationError('Installed nm and objdump are required')
        toolchain.update(inventory([nm, objdump]))
        symbols, _ = runner.run([nm, '-C', binary], label=label + '-symbols')
        assembly, _ = runner.run([objdump, '-Cd', binary], label=label + '-disassembly')
        for symbol in ('run_pair(', 'run_matrix(', 'snapshot_pair(', 'snapshot_matrix('):
            if not any(' T ' in line and symbol in line for line in symbols.splitlines()):
                raise PreparationError('Missing external symbol: ' + symbol)
            if not any(re.search(r'\bcallq?\b', line) and symbol in line for line in assembly.splitlines()):
                raise PreparationError('Missing actual external call site: ' + symbol)
        binaries[label] = item(binary)
    # Capture dynamic dependencies of every compiler tool and produced binary.
    ldd = shutil.which('ldd')
    if not ldd:
        raise PreparationError('Installed ldd required for dependency provenance')
    toolchain.update(inventory([ldd, sys.executable]))
    worker_runtime_environment = {key: runner.env.get(key) for key in ('LD_LIBRARY_PATH', 'LD_PRELOAD', 'LD_AUDIT')}
    original_ld_path = runner.env.get('LD_LIBRARY_PATH')
    compiler_runtime = str(LOCAL_CXX.parent.parent / 'root/usr/lib/x86_64-linux-gnu')
    binary_paths = {Path(entry['path']) for entry in binaries.values()}
    dependency_environment = {}
    for i, executable in enumerate(sorted(set([Path(sys.executable).resolve(), *actual_drivers, *tools.values(), *[Path(x['path']) for x in binaries.values()]]))):
        if local_preparation and executable in (set(tools.values()) | actual_drivers):
            runner.env['LD_LIBRARY_PATH'] = compiler_runtime + (':' + original_ld_path if original_ld_path else '')
        elif original_ld_path is None:
            runner.env.pop('LD_LIBRARY_PATH', None)
        else:
            runner.env['LD_LIBRARY_PATH'] = original_ld_path
        dependency_environment[str(executable)] = {'LD_LIBRARY_PATH': runner.env.get('LD_LIBRARY_PATH')}
        output, err = runner.run([ldd, executable], label=f'dynamic-dependencies-{i}')
        if 'not found' in output + err:
            raise PreparationError('Unresolved dynamic dependency')
        paths = re.findall(r'(?:=>\s*)?(/[^\s()]+)', output)
        toolchain.update(inventory([Path(p) for p in paths if Path(p).is_file()]))
    verify_map(scoped)
    verify_map(sources)
    verify_map(interpreted)
    verify_map(closure)
    verify_map(toolchain)
    manifest = {
        'schema_version': 1, 'manifest_kind': 'compiled-provisional', 'sealed': False,
        'artifact_root': str(HERE), 'protocol': item(ROOT / 'prospective-benchmark.md'),
        'baseline_commit': '9f05f01500656c65e394a8aa11c3f5afa4d9495a',
        'compiler': {**item(cxx), **metadata, 'flags': FLAGS, 'backend': item(tools['cc1plus']),
                     'tools': {name: item(path) for name, path in tools.items()},
                     'local_preparation': local_preparation},
        'commands': runner.commands, 'command_records': runner.records,
        'runtime_environment': worker_runtime_environment, 'dependency_probe_environment': dependency_environment,
        'sources': sources, 'interpreted_sources': interpreted,
        'baseline_source': {p: h for p, h in scoped.items() if '/base/' in p},
        'candidate_source': {p: h for p, h in scoped.items() if '/candidate/' in p},
        'objects': inventory(objects), 'closure_headers': closure, 'toolchain': toolchain,
        'binaries': binaries, 'fixtures': {},
        'fixture_roots': {label: str(HERE / 'fixtures' / label) for label in ('baseline', 'candidate')},
        'preparation_evidence': inventory(p for p in out.rglob('*') if p.is_file() and p.suffix != '.o' and not p.name.startswith('worker-baseline') and not p.name.startswith('worker-candidate')),
        'local_recovery': ({'record': item(out / 'first-parse-failure.json'),
                            'historical_build_script': item(out / 'build.py.before-parser-fix.txt'),
                            'note': 'Historical hashes are bound to immutable archived paths, not live source paths.'}
                           if recovery is not None else None),
        'no_lto': True, 'separate_translation_units': True,
        'external_kernel_symbols_and_call_sites_verified': True,
        'real_timing_executed': False,
        'status': 'Provisional compiled artifacts only; complete untimed fixtures and seal required.',
    }
    verify_manifest(manifest)
    write_json_new(HERE / 'build-manifest.compiled.json', manifest)
    print('BUILD PASS: untimed preparation only; provisional manifest written', flush=True)
    return manifest


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--local-preparation', action='store_true')
    parser.add_argument('--deadline-monotonic', type=float)
    parser.add_argument('--reuse-worker-object', action='store_true', help='Local preparation only: retain the once-compiled common worker after the documented assembler-parser correction')
    args = parser.parse_args(argv)
    if not args.local_preparation and os.environ.get('REROOTING_VERIFIED_STANDARD_RUNNER') != '1':
        parser.error('Future builds must be launched by the verified standard-runner gate')
    try:
        build(args.local_preparation, args.deadline_monotonic, args.reuse_worker_object)
    except (PreparationError, OSError, ValueError) as exc:
        print('BUILD BLOCKED: ' + str(exc), file=sys.stderr)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main())
