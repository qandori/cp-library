import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'fe21ba0'


def variants():
    original = subprocess.check_output(['git', 'show', f'{BASELINE}:graph/hld.hpp'], cwd=ROOT, text=True)
    preorder = original.replace('depth[head[u]] > depth[head[v]]', 'in[u] > in[v]')
    preorder = preorder.replace('return depth[u] < depth[v] ? u : v;', 'return in[u] < in[v] ? u : v;')
    preorder = preorder.replace('bool reversed = depth[u] > depth[v];', 'bool reversed = in[u] > in[v];')
    a = preorder.index('\t\twhile (k > depth[v] - depth[head[v]])')
    b = preorder.index('\n\t}', a)
    preorder = preorder[:a] + '''\t\tint target = depth[v] - k;
		while (depth[head[v]] > target) v = parent[head[v]];
		return order[in[v] - depth[v] + target];''' + preorder[b:]
    a = preorder.index('\tint jump(')
    b = preorder.index('\n\tstd::pair<int, int> subtree', a)
    fused = preorder[:a] + '''	int jump(int u, int v, int k) const {
		check(u), check(v);
		if (k < 0) return -1;
		int from = u, to = v, target = depth[u] - k;
		while (head[u] != head[v]) {
			if (in[u] > in[v]) {
				int h = head[u];
				if (depth[h] <= target) return order[in[u] - depth[u] + target];
				u = parent[h];
			} else v = parent[head[v]];
		}
		int w = in[u] < in[v] ? u : v;
		int length = depth[from] + depth[to] - 2 * depth[w];
		if (k > length) return -1;
		if (target >= depth[w]) return order[in[u] - depth[u] + target];
		return ancestor(to, length - k);
	}''' + preorder[b:]
    a = original.index('\t\tstd::vector<int> start(')
    b = original.index('\n\t\tparent[root] = -1;', a)
    peel_body = '''		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		degree[root] += 2;
		int count = 0;
		for (int v = 0; v < n; ++v) if (degree[v] == 1) order[count++] = v;
		for (int i = 0; i < count; ++i) {
			int v = order[i], p = neighbors[v];
			parent[v] = p;
			size[p] += size[v];
			if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order[count++] = p;
		}
		assert(count == n - 1);
		in[root] = 0, out[root] = n, head[root] = root;
		for (int i = n - 2; i >= 0; --i) {
			int v = order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			if (heavy[p] == v) in[v] = in[p] + 1, head[v] = head[p];
			else in[v] = out[p] -= size[v], head[v] = v;
			out[v] = in[v] + size[v];
		}
		for (int v = 0; v < n; ++v) out[v] = in[v] + size[v], order[in[v]] = v;'''
    result = dict(baseline=original, preorder=preorder, fused=fused)
    for name, source in list(result.items()):
        begin = source.index('\t\tstd::vector<int> start(')
        end = source.index('\n\t\tparent[root] = -1;', begin)
        result['peel' if name == 'baseline' else f'peel_{name}'] = source[:begin] + peel_body + source[end:]
    zero = result['peel_fused'].replace('if (k < 0) return -1;', 'if (k < 0) return -1;\n\t\tif (!k) return u;')
    result['peel_zero'] = zero
    packed = zero.replace('class HLD {', 'class HLD {\n\tstruct Node { int position, head, parent, offset; };\n\tstd::vector<Node> nodes;')
    packed = packed.replace('if (!n) return;', 'nodes.resize(n);\n\t\tif (!n) return;')
    packed = packed.replace('parent[root] = -1;', 'parent[root] = -1;\n\t\tfor (int v = 0; v < n; ++v) nodes[v] = {in[v], in[head[v]], parent[head[v]], depth[v] - in[v]};')
    begin = packed.index('\tint lca(')
    end = packed.index('\n\tint dist(', begin)
    packed = packed[:begin] + '''	int lca(int u, int v) const {
		check(u), check(v);
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) u = nodes[u].parent;
			else v = nodes[v].parent;
		}
		return nodes[u].position < nodes[v].position ? u : v;
	}''' + packed[end:]
    begin = packed.index('\tint ancestor(')
    end = packed.index('\n\tstd::pair<int, int> subtree', begin)
    packed = packed[:begin] + '''	int ancestor(int v, int k) const {
		check(v);
		if (k < 0) return -1;
		int target = nodes[v].position + nodes[v].offset - k;
		if (target < 0) return -1;
		while (nodes[v].head + nodes[v].offset > target) v = nodes[v].parent;
		return order[target - nodes[v].offset];
	}
	int jump(int u, int v, int k) const {
		check(u), check(v);
		if (k < 0) return -1;
		if (!k) return u;
		int from = u, to = v, target = nodes[u].position + nodes[u].offset - k;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				if (nodes[u].head + nodes[u].offset <= target) return order[target - nodes[u].offset];
				u = nodes[u].parent;
			} else v = nodes[v].parent;
		}
		int w = nodes[u].position < nodes[v].position ? u : v;
		int d = nodes[w].position + nodes[w].offset;
		int length = nodes[from].position + nodes[from].offset + nodes[to].position + nodes[to].offset - 2 * d;
		if (k > length) return -1;
		if (target >= d) return order[target - nodes[u].offset];
		return ancestor(to, length - k);
	}''' + packed[end:]
    begin = packed.index('\ttemplate <class F>')
    suffix = packed[begin:]
    for old, new in [('in[head[u]]', 'nodes[u].head'), ('in[head[v]]', 'nodes[v].head'), ('parent[head[u]]', 'nodes[u].parent'), ('parent[head[v]]', 'nodes[v].parent'), ('head[u]', 'nodes[u].head'), ('head[v]', 'nodes[v].head'), ('in[u]', 'nodes[u].position'), ('in[v]', 'nodes[v].position')]:
        suffix = suffix.replace(old, new)
    packed = packed[:begin] + suffix
    result['packed'] = packed
    for name in ['peel_zero', 'packed']:
        shaped = result[name].replace('class HLD {', 'class HLD {\n\tint shape_ = 0, center_ = 0;')
        shaped = shaped.replace('degree[root] += 2;', '''center_ = int(std::max_element(degree.begin(), degree.end()) - degree.begin());
		shape_ = degree[center_] <= 2 ? 1 : degree[center_] == n - 1 ? 2 : 0;
		degree[root] += 2;''')
        begin = shaped.index('\tint lca(')
        insert = shaped.index('\n', shaped.index('check(u), check(v);', begin))
        shaped = shaped[:insert] + '''
		if (shape_ == 1) return head[u] != head[v] ? order[0] : in[u] < in[v] ? u : v;
		if (shape_ == 2) return u == v ? u : u == order[0] || v == order[0] ? order[0] : center_;''' + shaped[insert:]
        begin = shaped.index('\tint ancestor(')
        insert = shaped.index('\n', shaped.index('check(v);', begin))
        shaped = shaped[:insert] + '''
		if (shape_) {
			if (k < 0 || k > depth[v]) return -1;
			if (!k) return v;
			if (k == depth[v]) return order[0];
			return shape_ == 2 ? parent[v] : order[in[v] - k];
		}''' + shaped[insert:]
        begin = shaped.index('\tint jump(')
        insert = shaped.index('\n', shaped.index('if (!k) return u;', begin))
        shaped = shaped[:insert] + '''
		if (shape_ == 1) {
			if (head[u] == head[v]) {
				int delta = in[v] - in[u];
				if (k > (delta < 0 ? -delta : delta)) return -1;
				return order[in[u] + (delta < 0 ? -k : k)];
			}
			int a = depth[u], b = depth[v];
			if (k > a + b) return -1;
			if (k == a) return order[0];
			return k < a ? order[in[u] - k] : order[in[v] - (a + b - k)];
		}
		if (shape_ == 2) {
			int d = (u != center_) + (v != center_);
			if (u == v || k > d) return -1;
			return k == d ? v : center_;
		}''' + shaped[insert:]
        result['shapes' if name == 'peel_zero' else 'packed_shapes'] = shaped
    result['current'] = (ROOT / 'graph/hld.hpp').read_text()
    return result


def generate(directory):
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    sources = variants()
    for name, source in sources.items():
        (directory / f'{name}.hpp').write_text(source)
        (directory / f'{name}_test.cpp').write_text((ROOT / 'test/unit/hld.cpp').read_text().replace('"graph/hld.hpp"', f'"{name}.hpp"'))
    combined = '#include <algorithm>\n#include <array>\n#include <cassert>\n#include <cstddef>\n#include <limits>\n#include <utility>\n#include <vector>\n'
    for name, source in sources.items():
        source = re.sub(r'^#.*\n', '', source, flags=re.M)
        combined += re.sub(r'\bHLD\b', 'HLD_' + name, source)
    (directory / 'variants.hpp').write_text(combined)
    return list(sources)


if __name__ == '__main__':
    import sys
    generate(sys.argv[1])
