#!/usr/bin/env python3
"""Validate the final Dynamic Wavelet Matrix headers and their consumers."""
import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import time


HEADERS = [
    'components/dynamic_quad_vector.hpp',
    'components/dynamic_quad_wavelet.hpp',
    'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp',
]
UNITS = [
    'test/unit/dynamic_quad_vector.cpp',
    'test/unit/dynamic_quad_wavelet.cpp',
    'test/unit/dynamic_wavelet_core.cpp',
    'test/unit/dynamic_wavelet_count.cpp',
    'test/unit/dynamic_wavelet_matrix.cpp',
    'test/unit/standalone.cpp',
]
PRIMARY = [UNITS[0], UNITS[1], UNITS[4]]
DOCS = [
    'docs/dynamic_quad_vector.md',
    'docs/dynamic_quad_wavelet.md',
    'docs/dynamic_wavelet_matrix.md',
]
PROBLEM_COMMIT = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
JUDGES = [
    ('point_set_range_frequency', 'test/dynamic_wavelet_matrix_freq.test.cpp', 25),
    ('range_kth_smallest', 'test/dynamic_wavelet_matrix_kth.test.cpp', 24),
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def utc():
    return datetime.now(timezone.utc).isoformat()


def manifest(repo, paths):
    pending, found = list(paths), {}
    while pending:
        path = (repo / pending.pop()).resolve()
        if path in found:
            continue
        source = path.read_text()
        found[path] = sha(path)
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', source, re.MULTILINE):
            for candidate in [path.parent / include, repo / include]:
                if candidate.is_file():
                    pending.append(candidate)
                    break
    return {path.relative_to(repo).as_posix(): value for path, value in sorted(found.items())}


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[3])
    parser.add_argument('--work', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--docs', action='store_true')
    parser.add_argument('--judge', action='store_true')
    parser.add_argument('--problem-repo', type=Path,
                        default=Path.home() / '.cache/online-judge-tools/library-checker-problems')
    parser.add_argument('--problem-commit', default=PROBLEM_COMMIT)
    parser.add_argument('--timeout', type=int, default=300)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    repo = args.repo.resolve()
    work = (args.work or repo / '.verify-helper/cache/dynamic-wavelet-nim-round2/validation').resolve()
    output = (args.output or repo / 'test/benchmark/results/dynamic_wavelet_nim_round2/validation.json').resolve()
    work.mkdir(parents=True, exist_ok=True)
    output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1', '-UNDEBUG', '-I', str(repo)]
    record = {
        'started_at': utc(),
        'status': 'running',
        'script_sha256': sha(Path(__file__)),
        'repository': str(repo),
        'work': str(work),
        'flags': flags,
        'headers': HEADERS,
        'units': UNITS,
        'primary_units': PRIMARY,
        'source_sha256': manifest(repo, HEADERS + UNITS + ([row[1] for row in JUDGES] if args.judge else [])),
        'environment': {key: os.environ.get(key) for key in [
            'PATH', 'CXXFLAGS', 'CPATH', 'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH', 'LD_LIBRARY_PATH',
            'ASAN_OPTIONS', 'UBSAN_OPTIONS',
        ]},
        'generated_sources': {},
        'commands': [],
    }

    def save():
        temporary = output.with_suffix('.json.tmp')
        temporary.write_text(json.dumps(record, indent=2) + '\n')
        temporary.replace(output)

    def generated(path, source):
        path.write_text(source)
        record['generated_sources'][str(path)] = {'sha256': sha(path), 'code': source}
        save()

    def command(label, argv, environment=None, input_path=None, expected_path=None):
        print(label, flush=True)
        log_path = work / (f'{len(record["commands"]):02d}-' + re.sub(r'[^a-zA-Z0-9_.-]', '_', label) + '.log')
        row = {'label': label, 'command': list(map(str, argv)), 'cwd': str(repo),
               'started_at': utc(), 'log': str(log_path), 'environment': environment or {}}
        if input_path:
            row['input'] = {'path': str(input_path), 'sha256': sha(input_path)}
        if expected_path:
            row['expected'] = {'path': str(expected_path), 'sha256': sha(expected_path)}
        record['commands'].append(row)
        save()
        started = time.monotonic()
        source = input_path.open('rb') if input_path else None
        try:
            process = subprocess.run(row['command'], cwd=repo, env=os.environ | (environment or {}),
                                     stdin=source, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                     timeout=args.timeout)
            text = process.stdout
            row['exit_code'] = process.returncode
        except subprocess.TimeoutExpired as error:
            text = error.stdout or ''
            if isinstance(text, bytes):
                text = text.decode(errors='replace')
            row['exit_code'], row['timeout'] = None, True
        except OSError as error:
            text = str(error) + '\n'
            row['exit_code'] = None
        finally:
            if source:
                source.close()
        row['elapsed_seconds'] = time.monotonic() - started
        if expected_path:
            actual, expected = text.encode(), expected_path.read_bytes()
            actual_tokens, expected_tokens = actual.split(), expected.split()
            row['output_sha256'] = hashlib.sha256(actual).hexdigest()
            row['output_tokens'] = len(actual_tokens)
            row['expected_tokens'] = len(expected_tokens)
            row['answer_matches'] = actual_tokens == expected_tokens
            if row['exit_code'] == 0 and row['answer_matches']:
                text = f'All {len(actual_tokens)} answer tokens matched; output_sha256={row["output_sha256"]}\n'
            else:
                actual_path = log_path.with_suffix('.actual')
                actual_path.write_bytes(actual)
                row['actual_output'] = str(actual_path)
                text = f'Answer mismatch or execution failure; actual output saved to {actual_path}\n' + text[:4000]
        log_path.write_text(text)
        row['log_sha256'] = sha(log_path)
        if '-o' in row['command']:
            artifact = Path(row['command'][row['command'].index('-o') + 1])
            if row['exit_code'] == 0:
                row['artifact'] = {'path': str(artifact), 'sha256': sha(artifact)}
        with output.with_suffix('.log').open('a') as combined:
            combined.write('$ ' + shlex.join(row['command']) + '\n' + text +
                           f'\nexit_code={row["exit_code"]}\n\n')
        save()
        if row['exit_code'] != 0:
            raise RuntimeError(f'{label} failed; see {log_path}')
        if expected_path and not row['answer_matches']:
            raise RuntimeError(f'{label} produced a wrong answer; see {log_path}')
        return text

    save()
    output.with_suffix('.log').write_text('')
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        print('waiting for /tmp/cp-wm-opt.lock', flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        record['lock_acquired_at'] = utc()
        try:
            os.sched_setaffinity(0, {args.cpu})
            record['cpu_affinity'] = sorted(os.sched_getaffinity(0))
            record['compilers'] = {cxx: command(cxx + ' version', [cxx, '--version'])
                                   for cxx in ['g++', 'clang++']}
            command('git revision', ['git', 'rev-parse', 'HEAD'])
            for cxx in ['g++', 'clang++']:
                for index, header in enumerate(HEADERS):
                    source = work / f'header-{index}.cpp'
                    generated(source, f'#include "{header}"\n#include "{header}"\n'
                              '#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) '
                              '|| defined(all) || defined(sz) || defined(uint)\n'
                              '#error "Library headers must not load template macros"\n#endif\n'
                              'int main() {}\n')
                    command(f'{cxx} header {index}', [cxx, *flags, '-fsyntax-only', str(source)])
                for unit in UNITS:
                    binary = work / (('gcc-' if cxx == 'g++' else 'clang-') + Path(unit).stem)
                    command(f'{cxx} compile {Path(unit).stem}', [cxx, *flags, unit, '-o', str(binary)])
                    command(f'{cxx} run {Path(unit).stem}', [str(binary)])
            for mode, cxx, extra in [
                ('sanitize', 'clang++', ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']),
                ('scalar', 'g++', ['-DDYNAMIC_QUAD_DISABLE_SIMD']),
            ]:
                for unit in PRIMARY:
                    binary = work / (mode + '-' + Path(unit).stem)
                    command(f'{mode} compile {Path(unit).stem}', [cxx, *flags, *extra, unit, '-o', str(binary)])
                    env = {'UBSAN_OPTIONS': 'halt_on_error=1'} if mode == 'sanitize' else None
                    command(f'{mode} run {Path(unit).stem}', [str(binary)], env)
            if args.judge:
                problems = args.problem_repo.resolve()
                commit = command('Library Checker revision', ['git', '-C', str(problems), 'rev-parse', 'HEAD']).strip()
                if commit != args.problem_commit:
                    raise RuntimeError(f'Library Checker commit mismatch: {commit} != {args.problem_commit}')
                official = {'root': str(problems), 'commit': commit,
                            'generator_sha256': sha(problems / 'generate.py'), 'problems': {}}
                record['library_checker'] = official
                for problem, driver, count in JUDGES:
                    directory = problems / 'data_structure' / problem
                    inputs = sorted((directory / 'in').glob('*.in'))
                    if len(inputs) != count:
                        raise RuntimeError(f'{problem}: expected {count} cached inputs, found {len(inputs)}')
                    cases = []
                    for input_path in inputs:
                        expected_path = directory / 'out' / input_path.with_suffix('.out').name
                        if not expected_path.is_file():
                            raise RuntimeError(f'Missing cached official answer: {expected_path}')
                        cases.append({'name': input_path.stem,
                                      'input': {'path': str(input_path), 'sha256': sha(input_path)},
                                      'expected': {'path': str(expected_path), 'sha256': sha(expected_path)}})
                    official['problems'][problem] = {
                        'driver': driver, 'driver_sha256': sha(repo / driver),
                        'info_sha256': sha(directory / 'info.toml'),
                        'generator_sources_sha256': {
                            path.relative_to(problems).as_posix(): sha(path)
                            for path in sorted((directory / 'gen').glob('*.cpp'))
                        },
                        'cases': cases,
                    }
                    save()
                    for cxx in ['g++', 'clang++']:
                        binary = work / (('gcc-' if cxx == 'g++' else 'clang-') + problem)
                        command(f'{cxx} compile judge {problem}', [cxx, *flags, driver, '-o', str(binary)])
                        for case in cases:
                            command(f'{cxx} judge {problem} {case["name"]}', [str(binary)],
                                    input_path=Path(case['input']['path']), expected_path=Path(case['expected']['path']))
            if args.docs:
                record['documentation_source_sha256'] = manifest(repo, DOCS + [
                    '.verify-helper/documentation.py', '.verify-helper/check-docs.py',
                ])
                selection = work / 'selection.json'
                generated(selection, json.dumps(dict(headers=HEADERS, unit=UNITS, sanitize=PRIMARY,
                                                      verification=[], bundles=HEADERS), indent=2) + '\n')
                source = work / 'generate_docs.py'
                generated(source,
                          'from pathlib import Path\nimport sys\n'
                          f'sys.path.insert(0, {str(repo / ".verify-helper")!r})\n'
                          'from onlinejudge_verify.documentation import configure\n'
                          f'configure.find_markdown_paths = lambda **kwargs: [Path(p) for p in {DOCS!r}]\n'
                          'from documentation import generate\n'
                          'generate(' + repr({'state': {'targets': {
                              'headers': dict.fromkeys(HEADERS, ''), 'verification': {},
                          }}}) + ')\n')
                command('generate docs', [sys.executable, str(source)])
                command('check docs and bundles', [sys.executable, '.verify-helper/check-docs.py',
                                                   '--selection', str(selection)])
            current = manifest(repo, record['source_sha256'])
            if current != record['source_sha256']:
                raise RuntimeError('Source files changed during validation')
            record['status'] = 'passed'
        except Exception as error:
            record['status'], record['error'] = 'failed', str(error)
            raise
        finally:
            record['finished_at'] = utc()
            save()
    print('Validation passed', flush=True)


if __name__ == '__main__':
    main()
