from array import array
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shlex
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / '.verify-helper/cache/dsu-on-tree/validation'
OUTPUT = ROOT / 'test/benchmark/results/dsu_on_tree_validation.json'
SOURCES = ['tree/dsu_on_tree.hpp', 'components/undirected_csr.hpp', 'test/unit/dsu_on_tree.cpp',
           'test/benchmark/dsu_on_tree_mode.cpp', 'test/dsu_on_tree_subtree_sum.test.cpp', 'docs/dsu_on_tree.md']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    os.chdir(ROOT)
    WORK.mkdir(parents=True, exist_ok=True)
    record = dict(utc=datetime.now(timezone.utc).isoformat(), source_sha256={p: sha(ROOT / p) for p in SOURCES}, commands=[])
    previous_path = WORK.parent / 'gcc-unit.json'
    previous = json.loads(previous_path.read_text()) if previous_path.exists() else None
    reused = (previous is not None and previous['compile_exit'] == 0 and previous['run_exit'] == 0 and
              all(sha(ROOT / p) == value for p, value in previous['source_sha256'].items()) and
              (WORK.parent / 'unit').is_file() and sha(WORK.parent / 'unit') == previous['binary_sha256'])
    record['successful_gcc_unit'] = previous if reused else {'reused': False}
    log = OUTPUT.with_suffix('.log')
    log.write_text('')

    def save():
        OUTPUT.write_text(json.dumps(record, indent=2) + '\n')

    def run(command, input_data=None, expected=None):
        command = list(map(str, command))
        process = subprocess.run(command, input=input_data, capture_output=True, timeout=120)
        row = dict(command=command, exit_code=process.returncode, stdout_sha256=hashlib.sha256(process.stdout).hexdigest())
        if expected is not None:
            row['answer_matches'] = process.stdout.split() == expected.split()
        record['commands'].append(row)
        with log.open('a') as stream:
            stream.write('$ ' + shlex.join(command) + '\n' + process.stderr.decode(errors='replace'))
            if expected is None:
                stream.write(process.stdout.decode(errors='replace'))
            stream.write('exit_code=' + str(process.returncode) + '\n')
        save()
        assert process.returncode == 0, (command, process.stderr.decode(errors='replace'))
        if expected is not None:
            assert row['answer_matches'], (command, process.stdout[:1000], expected[:1000])
        return process.stdout

    common = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT)]
    standalone = WORK / 'header.cpp'
    standalone.write_text('#include "tree/dsu_on_tree.hpp"\n#include "tree/dsu_on_tree.hpp"\n'
                          '#if defined(TEMPLATE_HPP) || defined(rep) || defined(all)\n#error "template macros"\n#endif\nint main() {}\n')
    snippets = re.findall(r'```cpp\n(.*?)```', (ROOT / 'docs/dsu_on_tree.md').read_text(), re.S)
    source = '#include "tree/dsu_on_tree.hpp"\nint main() {\n'
    source += re.sub(r'^#include.*\n', '', snippets[0], flags=re.M)
    source += '\nassert((answer == std::vector<int>{3, 2, 1, 1, 1}));\n'
    source += snippets[1] + '\nassert((mode == std::vector<int>{2, 2, 1, 1, 1}));\n}\n'
    (WORK / 'example.cpp').write_text(source)
    for compiler in ['g++', 'clang++']:
        run([compiler, '--version'])
        run([compiler, *common, '-fsyntax-only', standalone])
        run([compiler, *common, '-DNDEBUG', '-fsyntax-only', standalone])
        if compiler == 'clang++' or not reused:
            binary = WORK / (compiler + '-unit')
            run([compiler, *common, 'test/unit/dsu_on_tree.cpp', '-o', binary])
            run([binary])
        run([compiler, *common, WORK / 'example.cpp', '-o', WORK / (compiler + '-example')])
        run([WORK / (compiler + '-example')])
        for name, source in [('subtree_sum', 'test/dsu_on_tree_subtree_sum.test.cpp'),
                             ('mode', 'test/benchmark/dsu_on_tree_mode.cpp')]:
            run([compiler, *common, '-DNDEBUG', source, '-o', WORK / (compiler + '-' + name)])
    run(['clang++', *common, '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g',
         'test/unit/dsu_on_tree.cpp', '-o', WORK / 'sanitize-unit'])
    run([WORK / 'sanitize-unit'])

    upstream = Path('/home/qand/.cache/online-judge-tools/library-checker-problems')
    problem = upstream / 'tree/vertex_add_subtree_sum'
    record['library_checker'] = dict(commit=subprocess.check_output(['git', '-C', upstream, 'rev-parse', 'HEAD'], text=True).strip(),
                                     generator_sha256=sha(upstream / 'generate.py'), info_sha256=sha(problem / 'info.toml'), cases=[])
    for path in sorted((problem / 'in').glob('*.in')):
        expected = (problem / 'out' / path.with_suffix('.out').name).read_bytes()
        record['library_checker']['cases'].append(dict(name=path.name, input_sha256=sha(path), expected_sha256=hashlib.sha256(expected).hexdigest()))
        for compiler in ['g++', 'clang++']:
            run([WORK / (compiler + '-subtree_sum')], path.read_bytes(), expected)

    samples = [('4 454 4 2\n1 2 2\n1 2 2 3\n', 29), ('6 123 2 2\n1\n1 2\n', 101),
               ('15 1 4 5\n1 2 3\n5 3 1 3\n', 1199)]
    rng = random.Random(20261021)
    mode_cases = [(text.encode(), str(value).encode()) for text, value in samples]
    for case in range(120):
        n = rng.randrange(2, 121)
        m, f, state = rng.randrange(2, n + 1), rng.randrange(1, n + 1), rng.randrange(1, 1 << 31)
        seed = state
        parent = [-1]
        supplied = [rng.randrange(v) for v in range(1, m)]
        parent += supplied
        for v in range(m, n):
            parent.append(state % v)
            state = (state * 1103515245 + 12345) & 0x7fffffff
        given = [rng.randrange(n) for _ in range(m)]
        colors = given.copy()
        for v in range(m, n):
            colors.append(state % f)
            state = (state * 1103515245 + 12345) & 0x7fffffff
        members = [[] for _ in range(n)]
        for v in range(n):
            p = v
            while p != -1:
                members[p].append(v)
                p = parent[p]
        result = 0
        for v in range(n):
            frequency = [0] * n
            for x in members[v]:
                frequency[colors[x]] += 1
            maximum = max(frequency)
            result += (maximum ^ (v + 1)) * (frequency.count(maximum) ^ (v + 1))
        text = f'{n} {seed} {m} {f}\n' + ' '.join(str(p + 1) for p in supplied) + '\n' + ' '.join(str(c + 1) for c in given) + '\n'
        mode_cases.append((text.encode(), str(result % 998244353).encode()))
    record['mode_cases'] = []
    for data, expected in mode_cases:
        record['mode_cases'].append(dict(input=data.decode(), expected=expected.decode()))
        for compiler in ['g++', 'clang++']:
            run([WORK / (compiler + '-mode')], data, expected)
    n, seed = 2500000, 20261021
    parent, sizes, state = array('i', [-1, 0]), array('i', [1]) * n, seed
    for v in range(2, n):
        parent.append(state % v)
        state = (state * 1103515245 + 12345) & 0x7fffffff
    for v in range(n - 1, 0, -1):
        sizes[parent[v]] += sizes[v]
    result = sum((sizes[v] ^ (v + 1)) * (1 ^ (v + 1)) for v in range(n)) % 998244353
    data = f'{n} {seed} 2 1\n1\n1 1\n'.encode()
    record['large_mode'] = dict(input=data.decode(), expected=str(result), oracle='all colors equal; reverse parent order subtree sizes')
    for compiler in ['g++', 'clang++']:
        run([WORK / (compiler + '-mode')], data, str(result).encode())

    selection = dict(headers=['tree/dsu_on_tree.hpp'], bundles=['tree/dsu_on_tree.hpp'])
    (WORK / 'selection.json').write_text(json.dumps(selection))
    generator = WORK / 'docs.py'
    generator.write_text('from pathlib import Path\nimport sys\nsys.path.insert(0, ".verify-helper")\n'
                         'from onlinejudge_verify.documentation import configure\n'
                         'configure.find_markdown_paths = lambda **kwargs: [Path("docs/dsu_on_tree.md")]\n'
                         'from documentation import generate\n'
                         'generate({"state":{"targets":{"headers":{"tree/dsu_on_tree.hpp":""},"verification":{}}}})\n')
    run([sys.executable, generator])
    run([sys.executable, '.verify-helper/check-docs.py', '--selection', WORK / 'selection.json'])
    page = ROOT / '.verify-helper/markdown/tree/dsu_on_tree.hpp.md'
    record['documentation_sha256'] = sha(page)
    assert record['source_sha256'] == {p: sha(ROOT / p) for p in SOURCES}
    record['status'] = 'passed'
    save()
    print('DSU on Tree validation passed', flush=True)


if __name__ == '__main__':
    main()
