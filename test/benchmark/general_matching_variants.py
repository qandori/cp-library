import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/general_matching_baseline.hpp').read_text()
unit = (root / 'test/unit/general_matching.cpp').read_text()
short_method = '''	void short_paths() {
		std::vector<int> first(n, -1), second(n, -1);
		auto save = [&](int u, int v) {
			if (first[u] == -1) first[u] = v;
			else if (first[u] != v) second[u] = v;
		};
		for (auto [u, v] : input) {
			if (mate[u] == -1) save(v, u);
			if (mate[v] == -1) save(u, v);
		}
		for (int u = 0; u < n; ++u) if (u < mate[u]) {
			int v = mate[u];
			for (int a : {first[u], second[u]}) if (a != -1 && mate[a] == -1) {
				int b = first[v];
				if (b == -1 || b == a || mate[b] != -1) b = second[v];
				if (b == -1 || b == a || mate[b] != -1) continue;
				mate[u] = a;
				mate[a] = u;
				mate[v] = b;
				mate[b] = v;
				++cardinality;
				break;
			}
		}
	}
'''
short = source.replace('public:\n', short_method + 'public:\n')
short = short.replace('int build() {\n', 'int build() {\n\t\tif (cardinality < n / 2) short_paths();\n')
short2 = short.replace('if (cardinality < n / 2) short_paths();', 'for (int pass = 0; pass < 2 && cardinality < n / 2; ++pass) short_paths();')
plain = source.replace('if (u == v || cardinality == n / 2) return;', 'if (u == v) return;')
a = plain.index('\t\tif (mate[u] == -1 && mate[v] == -1) {', plain.index('\tvoid add_edge'))
b = plain.index('\n\t}', a)
plain = plain[:a] + plain[b:]
reverse = source.replace('for (int u = 0; u < n; ++u) if (mate[u] == -1 && augment(u))', 'for (int u = n - 1; u >= 0; --u) if (mate[u] == -1 && augment(u))')
free_first = source.replace('int u = queue[head];\n', '''int u = queue[head];
			for (int e = start[u]; e < start[u + 1]; ++e) if (mate[adj[e]] == -1 && adj[e] != root) {
				parent[adj[e]] = u;
				flip(adj[e]);
				return true;
			}
''')

def bitset(code, threshold):
    code = code.replace('#include <algorithm>', '#include <algorithm>\n#include <bit>')
    code = code.replace('int n, cardinality', 'int words = 0;\n\tstd::vector<std::uint64_t> bits, free_bits;\n\tint n, cardinality')
    code = code.replace('void flip(int v) {', 'void flip(int v) {\n\t\tif (words) free_bits[v >> 6] &= ~(std::uint64_t(1) << (v & 63));')
    start = code.index('\tbool augment(int root) {')
    end = code.index('\npublic:', start)
    if '\tvoid short_paths()' in code[start:end]:
        end = code.index('\tvoid short_paths()', start)
    original = code[start:end].replace('bool augment(', 'bool augment_sparse(')
    code = code[:start] + original + '''
	bool augment(int root) {
		if (!words) return augment_sparse(root);
		std::iota(base.begin(), base.end(), 0);
		std::fill(state.begin(), state.end(), -1);
		tail = 1;
		queue[0] = root;
		state[root] = 0;
		free_bits[root >> 6] &= ~(std::uint64_t(1) << (root & 63));
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			const auto *row = bits.data() + std::size_t(u) * words;
			for (int w = 0; w < words; ++w) if (auto available = row[w] & free_bits[w]) {
				int v = 64 * w + std::countr_zero(available);
				parent[v] = u;
				flip(v);
				return true;
			}
			for (int w = 0; w < words; ++w) {
				auto neighbors = row[w];
				while (neighbors) {
					int v = 64 * w + std::countr_zero(neighbors);
					neighbors &= neighbors - 1;
					if (state[v] == -1) {
						parent[v] = u;
						state[v] = 1;
						if (mate[v] == -1) { flip(v); return true; }
						state[mate[v]] = 0;
						queue[tail++] = mate[v];
					} else if (state[v] == 0 && find(u) != find(v)) {
						int b = lca(u, v);
						contract(u, v, b);
						contract(v, u, b);
					}
				}
			}
		}
		free_bits[root >> 6] |= std::uint64_t(1) << (root & 63);
		return false;
	}
''' + code[end:]
    a = code.index('\t\tstart.assign(n + 1, 0);')
    b = code.index('\t\tbase.resize(n);', a)
    csr = code[a:b]
    code = code[:a] + f'\t\twords = n <= {threshold} ? (n + 63) / 64 : 0;\n' + '''		if (words) {
			bits.assign(std::size_t(n) * words, 0);
			free_bits.assign(words, 0);
			for (int u = 0; u < n; ++u) if (mate[u] == -1) free_bits[u >> 6] |= std::uint64_t(1) << (u & 63);
			for (auto [u, v] : input) {
				bits[std::size_t(u) * words + (v >> 6)] |= std::uint64_t(1) << (v & 63);
				bits[std::size_t(v) * words + (u >> 6)] |= std::uint64_t(1) << (u & 63);
			}
			parent.resize(n);
		} else {
''' + csr + '\t\t}\n' + code[b:]
    return code

component_method = '''	int component_bound(std::vector<int> &roots) {
		std::fill(parent.begin(), parent.end(), -1);
		int bound = 0;
		for (int root = 0; root < n; ++root) if (parent[root] == -1) {
			int end = 1, count[2] = {1, 0};
			bool bipartite = true;
			queue[0] = root;
			parent[root] = 0;
			for (int head = 0; head < end; ++head) {
				int u = queue[head];
				for (int e = start[u]; e < start[u + 1]; ++e) {
					int v = adj[e];
					if (parent[v] == -1) {
						parent[v] = parent[u] ^ 1;
						++count[parent[v]];
						queue[end++] = v;
					} else if (parent[v] == parent[u]) bipartite = false;
				}
			}
			bound += bipartite ? std::min(count[0], count[1]) : end / 2;
			int side = count[0] > count[1];
			for (int i = 0; i < end; ++i) {
				int u = queue[i];
				if (mate[u] == -1 && (!bipartite || parent[u] == side)) roots.push_back(u);
			}
		}
		return bound;
	}
'''
def components(code):
    code = code.replace('public:\n', component_method + 'public:\n')
    code = code.replace('\t\tfor (int u = 0; u < n; ++u) if (mate[u] == -1 && augment(u)) {', '''		std::vector<int> roots;
		int limit = component_bound(roots);
		if (cardinality == limit) return cardinality;
		for (int u : roots) if (mate[u] == -1 && augment(u)) {''')
    code = code.replace('if (++cardinality == n / 2) break;', 'if (++cardinality == limit) break;')
    return code
variants = dict(plain=plain, greedy=source, reverse=reverse, short=short, short2=short2, free_first=free_first, bitset=bitset(source, 1024), bitset_short=bitset(short, 1024), bitset_short2=bitset(short2, 1024), components=components(source), components_short=components(short), components_short2=components(short2), current=(root / 'graph/general_matching.hpp').read_text())
for name, code in variants.items():
    code = code.replace('GENERAL_MATCHING_HPP', 'GENERAL_MATCHING_' + name.upper() + '_HPP').replace('GeneralMatching', 'Matching_' + name)
    (out / (name + '.hpp')).write_text(code)
    (out / (name + '_test.cpp')).write_text(unit.replace('"graph/general_matching.hpp"', '"' + name + '.hpp"').replace('GeneralMatching', 'Matching_' + name))
