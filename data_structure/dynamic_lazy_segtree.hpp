#ifndef DYNAMIC_LAZY_SEGTREE_HPP
#define DYNAMIC_LAZY_SEGTREE_HPP

#include "../components/ordered_midpoint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id,
	auto initial = nullptr, class Coord = std::int64_t>
class DynamicLazySegtree {
	static_assert(std::is_integral_v<Coord> && !std::is_same_v<Coord, bool> && sizeof(Coord) <= 8);
	using U = std::make_unsigned_t<Coord>;
	struct Node {
		S value;
		F lazy;
		int left = 0, right = 0;
		bool dirty = false;
	};
	std::vector<Node> nodes{Node{e(), id()}};
	int root = 0;
	Coord first, last;
	static bool leaf(Coord l, Coord r) { return U(U(r) - U(l)) == 1; }
	static S base(Coord l, Coord r) {
		if (l == r) return e();
		if constexpr (std::is_same_v<decltype(initial), std::nullptr_t>) return e();
		else return initial(l, r);
	}
	S value(int k, Coord l, Coord r) const { return k ? nodes[k].value : base(l, r); }
	int make_node(Coord l, Coord r) {
		assert(nodes.size() < std::size_t(std::numeric_limits<int>::max()));
		nodes.push_back(Node{base(l, r), id()});
		return static_cast<int>(nodes.size() - 1);
	}
	void all_apply(int k, const F &f, bool leaf) {
		nodes[k].value = mapping(f, std::move(nodes[k].value));
		if (!leaf) {
			nodes[k].lazy = nodes[k].dirty ? composition(f, nodes[k].lazy) : f;
			nodes[k].dirty = true;
		}
	}
	void push(int k, Coord l, Coord m, Coord r) {
		if (!nodes[k].dirty) return;
		F f = nodes[k].lazy;
		if (!nodes[k].left) nodes[k].left = make_node(l, m);
		if (!nodes[k].right) nodes[k].right = make_node(m, r);
		all_apply(nodes[k].left, f, leaf(l, m));
		all_apply(nodes[k].right, f, leaf(m, r));
		nodes[k].dirty = false;
	}
	void pull(int k, Coord l, Coord m, Coord r) {
		nodes[k].value = op(value(nodes[k].left, l, m), value(nodes[k].right, m, r));
	}
	int update(int k, Coord l, Coord r, Coord a, Coord b, const F &f) {
		if (!k) k = make_node(l, r);
		if (a == l && b == r) { all_apply(k, f, leaf(l, r)); return k; }
		Coord m = ordered_midpoint(l, r);
		push(k, l, m, r);
		if (a < m) nodes[k].left = update(nodes[k].left, l, m, a, std::min(b, m), f);
		if (m < b) nodes[k].right = update(nodes[k].right, m, r, std::max(a, m), b, f);
		pull(k, l, m, r);
		return k;
	}
	int write(int k, Coord l, Coord r, Coord p, const S &x) {
		if (!k) k = make_node(l, r);
		if (leaf(l, r)) { nodes[k].value = x; return k; }
		Coord m = ordered_midpoint(l, r);
		push(k, l, m, r);
		if (p < m) nodes[k].left = write(nodes[k].left, l, m, p, x);
		else nodes[k].right = write(nodes[k].right, m, r, p, x);
		pull(k, l, m, r);
		return k;
	}
	S product(int k, Coord l, Coord r, Coord a, Coord b) const {
		if (!k) return base(a, b);
		if (a == l && b == r) return nodes[k].value;
		Coord m = ordered_midpoint(l, r);
		S result = b <= m ? product(nodes[k].left, l, m, a, b) :
			m <= a ? product(nodes[k].right, m, r, a, b) :
			op(product(nodes[k].left, l, m, a, m), product(nodes[k].right, m, r, m, b));
		return nodes[k].dirty ? mapping(nodes[k].lazy, std::move(result)) : result;
	}
	template<bool Right, class Pred>
	bool search(int k, Coord l, Coord r, Coord bound, S &acc, const F *carry, Pred &f, Coord &answer) const {
		if ((Right ? bound <= l : r <= bound)) {
			S part = value(k, l, r);
			if (carry) part = mapping(*carry, std::move(part));
			S next = Right ? op(acc, part) : op(part, acc);
			if (f(next)) { acc = std::move(next); return false; }
			if (leaf(l, r)) { answer = Right ? l : r; return true; }
		}
		Coord m = ordered_midpoint(l, r);
		auto descend = [&](const F *pending) {
			int left = k ? nodes[k].left : 0, right = k ? nodes[k].right : 0;
			if constexpr (Right) {
				if (bound < m && search<Right>(left, l, m, bound, acc, pending, f, answer)) return true;
				return search<Right>(right, m, r, bound, acc, pending, f, answer);
			} else {
				if (m < bound && search<Right>(right, m, r, bound, acc, pending, f, answer)) return true;
				return search<Right>(left, l, m, bound, acc, pending, f, answer);
			}
		};
		if (k && nodes[k].dirty) {
			F pending = carry ? composition(*carry, nodes[k].lazy) : nodes[k].lazy;
			return descend(&pending);
		}
		return descend(carry);
	}
	int build_values(const std::vector<S> &a, Coord l, Coord r) {
		int k = make_node(l, r);
		if (leaf(l, r)) nodes[k].value = a[std::size_t(l)];
		else {
			Coord m = ordered_midpoint(l, r);
			int left = build_values(a, l, m), right = build_values(a, m, r);
			nodes[k].left = left;
			nodes[k].right = right;
			pull(k, l, m, r);
		}
		return k;
	}
	void check([[maybe_unused]] Coord l, [[maybe_unused]] Coord r) const {
		assert(first <= l && l <= r && r <= last);
	}

  public:
	using value_type = S;
	using update_type = F;
	using coordinate_type = Coord;
	explicit DynamicLazySegtree(Coord n = 0) : DynamicLazySegtree(0, n) {}
	DynamicLazySegtree(Coord l, Coord r) : first(l), last(r) { assert(l <= r); }
	explicit DynamicLazySegtree(const std::vector<S> &a) : first(0), last(0) { build(a); }
	DynamicLazySegtree(const DynamicLazySegtree &) = default;
	DynamicLazySegtree &operator=(const DynamicLazySegtree &) = default;
	DynamicLazySegtree(DynamicLazySegtree &&other) : DynamicLazySegtree(other.first, other.last) { swap(other); }
	DynamicLazySegtree &operator=(DynamicLazySegtree &&other) {
		if (this != &other) { DynamicLazySegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(DynamicLazySegtree &other) noexcept {
		nodes.swap(other.nodes);
		std::swap(root, other.root);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::uintmax_t(std::numeric_limits<Coord>::max()));
		assert(a.size() < std::size_t(std::numeric_limits<int>::max()) / 2);
		clear();
		first = 0;
		last = static_cast<Coord>(a.size());
		reserve(a.empty() ? 0 : 2 * a.size() - 1);
		if (!a.empty()) root = build_values(a, first, last);
	}
	void reserve(std::size_t count) {
		assert(count < std::size_t(std::numeric_limits<int>::max()));
		nodes.reserve(count + 1);
	}
	void clear() { nodes.erase(nodes.begin() + 1, nodes.end()); root = 0; }
	U size() const { return U(last) - U(first); }
	bool empty() const { return first == last; }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	S all_prod() const { return value(root, first, last); }
	S prod(Coord l, Coord r) const { check(l, r); return l == r ? e() : product(root, first, last, l, r); }
	S get(Coord p) const {
		assert(first <= p && p < last);
		std::array<int, std::numeric_limits<U>::digits> path;
		unsigned count = 0;
		int k = root;
		Coord l = first, r = last;
		while (k && !leaf(l, r)) {
			if (nodes[k].dirty) path[count++] = k;
			Coord m = ordered_midpoint(l, r);
			if (p < m) k = nodes[k].left, r = m;
			else k = nodes[k].right, l = m;
		}
		S result = k ? nodes[k].value : base(p, Coord(p + 1));
		while (count) result = mapping(nodes[path[--count]].lazy, std::move(result));
		return result;
	}
	void set(Coord p, const S &x) { assert(first <= p && p < last); root = write(root, first, last, p, x); }
	void apply(Coord l, Coord r, const F &f) {
		check(l, r);
		if (l != r) root = update(root, first, last, l, r, f);
	}
	void apply(Coord p, const F &f) { assert(first <= p && p < last); apply(p, Coord(p + 1), f); }
	template <auto f> Coord max_right(Coord l) const { return max_right(l, f); }
	template<class Pred>
	Coord max_right(Coord l, Pred f) const {
		assert(first <= l && l <= last && f(e()));
		Coord answer = last;
		if (l != last) { S acc = e(); search<true>(root, first, last, l, acc, nullptr, f, answer); }
		return answer;
	}
	template <auto f> Coord min_left(Coord r) const { return min_left(r, f); }
	template<class Pred>
	Coord min_left(Coord r, Pred f) const {
		assert(first <= r && r <= last && f(e()));
		Coord answer = first;
		if (r != first) { S acc = e(); search<false>(root, first, last, r, acc, nullptr, f, answer); }
		return answer;
	}
};

#endif
