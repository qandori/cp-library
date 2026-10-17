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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--max-reference', type=Path, required=True)
    parser.add_argument('--min-reference', type=Path, required=True)
    parser.add_argument('--anti-ssp', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--boost-include', type=Path)
    args = parser.parse_args()
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
