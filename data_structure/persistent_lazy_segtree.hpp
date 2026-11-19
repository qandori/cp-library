#ifndef PERSISTENT_LAZY_SEGTREE_HPP
#define PERSISTENT_LAZY_SEGTREE_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class PersistentLazySegtree {
  public:
	using value_type = S;
	using root_type = std::uint32_t;
  private:
	struct Node { S value; F lazy; root_type left, right; bool pending = false; };
	std::vector<Node> nodes{{e(), id(), 0, 0}};
	int n_ = 0;
	root_type initial_ = 0;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type join(root_type left, root_type right) { return make({op(nodes[left].value, nodes[right].value), id(), left, right}); }
	root_type build(const std::vector<S> &a, int l, int r) {
		if (r - l == 1) return make({a[l], id(), 0, 0});
		int m = l + (r - l) / 2;
		auto left = build(a, l, m), right = build(a, m, r);
		return join(left, right);
	}
	void inherit(root_type root, F &carry, bool &active) const {
		if (nodes[root].pending) {
			carry = active ? composition(carry, nodes[root].lazy) : nodes[root].lazy;
			active = true;
		}
	}
	root_type mapped(root_type root, F f, int length) {
		const auto &node = nodes[root];
		return make({mapping(f, node.value, length), node.pending ? composition(f, node.lazy) : std::move(f), node.left, node.right, true});
	}
	root_type carried(root_type root, F carry, int length, bool active) {
		return active ? mapped(root, std::move(carry), length) : root;
	}
	template <bool Prefix>
	root_type update_end(root_type root, int l, int r, int end, F f, F carry, bool active) {
		if (Prefix ? r <= end : end <= l) return mapped(root, active ? composition(f, carry) : std::move(f), r - l);
		inherit(root, carry, active);
		auto a = nodes[root].left, b = nodes[root].right;
		int m = l + (r - l) / 2;
		bool right = Prefix ? m < end : m <= end;
		root_type left, next;
		if (right) {
			left = Prefix ? mapped(a, active ? composition(f, carry) : f, m - l) : carried(a, carry, m - l, active);
			next = update_end<Prefix>(b, m, r, end, std::move(f), std::move(carry), active);
		} else {
			left = update_end<Prefix>(a, l, m, end, f, carry, active);
			next = Prefix ? carried(b, std::move(carry), r - m, active) : mapped(b, active ? composition(f, carry) : std::move(f), r - m);
		}
		return join(left, next);
	}
	root_type update(root_type root, int l, int r, int ql, int qr, F f, F carry, bool active) {
		if (ql <= l && r <= qr) return mapped(root, active ? composition(f, carry) : std::move(f), r - l);
		inherit(root, carry, active);
		auto a = nodes[root].left, b = nodes[root].right;
		int m = l + (r - l) / 2;
		root_type left, right;
		if (qr <= m) {
			left = update(a, l, m, ql, qr, f, carry, active);
			right = carried(b, std::move(carry), r - m, active);
		} else if (m <= ql) {
			left = carried(a, carry, m - l, active);
			right = update(b, m, r, ql, qr, std::move(f), std::move(carry), active);
		} else {
			left = update_end<false>(a, l, m, ql, f, carry, active);
			right = update_end<true>(b, m, r, qr, std::move(f), std::move(carry), active);
		}
		return join(left, right);
	}
	root_type assign(root_type root, int l, int r, int p, S value, F carry, bool active) {
		if (r - l == 1) return make({std::move(value), id(), 0, 0});
		inherit(root, carry, active);
		auto a = nodes[root].left, b = nodes[root].right;
		int m = l + (r - l) / 2;
		root_type left, right;
		if (p < m) {
			left = assign(a, l, m, p, std::move(value), carry, active);
			right = carried(b, std::move(carry), r - m, active);
		} else {
			left = carried(a, carry, m - l, active);
			right = assign(b, m, r, p, std::move(value), std::move(carry), active);
		}
		return join(left, right);
	}
	template <bool Prefix>
	S fold(root_type root, int l, int r, int end, F carry, bool active) const {
		S result = e();
		while (Prefix ? end < r : l < end) {
			inherit(root, carry, active);
			const auto &node = nodes[root];
			int m = l + (r - l) / 2;
			bool right = Prefix ? m < end : m <= end;
			if (Prefix ? right : !right) {
				auto child = right ? node.left : node.right;
				S part = active ? mapping(carry, nodes[child].value, right ? m - l : r - m) : nodes[child].value;
				result = Prefix ? op(std::move(result), std::move(part)) : op(std::move(part), std::move(result));
			}
			if (right) { root = node.right; l = m; }
			else { root = node.left; r = m; }
		}
		S part = active ? mapping(carry, nodes[root].value, r - l) : nodes[root].value;
		return Prefix ? op(std::move(result), std::move(part)) : op(std::move(part), std::move(result));
	}
	S query(root_type root, int l, int r, int ql, int qr, F carry, bool active) const {
		const auto &node = nodes[root];
		if (ql <= l && r <= qr) return active ? mapping(carry, node.value, r - l) : node.value;
		inherit(root, carry, active);
		int m = l + (r - l) / 2;
		if (qr <= m) return query(node.left, l, m, ql, qr, std::move(carry), active);
		if (m <= ql) return query(node.right, m, r, ql, qr, std::move(carry), active);
		return op(fold<false>(node.left, l, m, ql, carry, active), fold<true>(node.right, m, r, qr, carry, active));
	}
	template <bool Prefix>
	root_type copy_end(root_type root, root_type source, int l, int r, int end, F carry, F source_carry, bool active, bool source_active) {
		if (Prefix ? r <= end : end <= l) return carried(source, std::move(source_carry), r - l, source_active);
		if (root == source && !active && !source_active) return root;
		inherit(root, carry, active); inherit(source, source_carry, source_active);
		auto a = nodes[root].left, b = nodes[root].right, c = nodes[source].left, d = nodes[source].right;
		int m = l + (r - l) / 2;
		bool right = Prefix ? m < end : m <= end;
		root_type left, next;
		if (right) {
			left = Prefix ? carried(c, source_carry, m - l, source_active) : carried(a, carry, m - l, active);
			next = copy_end<Prefix>(b, d, m, r, end, std::move(carry), std::move(source_carry), active, source_active);
		} else {
			left = copy_end<Prefix>(a, c, l, m, end, carry, source_carry, active, source_active);
			next = Prefix ? carried(b, std::move(carry), r - m, active) : carried(d, std::move(source_carry), r - m, source_active);
		}
		return join(left, next);
	}
	root_type copy(root_type root, root_type source, int l, int r, int ql, int qr, F carry, F source_carry, bool active, bool source_active) {
		if (ql <= l && r <= qr) return carried(source, std::move(source_carry), r - l, source_active);
		if (root == source && !active && !source_active) return root;
		inherit(root, carry, active); inherit(source, source_carry, source_active);
		auto a = nodes[root].left, b = nodes[root].right, c = nodes[source].left, d = nodes[source].right;
		int m = l + (r - l) / 2;
		root_type left, right;
		if (qr <= m) {
			left = copy(a, c, l, m, ql, qr, carry, source_carry, active, source_active);
			right = carried(b, std::move(carry), r - m, active);
		} else if (m <= ql) {
			left = carried(a, carry, m - l, active);
			right = copy(b, d, m, r, ql, qr, std::move(carry), std::move(source_carry), active, source_active);
		} else {
			left = copy_end<false>(a, c, l, m, ql, carry, source_carry, active, source_active);
			right = copy_end<true>(b, d, m, r, qr, std::move(carry), std::move(source_carry), active, source_active);
		}
		return join(left, right);
	}
	template <bool Right, class Pred>
	int search(root_type root, int l, int r, int end, F carry, bool active, S &value, Pred &f) const {
		const auto &node = nodes[root];
		if (Right ? end <= l : r <= end) {
			S part = active ? mapping(carry, node.value, r - l) : node.value;
			S next = Right ? op(value, part) : op(part, value);
			if (f(next)) { value = std::move(next); return Right ? n_ : 0; }
			if (r - l == 1) return Right ? l : r;
		}
		inherit(root, carry, active);
		int m = l + (r - l) / 2;
		if constexpr (Right) {
			if (end < m) { int found = search<Right>(node.left, l, m, end, carry, active, value, f); if (found != n_) return found; }
			return search<Right>(node.right, m, r, end, carry, active, value, f);
		} else {
			if (m < end) { int found = search<Right>(node.right, m, r, end, carry, active, value, f); if (found) return found; }
			return search<Right>(node.left, l, m, end, carry, active, value, f);
		}
	}

  public:
	explicit PersistentLazySegtree(int n = 0) : n_(n) { assert(n >= 0); }
	explicit PersistentLazySegtree(const std::vector<S> &a) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) { reserve(2 * a.size() - 1); initial_ = build(a, 0, n_); }
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	root_type set(root_type root, int p, S value) { check(root); assert(0 <= p && p < n_); return assign(root, 0, n_, p, std::move(value), id(), false); }
	root_type apply(root_type root, int l, int r, F f) {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? root : update(root, 0, n_, l, r, std::move(f), id(), false);
	}
	root_type apply(root_type root, int p, F f) { assert(0 <= p && p < n_); return apply(root, p, p + 1, std::move(f)); }
	S prod(root_type root, int l, int r) const {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? e() : query(root, 0, n_, l, r, id(), false);
	}
	S get(root_type root, int p) const { assert(0 <= p && p < n_); return prod(root, p, p + 1); }
	S all_prod(root_type root) const { check(root); return nodes[root].value; }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		check(root); check(source); assert(0 <= l && l <= r && r <= n_);
		if (l == r || root == source) return root;
		if (l == 0 && r == n_) return source;
		return copy(root, source, 0, n_, l, r, id(), id(), false, false);
	}
	template <auto f> int max_right(root_type root, int l) const { return max_right(root, l, f); }
	template <class Pred> int max_right(root_type root, int l, Pred f) const {
		check(root); assert(0 <= l && l <= n_ && f(e()));
		S value = e();
		return l == n_ ? n_ : search<true>(root, 0, n_, l, id(), false, value, f);
	}
	template <auto f> int min_left(root_type root, int r) const { return min_left(root, r, f); }
	template <class Pred> int min_left(root_type root, int r, Pred f) const {
		check(root); assert(0 <= r && r <= n_ && f(e()));
		S value = e();
		return r == 0 ? 0 : search<false>(root, 0, n_, r, id(), false, value, f);
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
