import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '2be6469cbc4f7864e7920c98ac1a38450f0770eb'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    lock = open('/tmp/cp-dc-linkcut.lock', 'w')
    fcntl.flock(lock, fcntl.LOCK_EX)
    source = (ROOT / 'test/benchmark/dynamic_connectivity_linkcut.cpp').read_text()
    source = source.replace('std::puts("}");', '''std::printf(",\\\"nodes\\\":%zu,\\\"vertex_nodes\\\":%zu,\\\"node_bytes\\\":%zu", graph.benchmark_nodes(), graph.benchmark_vertices(), graph.benchmark_node_bytes());
	std::puts("}");''')
    before = subprocess.check_output(['git', 'show', BASELINE + ':data_structure/dynamic_connectivity.hpp'], cwd=ROOT, text=True)
    current = (ROOT / 'data_structure/dynamic_connectivity.hpp').read_text()
    result = dict(baseline=BASELINE, compiler=subprocess.check_output(['g++', '--version'], text=True), cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu], seed=713,
        note='Instrumented builds; operation counts and node allocation/RSS diagnostics, not performance timings', cases={})
    with tempfile.TemporaryDirectory(prefix='cp-dc-profile-') as temporary:
        work = Path(temporary)
        cpp = work / 'profile.cpp'
        cpp.write_text(source)
        binaries = {}
        for name, code in [('before', before), ('current', current)]:
            code = code.replace('../components/splitmix64.hpp', 'components/splitmix64.hpp').replace('std::chrono::steady_clock::now().time_since_epoch().count()', 'dc_benchmark_seed')
            code = code.replace('void rotate(int x) {', 'void rotate(int x) {\n\t\t++dc_rotations;')
            code = code.replace('void splay(int x) {', 'void splay(int x) {\n\t\t++dc_splays;')
            code = code.replace('int gg = t[g].parent, d =', 'dc_rotations += 2;\n\t\t\tint gg = t[g].parent, d =')
            code = code.replace('while (!(t[x].own & flag)) {', 'while (!(t[x].own & flag)) {\n\t\t\t++dc_visits;')
            code = code.replace('auto [a, b] = roots[level];', '++dc_levels[level];\n\t\t\tauto [a, b] = roots[level];')
            code = code.replace('link_tree(edge, level + 1, true);', '++dc_promotions;\n\t\t\t\t\t\t\tlink_tree(edge, level + 1, true);')
            code = code.replace('\n};\n\n#endif', '''
	std::size_t benchmark_nodes() const { return t.size(); }
	std::size_t benchmark_node_bytes() const { return sizeof(Node); }
	std::size_t benchmark_vertices() const {
		std::size_t result = 0;
		for (const auto &node : t) result += node.key > 0;
		return result;
	}
};

#endif''')
            header = work / (name + '.hpp')
            header.write_text(code)
            binary = work / name
            flags = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE']
            subprocess.run(['g++', *flags, '-DDC_PROFILE', '-I', str(ROOT), '-DDC_BENCH_HEADER="' + str(header) + '"', str(cpp), '-o', str(binary)], check=True)
            result['flags'] = flags
            binaries[name] = binary
        for case in ['link_and_cut_00', 'dense_00', 'max_random_00']:
            path = Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/dynamic_graph_vertex_add_component_sum/in' / (case + '.in')
            rows = {}
            for name, binary in binaries.items():
                rss = work / 'rss'
                with path.open('rb') as stream:
                    row = json.loads(subprocess.check_output(['/usr/bin/time', '-f', '%M', '-o', str(rss), binary, '713'], stdin=stream))
                row['peak_rss_kib'] = int(rss.read_text())
                rows[name] = row
            assert rows['before']['checksum'] == rows['current']['checksum']
            result['cases'][case] = rows
            print(case, rows, flush=True)
        result['source_sha256'] = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in ['data_structure/dynamic_connectivity.hpp', 'test/benchmark/dynamic_connectivity_linkcut.cpp', 'test/benchmark/dynamic_connectivity_linkcut_profile.py']}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
