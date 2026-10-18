import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


FLOW_BATCHES = {'max_random': 64, 'max_ladder': 64, 'max_layered': 64, 'max_dense': 16,
                'max_bipartite_lex': 16, 'max_bipartite_shuffled': 16, 'max_gap': 16, 'max_chain': 4}
FLOW_SCHEDULE = ['baseline', 'candidate', 'candidate', 'baseline',
                 'baseline', 'candidate', 'candidate', 'baseline']


def comparison_inputs():
    rng = random.Random(36683669)
    cases = {}
    def put(name, n, edges):
        data = (f'{n} {len(edges)} 1 {n}\n' + ''.join(f'{u+1} {v+1} {c}\n' for u, v, c in edges)).encode()
        cases[name] = {'vertices': n, 'edges': len(edges), 'data': data, 'batch': FLOW_BATCHES[name]}
    n, m = 2000, 10000
    edges = [(rng.randrange(n), rng.randrange(n), rng.randint(1, 10**9)) for _ in range(m)]
    edges = [(u, (v + (u == v)) % n, c) for u, v, c in edges]
    put('max_random', n, edges)
    edges = [(0, v, 1) for v in range(1, n-1)] + [(v, v+1, 10**9) for v in range(1, n-1)]
    put('max_ladder', n, edges)
    width, layers = 20, 98
    edges = [(0, 1+v, 10**9) for v in range(width)]
    for layer in range(layers-1):
        for v in range(width):
            for k in range(4):
                edges.append((1+layer*width+v, 1+(layer+1)*width+rng.randrange(width), rng.randint(1, 10**9)))
    end = width*layers+1
    edges += [(1+(layers-1)*width+v, end, 10**9) for v in range(width)]
    put('max_layered', end+1, edges)
    rng_dense = random.Random(36683669)
    n, m = 1024, 131072
    edges = [(rng_dense.randrange(n), rng_dense.randrange(n), rng_dense.randint(1, 10**9)) for _ in range(m)]
    edges = [(u, (v + (u == v)) % n, c) for u, v, c in edges]
    put('max_dense', n, edges)
    size = 256
    edges = [(0, 1+u, 1) for u in range(size)]
    edges += [(1+u, 1+size+v, 1) for u in range(size) for v in range(size)]
    edges += [(1+size+v, 2*size+1, 1) for v in range(size)]
    put('max_bipartite_lex', 2*size+2, edges)
    random.Random(36683669).shuffle(edges)
    put('max_bipartite_shuffled', 2*size+2, edges)
    size = 10000
    edges = [(0, 1+v, 10**6) for v in range(size)]
    edges += [(1+v, 1+(v+offset)%size, 10**6) for v in range(size) for offset in [1,2,4,8,16,32,64,128]]
    edges += [(1+v, size+1, 1) for v in range(100)]
    put('max_gap', size+2, edges)
    n = 300000
    put('max_chain', n, [(v-1, v, 123) for v in range(1, n)])
    return cases


def compare_revisions(args):
    import re
    import tempfile

    def git(*command):
        return subprocess.check_output(['git', '-C', str(ROOT), *command], timeout=30)

    refs = {'baseline': args.base_ref, 'candidate': args.candidate_ref}
    for ref in refs.values():
        if not ref or not re.fullmatch('[0-9a-f]{40}', ref):
            raise ValueError('Comparison requires two exact commit IDs')
        if git('rev-parse', '--verify', ref + '^{commit}').decode().strip() != ref:
            raise ValueError('Cannot resolve exact source commit')
    if git('rev-parse', 'HEAD').decode().strip() != args.candidate_ref:
        raise ValueError('Comparison checkout is not the exact candidate')
    git('merge-base', '--is-ancestor', args.base_ref, args.candidate_ref)
    if git('diff', '--name-only', 'HEAD', '--') or git('ls-files', '--others', '--exclude-standard'):
        raise ValueError('Comparison checkout must be clean')
    if git('show', args.candidate_ref + ':test/benchmark/flow.py') != Path(__file__).read_bytes():
        raise ValueError('Comparison script differs from candidate source')
    compiler = '/usr/bin/g++-15'
    if subprocess.check_output([compiler, '-dumpfullversion'], timeout=30).strip() != b'15.2.0':
        raise ValueError('Exact GCC 15.2.0 is required; do not install or substitute')
    cpu = min(os.sched_getaffinity(0)) if args.cpu is None else args.cpu
    os.sched_setaffinity(0, {cpu})
    flags = ['-O2', '-std=gnu++23', '-ftrivial-auto-var-init=zero', '-fmodules',
             '-DONLINE_JUDGE', '-DATCODER', '-fno-lto', '-march=native', '-Wall', '-Wextra', '-Werror']
    paths = ['components/push_relabel.hpp', 'components/flow_graph.hpp', 'graph/max_flow.hpp', 'components/fast_io.hpp']
    driver_path = 'test/benchmark/flow_compare.cpp'
    driver = git('show', args.candidate_ref + ':' + driver_path)
    source_hashes, binaries = {}, {}
    cases = comparison_inputs()
    with tempfile.TemporaryDirectory(prefix='cp-flow-compare-') as directory:
        directory = Path(directory)
        environment = {'PATH': '/usr/bin:/bin', 'HOME': str(directory), 'TMPDIR': str(directory),
                       'LC_ALL': 'C', 'TZ': 'UTC'}
        macros = subprocess.check_output([compiler, *flags, '-dM', '-E', '-x', 'c++', '-'],
                                         input=b'#include <cassert>\n', timeout=30, env=environment)
        if b'#define __AVX2__ 1\n' not in macros or re.search(rb'^#define NDEBUG\b', macros, re.MULTILINE):
            raise ValueError('Comparison requires native AVX2 and active assertions')
        for name, ref in refs.items():
            snapshot = directory / name
            source_hashes[name] = {}
            for path in paths + [driver_path]:
                data = driver if path == driver_path else git('show', ref + ':' + path)
                target = snapshot / path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
                source_hashes[name][path] = hashlib.sha256(data).hexdigest()
            binary = directory / (name + '.bin')
            subprocess.run([compiler, *flags, '-I', str(snapshot), str(snapshot / driver_path), '-o', str(binary)],
                           check=True, timeout=120, env=environment)
            binaries[name] = binary
        print(json.dumps({'event': 'flow_comparison', 'refs': refs,
              'trees': {name: git('rev-parse', ref + '^{tree}').decode().strip() for name, ref in refs.items()},
              'compiler': compiler, 'version': '15.2.0', 'flags': flags, 'cpu': cpu,
              'cpu_model': next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'uname': list(os.uname()), 'seed': 36683669, 'schedule': FLOW_SCHEDULE, 'case_count': len(cases),
              'source_sha256': source_hashes, 'common_driver_origin': args.candidate_ref,
              'binary_sha256': {name: hashlib.sha256(binary.read_bytes()).hexdigest() for name, binary in binaries.items()},
              'solver_scope': 'flow calls including CSR construction and solver scratch allocation/destruction',
              'process_scope': 'startup, input, repeated construction, allocation, solver, all-edge output consumption, destruction, output and exit'}, sort_keys=True), flush=True)
        for case, item in cases.items():
            data, batch = item['data'], item['batch']
            expected_residual = None
            for name in refs:
                process = subprocess.run([str(binaries[name]), '1', 'residual'], input=data, capture_output=True,
                                         check=True, timeout=120, env=environment)
                if expected_residual is None:
                    expected_residual = process.stdout
                if process.stdout != expected_residual:
                    raise RuntimeError('Complete residual mismatch: ' + case + ' ' + name)
            expected, samples = None, []
            for index, name in enumerate(FLOW_SCHEDULE):
                begin = time.perf_counter_ns()
                process = subprocess.run([str(binaries[name]), str(batch)], input=data, capture_output=True,
                                         check=True, timeout=120, env=environment)
                wall_ns = time.perf_counter_ns() - begin
                if expected is None:
                    expected = process.stdout
                if process.stdout != expected:
                    raise RuntimeError('Complete batch output mismatch: ' + case + ' ' + name)
                timing = re.fullmatch(rb'solver_ns=([0-9]+)\n', process.stderr)
                if timing is None or int(timing[1]) <= 0:
                    raise ValueError('Invalid solver timing')
                samples.append({'variant': name, 'pair': index // 2, 'process_seconds': wall_ns / 1e9,
                                'solver_seconds': int(timing[1]) / 1e9})
            summary = {}
            for name in refs:
                summary[name] = {key: {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
                                 for key in ('solver_seconds', 'process_seconds')
                                 for values in ([sample[key] for sample in samples if sample['variant'] == name],)}
            print(json.dumps({'event': 'flow_case', 'case': case, 'vertices': item['vertices'], 'edges': item['edges'],
                  'batch': batch, 'input_sha256': hashlib.sha256(data).hexdigest(), 'samples': samples, 'summary': summary,
                  'complete_residual_sha256': hashlib.sha256(expected_residual).hexdigest(),
                  'complete_residual_bytes': len(expected_residual), 'output_sha256': hashlib.sha256(expected).hexdigest(),
                  'output_bytes': len(expected), 'full_outputs_equal': True}, sort_keys=True), flush=True)
        print(json.dumps({'event': 'flow_comparison_completed', 'refs': refs, 'cases': len(cases),
              'pairs': 4 * len(cases), 'full_outputs_equal': True, 'artifacts_uploaded': False,
              'cache_written': False}, sort_keys=True), flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--max-reference', type=Path)
    parser.add_argument('--min-reference', type=Path)
    parser.add_argument('--anti-ssp', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--boost-include', type=Path)
    parser.add_argument('--base-ref')
    parser.add_argument('--candidate-ref')
    parser.add_argument('--cpu', type=int)
    args = parser.parse_args()
    if args.base_ref or args.candidate_ref:
        compare_revisions(args)
        return
    if any(value is None for value in (args.max_reference, args.min_reference, args.anti_ssp, args.output)):
        parser.error('--max-reference, --min-reference, --anti-ssp and --output are required outside comparison')
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    cache = ROOT / '.verify-helper/cache/flow-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native: flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    if args.boost_include: flags += ['-I', str(args.boost_include.resolve())]
    sources = {
        'maximum': ROOT / 'test/benchmark/flow.cpp',
        'minimum': ROOT / 'test/min_cost_b_flow.test.cpp',
        'max_reference_1174960': args.max_reference.resolve(),
        'min_reference_385580': args.min_reference.resolve(),
        'dijkstra_bigint': ROOT / 'test/dijkstra_bigint_yuki.test.cpp',
    }
    bins = {}
    for name, source in sources.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        bins[name] = str(binary)
    rng = random.Random(36683669)
    cases = {}
    n, m = 2000, 10000
    edges = [(rng.randrange(n), rng.randrange(n), rng.randint(1, 10**9)) for _ in range(m)]
    edges = [(u, (v + (u == v)) % n, c) for u, v, c in edges]
    cases['max_random'] = (f'{n} {m} 1 {n}\n' + ''.join(f'{u+1} {v+1} {c}\n' for u,v,c in edges)).encode()
    edges = [(0, v, 1) for v in range(1, n-1)] + [(v, v+1, 10**9) for v in range(1, n-1)]
    cases['max_ladder'] = (f'{n} {len(edges)} 1 {n}\n' + ''.join(f'{u+1} {v+1} {c}\n' for u,v,c in edges)).encode()
    width, layers = 20, 98
    edges = [(0, 1+v, 10**9) for v in range(width)]
    for layer in range(layers-1):
        for v in range(width):
            for k in range(4):
                edges.append((1+layer*width+v, 1+(layer+1)*width+rng.randrange(width), rng.randint(1, 10**9)))
    end = width*layers+1
    edges += [(1+(layers-1)*width+v, end, 10**9) for v in range(width)]
    cases['max_layered'] = (f'{end+1} {len(edges)} 1 {end+1}\n' + ''.join(f'{u+1} {v+1} {c}\n' for u,v,c in edges)).encode()
    n, m = 100, 1000
    cases['min_random'] = (f'{n} {m}\n' + '0\n'*n + ''.join(f'{rng.randrange(n)} {rng.randrange(n)} 0 {rng.randint(1,10**9)} {rng.randint(-10**9,10**9)}\n' for _ in range(m))).encode()
    cases['min_anti_ssp'] = args.anti_ssp.read_bytes()
    n, m = 30000, 50000
    edges = [(v-1, v) for v in range(1, n)]
    used = set(edges)
    while len(edges) < m:
        u, v = sorted(rng.sample(range(n), 2))
        if (u,v) not in used: edges.append((u,v)); used.add((u,v))
    weighted = []
    for u, v in edges:
        a, b = rng.randint(1,300), rng.randint(1,300)
        g = math.gcd(a,b)
        weighted.append(f'{u+1} {v+1} {a//g} {b//g}\n')
    cases['dijkstra_bigint'] = (f'{n} {m}\n' + ''.join(weighted)).encode()
    result = {'platform': platform.platform(), 'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              'flags': flags, 'cpu_affinity': [2], 'trials': 7, 'seed': 36683669,
              'metric': 'wall time including startup, input, construction, computation, output and exit',
              'sources': {name: sha(path) for name,path in sources.items()},
              'headers': {name: sha(ROOT/name) for name in ['graph/max_flow.hpp','graph/min_cost_flow.hpp','components/flow_graph.hpp',
                  'components/push_relabel.hpp','components/min_cost_circulation.hpp','components/RadixHeap.hpp',
                  'components/d_ary_heap.hpp','components/fast_io.hpp','graph/dijkstra.hpp']}, 'workloads': {}}
    for name, data in cases.items():
        if name.startswith('max_'):
            programs = {'push_relabel': [bins['maximum']], 'dinic': [bins['maximum'], 'dinic'], 'reference_1174960': [bins['max_reference_1174960']]}
        elif name.startswith('min_'):
            programs = {'library': [bins['minimum']], 'reference_385580': [bins['min_reference_385580']]}
        else: programs = {'library': [bins['dijkstra_bigint']]}
        samples = {p: [] for p in programs}
        expected = None
        output_hashes = {}
        for program, command in programs.items():
            output = subprocess.check_output(command, input=data, timeout=20)
            answer = output.split()[0] if name.startswith('min_') else output
            if expected is None: expected = answer
            assert expected == answer, (name, program)
            output_hashes[program] = hashlib.sha256(output).hexdigest()
        schedule = list(programs)*7
        rng.shuffle(schedule)
        for program in schedule:
            start = time.perf_counter()
            output = subprocess.check_output(programs[program], input=data, timeout=20)
            samples[program].append((time.perf_counter()-start)*1000)
            assert hashlib.sha256(output).hexdigest() == output_hashes[program]
        item = {'input_sha256': hashlib.sha256(data).hexdigest(), 'input_bytes': len(data), 'programs': {p: {'milliseconds': x, 'median_ms': statistics.median(x), 'output_sha256': output_hashes[p]} for p,x in samples.items()}}
        result['workloads'][name] = item
        print(name, {p: round(x['median_ms'],3) for p,x in item['programs'].items()}, flush=True)
    (args.output/'results.json').write_text(json.dumps(result, indent=2)+'\n')


if __name__ == '__main__': main()
