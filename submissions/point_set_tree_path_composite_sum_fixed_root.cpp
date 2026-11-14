#line 1 "test/point_set_tree_path_composite_sum_fixed_root.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root"
#line 1 "tree/static_top_tree.hpp"



#include <cassert>
#include <utility>
#include <vector>
#line 1 "components/static_top_tree_structure.hpp"



#include <algorithm>
#line 6 "components/static_top_tree_structure.hpp"
#include <functional>
#include <limits>
#line 1 "tree/hld.hpp"



#line 5 "tree/hld.hpp"
#include <array>
#line 7 "tree/hld.hpp"
#include <cstddef>
#line 11 "tree/hld.hpp"

class HLD {
	int shape_ = 0, center_ = 0;
	struct Node { int position, head, parent, offset; };
	std::vector<Node> nodes;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && int(in.size()) == n); }

	void initialize() {
		depth.assign(n, 0);
		size.assign(n, 1);
		heavy.assign(n, -1);
		head.resize(n), in.resize(n), out.resize(n), order.resize(n);
		nodes.resize(n);
		shape_ = center_ = 0;
	}
	void finish(int root) {
		in[root] = 0, out[root] = n, head[root] = root;
		for (int i = n - 2; i >= 0; --i) {
			int v = order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			if (heavy[p] == v) in[v] = in[p] + 1, head[v] = head[p];
			else in[v] = out[p] -= size[v], head[v] = v;
			out[v] = in[v] + size[v];
		}
		for (int v = 0; v < n; ++v) out[v] = in[v] + size[v], order[in[v]] = v;
		parent[root] = -1;
		for (int v = 0; v < n; ++v) nodes[v] = {in[v], in[head[v]], parent[head[v]], depth[v] - in[v]};
	}

  public:
	struct Segment {
		int l, r;
		bool reversed;
	};
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth, size, heavy, head, in, out, order;
	explicit HLD(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return int(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		parent.assign(n, -1);
		initialize();
		if (!n) return;
		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		center_ = int(std::max_element(degree.begin(), degree.end()) - degree.begin());
		shape_ = degree[center_] <= 2 ? 1 : degree[center_] == n - 1 ? 2 : 0;
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
		finish(root);
	}
	void build_from_parents(std::vector<int> parents) {
		assert(parents.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		n = static_cast<int>(parents.size());
		parent = std::move(parents);
		initialize();
		edges.clear();
		edges.reserve(std::max(0, n - 1));
		if (!n) return;
		std::fill(out.begin(), out.end(), 0);
		int root = -1;
		bool ordered = true;
		for (int v = 0; v < n; ++v) {
			int p = parent[v];
			assert(-1 <= p && p < n && p != v);
			if (p == -1) {
				assert(root == -1);
				root = v;
			} else {
				++out[p];
				edges.emplace_back(p, v);
				ordered &= p < v;
			}
		}
		assert(root != -1);
		int maximum = 0;
		for (int v = 0; v < n; ++v) {
			int degree = out[v] + (v != root);
			if (degree > maximum) maximum = degree, center_ = v;
		}
		shape_ = maximum <= 2 ? 1 : maximum == n - 1 ? 2 : 0;
		if (ordered) {
			for (int v = n - 1; v > 0; --v) {
				int p = parent[v];
				size[p] += size[v];
				if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
				order[n - 1 - v] = v;
			}
		} else {
			int count = 0;
			for (int v = 0; v < n; ++v) if (!out[v] && v != root) order[count++] = v;
			for (int i = 0; i < count; ++i) {
				int v = order[i], p = parent[v];
				size[p] += size[v];
				if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
				if (!--out[p] && p != root) order[count++] = p;
			}
			assert(count == n - 1);
		}
		finish(root);
	}
	int lca(int u, int v) const {
		check(u), check(v);
		if (shape_ == 1) return head[u] != head[v] ? order[0] : in[u] < in[v] ? u : v;
		if (shape_ == 2) return u == v ? u : u == order[0] || v == order[0] ? order[0] : center_;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) u = nodes[u].parent;
			else v = nodes[v].parent;
		}
		return nodes[u].position < nodes[v].position ? u : v;
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	int distance(int u, int v) const { return dist(u, v); }
	bool is_ancestor(int u, int v) const {
		check(u), check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	int ancestor(int v, int k) const {
		check(v);
		if (shape_) {
			if (k < 0 || k > depth[v]) return -1;
			if (!k) return v;
			if (k == depth[v]) return order[0];
			return shape_ == 2 ? parent[v] : order[in[v] - k];
		}
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
		}
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
	}
	std::pair<int, int> subtree(int v, bool edge = false) const {
		check(v);
		return {in[v] + edge, out[v]};
	}
	int edge_vertex(int id) const {
		assert(0 <= id && id < int(edges.size()));
		auto [u, v] = edges[id];
		check(u);
		return parent[u] == v ? u : v;
	}
	template <class F>
	void for_each_path(int u, int v, F &&f, bool edge = false) const {
		check(u), check(v);
		if (shape_ == 1) {
			int x = in[u], y = in[v];
			if (head[u] == head[v]) {
				int l = std::min(x, y) + edge, r = std::max(x, y) + 1;
				if (l < r) f(l, r, x > y);
			} else if (head[u] == order[0]) {
				if (edge < x + 1) f(int(edge), x + 1, true);
				f(in[head[v]], y + 1, false);
			} else {
				f(in[head[u]], x + 1, true);
				if (edge < y + 1) f(int(edge), y + 1, false);
			}
			return;
		}
		std::array<std::array<int, 2>, 32> right;
		int count = 0;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				f(nodes[u].head, nodes[u].position + 1, true);
				u = nodes[u].parent;
			} else {
				assert(count < int(right.size()));
				right[count++] = {nodes[v].head, nodes[v].position + 1};
				v = nodes[v].parent;
			}
		}
		bool reversed = nodes[u].position > nodes[v].position;
		int l = std::min(nodes[u].position, nodes[v].position) + edge, r = std::max(nodes[u].position, nodes[v].position) + 1;
		if (l < r) f(l, r, reversed);
		while (count) {
			auto [a, b] = right[--count];
			f(a, b, false);
		}
	}
	std::vector<Segment> path(int u, int v, bool edge = false) const {
		std::vector<Segment> result;
		for_each_path(u, v, [&](int l, int r, bool reversed) { result.push_back({l, r, reversed}); }, edge);
		return result;
	}
};


#line 11 "components/static_top_tree_structure.hpp"

namespace static_top_tree_detail {
struct Structure {
	enum Kind : unsigned char { Vertex, Compress, Rake };
	struct Node {
		int parent = -1, left = -1, right = -1, index = -1;
		Kind kind = Vertex;
	};
	std::vector<Node> nodes;
	std::vector<int> order, parent, edge_child;
	int root = -1, root_vertex = -1, paths = 0, points = 0, height = 0;
	Structure() = default;
	explicit Structure(const HLD& tree) { build(tree); }
	void build(const HLD& tree) {
		int n = tree.n;
		assert(n <= std::numeric_limits<int>::max() / 2);
		assert(int(tree.order.size()) == n);
		nodes.assign(n, {}), order.clear();
		nodes.reserve(std::max(0, 2 * n - 1)), order.reserve(std::max(0, 2 * n - 1));
		parent = tree.parent;
		edge_child.clear(), edge_child.reserve(tree.edges.size());
		for (auto [u, v] : tree.edges) edge_child.push_back(parent[u] == v ? u : v);
		paths = n, points = 0, root = -1, height = 0;
		root_vertex = n ? tree.order[0] : -1;
		for (int v = 0; v < n; ++v) nodes[v].index = v;
		std::vector<std::pair<int, int>> heads(n), stack, heap;
		auto join = [&](std::pair<int, int> a, std::pair<int, int> b, Kind kind) {
			int id = int(nodes.size());
			nodes.push_back({-1, a.second, b.second, kind == Compress ? paths++ : points++, kind});
			nodes[a.second].parent = nodes[b.second].parent = id;
			order.push_back(id);
			return std::pair<int, int>{std::max(a.first, b.first) + 1, id};
		};
		auto merge = [&] {
			auto b = stack.back(); stack.pop_back();
			auto a = stack.back(); stack.back() = join(a, b, Compress);
		};
		for (int i = n - 1; i >= 0; --i) {
			int head = tree.order[i];
			if (tree.head[head] != head) continue;
			stack.clear();
			for (int v = head; v != -1; v = tree.heavy[v]) {
				heap.clear();
				for (int j = tree.in[v] + 1; j < tree.out[v];) {
					int u = tree.order[j];
					if (u != tree.heavy[v]) heap.push_back(heads[u]);
					j = tree.out[u];
				}
				using Greater = std::greater<std::pair<int, int>>;
				std::make_heap(heap.begin(), heap.end(), Greater{});
				while (heap.size() > 1) {
					std::pop_heap(heap.begin(), heap.end(), Greater{});
					auto a = heap.back(); heap.pop_back();
					std::pop_heap(heap.begin(), heap.end(), Greater{});
					auto b = heap.back(); heap.back() = join(a, b, Rake);
					std::push_heap(heap.begin(), heap.end(), Greater{});
				}
				int h = 0;
				if (!heap.empty()) {
					nodes[v].left = heap[0].second;
					nodes[heap[0].second].parent = v;
					h = heap[0].first + 1;
				}
				order.push_back(v);
				stack.emplace_back(h, v);
				while (true) {
					int s = int(stack.size());
					if (s >= 3 && (stack[s - 3].first == stack[s - 2].first || stack[s - 3].first <= stack[s - 1].first)) {
						auto last = stack.back(); stack.pop_back();
						merge(), stack.push_back(last);
					} else if (s >= 2 && stack[s - 2].first <= stack[s - 1].first) merge();
					else break;
				}
			}
			while (stack.size() > 1) merge();
			heads[head] = stack[0];
			root = stack[0].second, height = stack[0].first;
		}
	}
};
}


#line 8 "tree/static_top_tree.hpp"

template <class DP>
class StaticTopTree {
	using Structure = static_top_tree_detail::Structure;
	using PathValue = typename DP::Path;
	using PointValue = typename DP::Point;
	struct Link { int index = -1; Structure::Kind kind = Structure::Vertex; bool right = false; };
	struct VertexNode { typename DP::Vertex info; PointValue light; Link up; };
	struct PathNode { PathValue child[2]; Link up; };
	struct PointNode { PointValue child[2]; Link up; };
	std::vector<VertexNode> vertices_;
	std::vector<PathNode> paths_;
	std::vector<PointNode> points_;
	std::vector<int> parent_, edge_child_;
	PathValue all_ = DP::path_identity();
	int root_ = -1;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }

  public:
	using Vertex = typename DP::Vertex;
	using Path = typename DP::Path;
	using Point = typename DP::Point;
	StaticTopTree() = default;
	StaticTopTree(const HLD& tree, std::vector<Vertex> values) { build(tree, std::move(values)); }
	void build(const HLD& tree, std::vector<Vertex> values) {
		assert(int(values.size()) == tree.n);
		Structure structure(tree);
		int n = tree.n;
		vertices_.clear(), vertices_.reserve(n);
		for (auto& value : values) vertices_.push_back({std::move(value), DP::point_identity(), {}});
		paths_.assign(structure.paths - n, PathNode{{DP::path_identity(), DP::path_identity()}, {}});
		points_.assign(structure.points, PointNode{{DP::point_identity(), DP::point_identity()}, {}});
		std::vector<Path> paths(structure.paths, DP::path_identity());
		std::vector<Point> points(structure.points, DP::point_identity());
		auto point = [&](int id) {
			if (id == -1) return DP::point_identity();
			const auto& node = structure.nodes[id];
			return node.kind == Structure::Rake ? points[node.index] : DP::add_edge(paths[node.index]);
		};
		for (int id : structure.order) {
			const auto& node = structure.nodes[id];
			Link up;
			if (node.parent != -1) {
				const auto& p = structure.nodes[node.parent];
				up = {p.kind == Structure::Compress ? p.index - n : p.index, p.kind, p.right == id};
			}
			if (node.kind == Structure::Vertex) {
				auto& v = vertices_[id];
				v.light = point(node.left), v.up = up;
				paths[id] = DP::add_vertex(v.light, v.info);
			} else if (node.kind == Structure::Compress) {
				auto& v = paths_[node.index - n];
				v = {{paths[structure.nodes[node.left].index], paths[structure.nodes[node.right].index]}, up};
				paths[node.index] = DP::compress(v.child[0], v.child[1]);
			} else {
				auto& v = points_[node.index];
				v = {{point(node.left), point(node.right)}, up};
				points[node.index] = DP::rake(v.child[0], v.child[1]);
			}
		}
		all_ = n ? paths[structure.nodes[structure.root].index] : DP::path_identity();
		root_ = structure.root_vertex;
		parent_ = std::move(structure.parent), edge_child_ = std::move(structure.edge_child);
	}
	int size() const { return int(vertices_.size()); }
	bool empty() const { return vertices_.empty(); }
	int root() const { return root_; }
	int parent(int v) const { check(v); return parent_[v]; }
	int edge_vertex(int e) const {
		assert(0 <= e && e < int(edge_child_.size()));
		return edge_child_[e];
	}
	const Vertex& get(int v) const { check(v); return vertices_[v].info; }
	void set(int v, const Vertex& value) {
		check(v);
		auto& vertex = vertices_[v];
		vertex.info = value;
		Path path = DP::add_vertex(vertex.light, vertex.info);
		Link up = vertex.up;
		while (up.index != -1) {
			while (up.index != -1 && up.kind == Structure::Compress) {
				auto& node = paths_[up.index];
				node.child[up.right] = path;
				path = DP::compress(node.child[0], node.child[1]), up = node.up;
			}
			if (up.index == -1) break;
			Point point = DP::add_edge(path);
			while (up.kind == Structure::Rake) {
				auto& node = points_[up.index];
				node.child[up.right] = point;
				point = DP::rake(node.child[0], node.child[1]), up = node.up;
			}
			auto& node = vertices_[up.index];
			node.light = point;
			path = DP::add_vertex(point, node.info), up = node.up;
		}
		all_ = path;
	}
	Path all_prod() const { return all_; }
	Point dp() const { return DP::add_edge(all_); }
	Path subtree_prod(int v) const {
		check(v);
		const auto& vertex = vertices_[v];
		Path result = DP::add_vertex(vertex.light, vertex.info);
		Link up = vertex.up;
		while (up.index != -1 && up.kind == Structure::Compress) {
			const auto& node = paths_[up.index];
			if (!up.right) result = DP::compress(result, node.child[1]);
			up = node.up;
		}
		return result;
	}
	Path prod(int v) const {
		check(v);
		Point light = vertices_[v].light;
		Path result = DP::path_identity();
		Link up = vertices_[v].up;
		while (true) {
			Path above = DP::path_identity(), below = DP::path_identity();
			while (up.index != -1 && up.kind == Structure::Compress) {
				const auto& node = paths_[up.index];
				if (!up.right) below = DP::compress(below, node.child[1]);
				else above = DP::compress(node.child[0], above);
				up = node.up;
			}
			light = DP::rake(light, DP::add_edge(below));
			result = DP::compress(above, DP::compress(DP::add_vertex(light, vertices_[v].info), result));
			if (up.index == -1) return result;
			light = DP::point_identity();
			while (up.kind == Structure::Rake) {
				const auto& node = points_[up.index];
				if (!up.right) light = DP::rake(light, node.child[1]);
				else light = DP::rake(node.child[0], light);
				up = node.up;
			}
			v = up.index, up = vertices_[v].up;
		}
	}
};


#line 1 "components/fast_io.hpp"



#line 6 "components/fast_io.hpp"
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 10 "components/fast_io.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 1 "test/fixtures/static_top_tree_dp.hpp"



#line 1 "math/mint.hpp"


#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 5 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 8 "math/mint.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct [[gnu::warn_unused]] Mint {
	using mint = Mint;
	unsigned v = 0;
	[[nodiscard]] static constexpr unsigned mod() { return 998244353; }
	[[nodiscard, gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[nodiscard, gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[nodiscard, gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[nodiscard, gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[nodiscard, gnu::always_inline]] Mint operator+() const { return *this; }
	[[nodiscard, gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[nodiscard, gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[nodiscard, gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[nodiscard, gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	[[nodiscard]] Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[nodiscard, gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[nodiscard, gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	[[nodiscard]] Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[nodiscard, gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 5 "test/fixtures/static_top_tree_dp.hpp"

struct StaticTopTreeSumDP {
	struct Vertex { mint value = 0, a = 1, b = 0; };
	struct Point { mint sum, count; };
	struct Path { mint a, b, sum, count; };
	static Point point_identity() { return {0, 0}; }
	static Path path_identity() { return {1, 0, 0, 0}; }
	static Point rake(const Point& x, const Point& y) { return {x.sum + y.sum, x.count + y.count}; }
	static Point add_edge(const Path& x) { return {x.sum, x.count}; }
	static Path add_vertex(const Point& p, const Vertex& v) {
		mint count = p.count + 1;
		return {v.a, v.b, v.a * (p.sum + v.value) + v.b * count, count};
	}
	static Path compress(const Path& x, const Path& y) {
		return {x.a * y.a, x.a * y.b + x.b, x.sum + x.a * y.sum + x.b * y.count, x.count + y.count};
	}
};

struct StaticTopTreeRerootDP {
	using Vertex = StaticTopTreeSumDP::Vertex;
	using Point = StaticTopTreeSumDP::Point;
	using Half = StaticTopTreeSumDP::Path;
	struct Path { Half forward, reverse; };
	static Point point_identity() { return {0, 0}; }
	static Path path_identity() { return {StaticTopTreeSumDP::path_identity(), StaticTopTreeSumDP::path_identity()}; }
	static Point rake(const Point& x, const Point& y) { return StaticTopTreeSumDP::rake(x, y); }
	static Point add_edge(const Path& x) { return {x.forward.sum, x.forward.count}; }
	static Path add_vertex(const Point& p, const Vertex& v) {
		return {StaticTopTreeSumDP::add_vertex(p, v), {v.a, v.b, p.sum + v.value, p.count + 1}};
	}
	static Path compress(const Path& x, const Path& y) {
		return {StaticTopTreeSumDP::compress(x.forward, y.forward), StaticTopTreeSumDP::compress(y.reverse, x.reverse)};
	}
};


#line 5 "test/point_set_tree_path_composite_sum_fixed_root.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	using DP = StaticTopTreeSumDP;
	std::vector<DP::Vertex> values(n);
	for (auto& v : values) { unsigned x = 0; input.read(x); v.value = x; }
	HLD tree(n);
	std::vector<std::pair<mint, mint>> functions;
	for (int i = 0; i < n - 1; ++i) {
		int u = 0, v = 0;
		unsigned a = 0, b = 0;
		input.read(u), input.read(v), input.read(a), input.read(b);
		tree.add_edge(u, v), functions.emplace_back(a, b);
	}
	tree.build();
	for (int i = 0; i < n - 1; ++i) {
		auto [u, v] = tree.edges[i];
		if (tree.parent[u] == v) std::swap(u, v);
		values[v].a = functions[i].first, values[v].b = functions[i].second;
	}
	StaticTopTree<DP> dp(tree, std::move(values));
	for (int j = 0; j < q; ++j) {
		int type = 0, i = 0;
		unsigned x = 0, y = 0;
		input.read(type), input.read(i), input.read(x);
		if (type == 0) {
			auto value = dp.get(i); value.value = x; dp.set(i, value);
		} else {
			input.read(y);
			int v = dp.edge_vertex(i);
			auto value = dp.get(v); value.a = x, value.b = y; dp.set(v, value);
		}
		output.println(dp.all_prod().sum.val());
	}
}
