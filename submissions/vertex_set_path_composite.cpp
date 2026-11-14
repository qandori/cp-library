#line 1 "test/hld_path_composite.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"
#line 1 "tree/path_product.hpp"



#line 1 "tree/hld.hpp"



#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

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


#line 5 "tree/path_product.hpp"
#include <bit>
#include <cstdint>
#include <type_traits>

template<class S, auto op, auto e, auto reverse = nullptr> class TreePathProduct {
	static constexpr bool has_reverse = !std::is_same_v<decltype(reverse), std::nullptr_t>;
	struct Empty {};
	struct Back { S prod, prefix; };
	using BackStorage = std::conditional_t<has_reverse, Empty, Back>;
	struct alignas(64) Node {
		int left = 0, right = 0, parent = 0, head = 0, up = 0;
		unsigned char priority = 0;
		S value, prod, prefix;
		[[no_unique_address]] BackStorage back;
		static BackStorage make_back(const S& value) {
			if constexpr (has_reverse) return {};
			else return {value, value};
		}
		explicit Node(const S& v) : value(v), prod(v), prefix(v), back(make_back(v)) {}
	};
	std::vector<Node> nodes_;
	std::vector<int> index_;
	std::vector<S> dense_;
	std::vector<int> dense_heads_, dense_up_;
	unsigned base_ = 1;
	bool flat_ = false;
	int index(int v) const {
		assert(0 <= v && v < size());
		return index_[v];
	}
	void pull(int v) {
		auto& node = nodes_[v];
		node.prefix = node.left ? op(nodes_[node.left].prod, node.value) : node.value;
		node.prod = node.right ? op(node.prefix, nodes_[node.right].prod) : node.prefix;
		if constexpr (!has_reverse) {
			node.back.prefix = node.left ? op(node.value, nodes_[node.left].back.prod) : node.value;
			node.back.prod = node.right ? op(nodes_[node.right].back.prod, node.back.prefix) : node.back.prefix;
		}
	}
	S back_prod(int v) const {
		if constexpr (has_reverse) return reverse(nodes_[v].prod);
		else return nodes_[v].back.prod;
	}
	S back_prefix(int v) const {
		if constexpr (has_reverse) return reverse(nodes_[v].prefix);
		else return nodes_[v].back.prefix;
	}
	template<bool Reverse> S dense_interval(int l, int r) const {
		unsigned a = base_ + unsigned(l - 1), b = base_ + unsigned(r);
		S left = e(), right = e();
		while (a < b) {
			if (a & 1) left = op(left, dense_[a++]);
			if (b & 1) right = op(dense_[--b], right);
			a >>= 1, b >>= 1;
		}
		S result = op(left, right);
		if constexpr (Reverse && has_reverse) return reverse(result);
		else return result;
	}
	template<bool Reverse> S prefix(int v) const {
		S result = e();
		int last = v;
		for (; v; v = nodes_[v].parent) if (v <= last) {
			if constexpr (Reverse) result = op(result, back_prefix(v));
			else result = op(nodes_[v].prefix, result);
		}
		return result;
	}
	template<bool Reverse> S interval(int l, int r) const {
		int a = l, b = r;
		S left = e(), right = e();
		while (a != b) {
			if (nodes_[a].priority < nodes_[b].priority) {
				if (l <= a) {
					const auto& node = nodes_[a];
					if constexpr (Reverse) left = op(op(back_prod(node.right), node.value), left);
					else left = op(left, op(node.value, nodes_[node.right].prod));
				}
				a = nodes_[a].parent;
			} else {
				if (b <= r) {
					if constexpr (Reverse) right = op(right, back_prefix(b));
					else right = op(nodes_[b].prefix, right);
				}
				b = nodes_[b].parent;
			}
		}
		if constexpr (Reverse) return op(right, op(nodes_[a].value, left));
		else return op(left, op(nodes_[a].value, right));
	}

	template<bool Reverse, class X, class Apply> X apply_interval(int l, int r, X value, Apply& mapping) const {
		int a = l, b = r;
		S outside = e();
		while (a != b) {
			if (nodes_[a].priority < nodes_[b].priority) {
				if (l <= a) {
					const auto& node = nodes_[a];
					if constexpr (Reverse) outside = op(op(back_prod(node.right), node.value), outside);
					else value = mapping(nodes_[node.right].prod, mapping(node.value, std::move(value)));
				}
				a = nodes_[a].parent;
			} else {
				if (b <= r) {
					if constexpr (Reverse) value = mapping(back_prefix(b), std::move(value));
					else outside = op(nodes_[b].prefix, outside);
				}
				b = nodes_[b].parent;
			}
		}
		return mapping(outside, mapping(nodes_[a].value, std::move(value)));
	}

	template<class X, class Apply> X dense_apply(int u, int v, X value, Apply& mapping) const {
		std::array<std::pair<int, int>, 4> right;
		int count = 0;
		while (dense_heads_[u] != dense_heads_[v]) {
			if (dense_heads_[u] > dense_heads_[v]) {
				value = mapping(dense_interval<true>(dense_heads_[u], u), std::move(value));
				u = dense_up_[u];
			} else {
				assert(count < int(right.size()));
				right[count++] = {dense_heads_[v], v};
				v = dense_up_[v];
			}
		}
		if (u <= v) value = mapping(dense_interval<false>(u, v), std::move(value));
		else value = mapping(dense_interval<true>(v, u), std::move(value));
		while (count) {
			auto [l, r] = right[--count];
			value = mapping(dense_interval<false>(l, r), std::move(value));
		}
		return value;
	}

  public:
	TreePathProduct() = default;
	TreePathProduct(const HLD& tree, const std::vector<S>& values) { build(tree, values); }
	void build(const HLD& tree, const std::vector<S>& values) {
		assert(values.size() == std::size_t(tree.n) && tree.order.size() == values.size());
		index_.resize(values.size());
		nodes_.clear();
		for (int v : tree.order) index_[v] = tree.in[v] + 1;
		flat_ = false;
		dense_.clear();
		if constexpr (has_reverse) {
			std::vector<unsigned char> levels(values.size());
			unsigned maximum = 0;
			for (int v : tree.order) {
				if (tree.parent[v] != -1) levels[v] = levels[tree.parent[v]] + (tree.head[v] == v);
				maximum = std::max(maximum, unsigned(levels[v]));
			}
			flat_ = maximum <= 4;
		}
		if (flat_) {
			base_ = std::bit_ceil(unsigned(size()) ? unsigned(size()) : 1U);
			dense_.assign(std::size_t(base_) * 2, e());
			dense_heads_.assign(values.size() + 1, 0), dense_up_.assign(values.size() + 1, 0);
			for (int v : tree.order) {
				int id = index_[v], h = tree.head[v];
				dense_heads_[id] = index_[h];
				dense_up_[id] = tree.parent[h] == -1 ? 0 : index_[tree.parent[h]];
				dense_[base_ + unsigned(id - 1)] = values[v];
			}
			for (unsigned i = base_ - 1; i; --i) dense_[i] = op(dense_[i * 2], dense_[i * 2 + 1]);
			return;
		}
		dense_heads_.clear(), dense_up_.clear();
		nodes_.reserve(values.size() + 1);
		nodes_.emplace_back(e());
		for (int v : tree.order) nodes_.emplace_back(values[v]);
		std::vector<int> stack, order;
		order.reserve(values.size());
		for (int v : tree.order) {
			if (tree.head[v] != v) continue;
			stack.clear();
			unsigned position = 0;
			int head = index_[v], up = tree.parent[v] == -1 ? 0 : index_[tree.parent[v]];
			for (int u = v; u != -1; u = tree.heavy[u]) {
				int id = index_[u], weight = tree.size[u] - (tree.heavy[u] == -1 ? 0 : tree.size[tree.heavy[u]]);
				auto& node = nodes_[id];
				node.head = head, node.up = up;
				node.priority = std::bit_width(position ^ (position + unsigned(weight)));
				position += unsigned(weight);
				int last = 0;
				while (!stack.empty() && nodes_[stack.back()].priority < node.priority) {
					last = stack.back();
					stack.pop_back();
				}
				node.left = last;
				if (last) nodes_[last].parent = id;
				if (!stack.empty()) nodes_[stack.back()].right = id, node.parent = stack.back();
				stack.push_back(id);
			}
			order.push_back(stack[0]);
		}
		for (std::size_t i = 0; i < order.size(); ++i) {
			const auto& node = nodes_[order[i]];
			if (node.left) order.push_back(node.left);
			if (node.right) order.push_back(node.right);
		}
		for (auto it = order.rbegin(); it != order.rend(); ++it) pull(*it);
	}
	int size() const { return int(index_.size()); }
	bool empty() const { return index_.empty(); }
	const S& get(int v) const {
		int id = index(v);
		return flat_ ? dense_[base_ + unsigned(id - 1)] : nodes_[id].value;
	}
	void set(int v, const S& value) {
		int id = index(v);
		if (flat_) {
			unsigned k = base_ + unsigned(id - 1);
			dense_[k] = value;
			for (k >>= 1; k; k >>= 1) dense_[k] = op(dense_[k * 2], dense_[k * 2 + 1]);
			return;
		}
		nodes_[id].value = value;
		for (; id; id = nodes_[id].parent) pull(id);
	}
	S prod(int u, int v) const {
		u = index(u), v = index(v);
		if (flat_) {
			auto append = [](const S& part, S value) { return op(std::move(value), part); };
			return dense_apply(u, v, e(), append);
		}
		S left = e(), right = e();
		while (nodes_[u].head != nodes_[v].head) {
			if (nodes_[u].head > nodes_[v].head) {
				left = op(left, prefix<true>(u));
				u = nodes_[u].up;
			} else {
				right = op(prefix<false>(v), right);
				v = nodes_[v].up;
			}
		}
		S middle = u <= v ? interval<false>(u, v) : interval<true>(v, u);
		return op(left, op(middle, right));
	}
	template<class X, class Apply> X apply(int u, int v, X value, Apply mapping) const {
		u = index(u), v = index(v);
		if (flat_) return dense_apply(u, v, std::move(value), mapping);
		S right = e();
		while (nodes_[u].head != nodes_[v].head) {
			if (nodes_[u].head > nodes_[v].head) {
				for (int id = u; id; id = nodes_[id].parent)
					if (id <= u) value = mapping(back_prefix(id), std::move(value));
				u = nodes_[u].up;
			} else {
				right = op(prefix<false>(v), right);
				v = nodes_[v].up;
			}
		}
		if (u <= v) value = apply_interval<false>(u, v, std::move(value), mapping);
		else value = apply_interval<true>(v, u, std::move(value), mapping);
		return mapping(right, std::move(value));
	}

};


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


#line 1 "components/fast_io_bulk.hpp"



#line 5 "components/fast_io_bulk.hpp"

#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
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


#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "components/fast_io_bulk.hpp"
#include <span>
#line 10 "components/fast_io_bulk.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	template <bool Signed = false>
	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid, unsigned *signs = nullptr) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		unsigned n0 = 0, n1 = 0;
		if constexpr (Signed) {
			if (a < 1 || a > 11 || b <= a + 1 || b - a - 1 > 11) [[unlikely]] {
				invalid = _mm256_set1_epi32(-1);
				p += std::min(b + 1, 32u);
				*signs = 0;
				return _mm256_setzero_si256();
			}
			n0 = p[0] == '-', n1 = p[a + 1] == '-';
			*signs = n0 | (n1 << 1);
		}
		if (a < n0 + 1 || a - n0 > 10 || b <= a + n1 + 1 || b - a - n1 - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		auto window = bytes;
		if constexpr (Signed) window = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1 + n1)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + n0)));
		else window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - n1 - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a - n0].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	template <bool Signed = false>
	static std::uint32_t one(const char *&p, bool &bad) {
		bool negative = false;
		if constexpr (Signed) negative = *p == '-', p += negative;
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		if constexpr (Signed) bad |= value > 2147483647ULL + negative;
		else bad |= value > 0xffffffffULL;
		p += n + 1;
		return negative ? 0u - std::uint32_t(value) : std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	template <bool Signed = false>
	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				unsigned signs0 = 0, signs1 = 0, signs2 = 0, signs3 = 0;
				const auto g0 = two<Signed>(p[0], invalid, &signs0), g1 = two<Signed>(p[1], invalid, &signs1), g2 = two<Signed>(p[2], invalid, &signs2), g3 = two<Signed>(p[3], invalid, &signs3);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				auto max_high = _mm256_set1_epi32(42), max_low = _mm256_set1_epi32(94967295);
				auto signs = _mm256_setzero_si256();
				if constexpr (Signed) {
					signs = _mm256_setr_epi32(signs0 & 1, signs1 & 1, signs2 & 1, signs3 & 1, signs0 >> 1, signs1 >> 1, signs2 >> 1, signs3 >> 1);
					max_high = _mm256_set1_epi32(21), max_low = _mm256_add_epi32(_mm256_set1_epi32(47483647), signs);
				}
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, max_high),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, max_high), _mm256_cmpgt_epi32(low, max_low)));
				invalid = _mm256_or_si256(invalid, overflow);
				auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				if constexpr (Signed) {
					const auto mask = _mm256_sub_epi32(_mm256_setzero_si256(), signs);
					value = _mm256_sub_epi32(_mm256_xor_si256(value, mask), mask);
				}
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <bool Signed = false, class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep<Signed>(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one<Signed>(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <bool Signed = false, class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks<Signed>(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (!Signed && done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				std::conditional_t<Signed, std::int32_t, unsigned> value = 0;
				in.read(value);
				emit(reinterpret_cast<unsigned *>(&value), 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, class T, std::size_t... I>
void emit_record(Emit &emit, const T *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template <class Emit>
void read_signed_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks<true>(input, count, [&](const unsigned *values, std::size_t n) {
			emit(reinterpret_cast<const std::int32_t *>(values), n);
		});
		return;
	}
#endif
	std::int32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

template <std::size_t Fields, class Emit>
void read_signed_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::int32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_signed_blocks(input, count * Fields, [&](const std::int32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 7 "test/hld_path_composite.test.cpp"

struct S {
	mint a, b, reverse_b;
};
inline S op(S x, S y) { return {x.a * y.a, x.b * y.a + y.b, y.reverse_b * x.a + x.reverse_b}; }
inline S e() { return {1, 0, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	std::size_t position = 0;
	read_unsigned_records<2>(input, n, [&](unsigned a, unsigned b) { values[position++] = {a, b, b}; });
	HLD tree(n);
	read_unsigned_records<2>(input, n - 1, [&](unsigned u, unsigned v) { tree.add_edge(u, v); });
	tree.build();
	TreePathProduct<S, op, e, [](S value) {
		std::swap(value.b, value.reverse_b);
		return value;
	}> seg(tree, values);
	read_unsigned_records<4>(input, q, [&](unsigned type, unsigned u, unsigned a, unsigned b) {
		if (type == 0) seg.set(u, {a, b, b});
		else {
			auto x = seg.apply(u, a, mint(b), [](const S& f, mint value) { return f.a * value + f.b; });
			output.println(x.val());
		}
	});
}
