import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--official-root', type=Path, required=True)
    parser.add_argument('--selection', type=Path, default=ROOT / 'test/benchmark/results/fps_speed_selection.json')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--generate', action='store_true')
    parser.add_argument('--compile-only', action='store_true')
    parser.add_argument('--resume', action='store_true')
    parser.add_argument('--limit-cases', type=int)
    args = parser.parse_args()
    official = args.official_root.resolve()
    folders = {p.parent.name: p.parent for p in official.glob('*/*/info.toml')}
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-DEVAL', '-DONLINE_JUDGE', '-I', str(ROOT)]
    result = dict(mode='compile' if args.compile_only else 'judge', case_limit=args.limit_cases, compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0], flags=flags,
                  official_commit=subprocess.check_output(['git', '-C', str(official), 'rev-parse', 'HEAD'], text=True).strip(),
                  sources={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                           for folder in ['fps', 'components', 'math', 'convolution', 'test']
                           for p in (ROOT / folder).glob('*.hpp')}, tests={}, skipped={})
    if args.resume and args.output.exists():
        previous = json.loads(args.output.read_text())
        if previous['sources'] != result['sources'] or previous['flags'] != result['flags']:
            raise RuntimeError('Cannot reuse checks from different sources or flags')
        if previous.get('mode') != result['mode']:
            if not (args.compile_only and all(row.get('compiled') for row in previous['tests'].values())):
                raise RuntimeError('Cannot reuse checks from a different verification mode')
        if previous.get('case_limit') != args.limit_cases:
            raise RuntimeError('Cannot reuse checks with a different case limit')
        result['tests'].update(previous['tests'])

    def check_sources():
        for name, digest in result['sources'].items():
            if hashlib.sha256((ROOT / name).read_bytes()).hexdigest() != digest:
                raise RuntimeError(f'Source changed during verification: {name}')

    generated = set()
    with tempfile.TemporaryDirectory(prefix='fps-speed-verify-') as temp:
        work = Path(temp)
        for name in json.loads(args.selection.read_text())['verification']:
            source = ROOT / name
            url = re.search(r'#define PROBLEM "([^"]+)"', source.read_text())[1]
            if name in result['tests'] and result['tests'][name]['source_sha256'] == hashlib.sha256(source.read_bytes()).hexdigest():
                print(name, 'reused', flush=True)
                continue
            if args.compile_only and '#include <boost/' in source.read_text() and not Path('/usr/include/boost').exists():
                result['skipped'][name] = 'Boost headers unavailable'
                print(name, 'skipped: Boost headers unavailable', flush=True)
                continue
            if args.compile_only:
                check_sources()
                subprocess.run(['g++', *flags, str(source), '-o', str(work / 'solve')], check=True)
                result['tests'][name] = dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(), compiled=True)
                print(name, 'compiled', flush=True)
                args.output.write_text(json.dumps(result, indent=2) + '\n')
                continue
            if not url.startswith('https://judge.yosupo.jp/problem/'):
                result['skipped'][name] = url
                continue
            problem = url.rsplit('/', 1)[1]
            folder = folders[problem]
            if args.generate and problem not in generated:
                subprocess.run([sys.executable, str(official / 'generate.py'), str(folder / 'info.toml')], check=True)
                generated.add(problem)
            check_sources()
            binary, output = work / 'solve', work / 'actual.txt'
            subprocess.run(['g++', *flags, str(source), '-o', str(binary)], check=True)
            cases = []
            inputs = sorted((folder / 'in').glob('*.in'))
            if args.limit_cases:
                inputs = sorted(inputs, key=lambda p: p.stat().st_size, reverse=True)[:args.limit_cases]
            for inp in inputs:
                answer = folder / 'out' / (inp.stem + '.out')
                with inp.open('rb') as stdin, output.open('wb') as stdout:
                    subprocess.run([str(binary)], stdin=stdin, stdout=stdout, check=True, timeout=120)
                subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)
                cases.append(inp.name)
            if not cases:
                raise RuntimeError(f'No official cases for {problem}')
            result['tests'][name] = dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(), cases=cases)
            print(name, len(cases), 'passed', flush=True)
            args.output.write_text(json.dumps(result, indent=2) + '\n')
    check_sources()
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
