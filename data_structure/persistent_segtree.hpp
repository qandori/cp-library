#ifndef PERSISTENT_SEGTREE_HPP
#define PERSISTENT_SEGTREE_HPP

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e>
class PersistentSegtree {
  public:
	using value_type = S;
	using root_type = std::uint32_t;

  private:
	struct Node { S value; std::array<root_type, 4> child; };
	int n_ = 0, height = 0;
	root_type initial_ = 0;
	std::vector<Node> nodes{{e(), {}}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	std::uint64_t capacity() const { return std::uint64_t(1) << (2 * height); }
	void initialize(int n) {
		assert(n >= 0);
		n_ = n;
		height = n ? (std::bit_width(static_cast<unsigned>(n - 1)) + 1) / 2 : 0;
	}
	root_type make(S value, std::array<root_type, 4> child = {}) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back({std::move(value), child});
		return result;
	}
	S combine(root_type a, root_type b) const {
		if (!a) return nodes[b].value;
		if (!b) return nodes[a].value;
		return op(nodes[a].value, nodes[b].value);
	}
	root_type join(std::array<root_type, 4> child) {
		if constexpr (std::is_trivially_copyable_v<S>) {
			return make(op(op(nodes[child[0]].value, nodes[child[1]].value), op(nodes[child[2]].value, nodes[child[3]].value)), child);
		} else {
			if (!(child[2] | child[3])) return make(combine(child[0], child[1]), child);
			if (!(child[0] | child[1])) return make(combine(child[2], child[3]), child);
			return make(op(combine(child[0], child[1]), combine(child[2], child[3])), child);
		}
	}
	root_type build(const std::vector<S> &a, std::uint64_t l, std::uint64_t width) {
		if (l >= a.size()) return 0;
		if (width == 1) return make(a[l]);
		width >>= 2;
		std::array<root_type, 4> child;
		for (int i = 0; i < 4; ++i) child[i] = build(a, l + i * width, width);
		return join(child);
	}
	template <bool Append>
	root_type update(root_type root, int p, S value) {
		check(root); assert(0 <= p && p < n_);
		std::array<std::array<root_type, 4>, 16> path;
		for (int level = height - 1; level >= 0; --level) {
			path[level] = nodes[root].child;
			root = path[level][(static_cast<unsigned>(p) >> (2 * level)) & 3];
		}
		if constexpr (Append) value = op(nodes[root].value, value);
		root = make(std::move(value));
		for (int level = 0; level < height; ++level) {
			path[level][(static_cast<unsigned>(p) >> (2 * level)) & 3] = root;
			root = join(path[level]);
		}
		return root;
	}
	S prefix(root_type root, int level, std::uint64_t end) const {
		auto width = std::uint64_t(1) << (2 * level);
		S result = e();
		while (root && end < width) {
			width >>= 2;
			unsigned d = static_cast<unsigned>(end >> (2 * --level));
			const auto &child = nodes[root].child;
			if (d >= 2) result = op(std::move(result), op(nodes[child[0]].value, nodes[child[1]].value));
			if (d & 1) result = op(std::move(result), nodes[child[d - 1]].value);
			end -= d * width;
			if (!end) return result;
			root = child[d];
		}
		return root ? op(std::move(result), nodes[root].value) : result;
	}
	S suffix(root_type root, int level, std::uint64_t start) const {
		auto width = std::uint64_t(1) << (2 * level);
		S result = e();
		while (root && start) {
			width >>= 2;
			unsigned d = static_cast<unsigned>(start >> (2 * --level));
			const auto &child = nodes[root].child;
			if (d < 2) result = op(op(nodes[child[2]].value, nodes[child[3]].value), std::move(result));
			if (!(d & 1)) result = op(nodes[child[d + 1]].value, std::move(result));
			root = child[d];
			start -= d * width;
		}
		return root ? op(nodes[root].value, std::move(result)) : result;
	}
	root_type copy_impl(root_type root, root_type source, std::uint64_t l, std::uint64_t width, std::uint64_t ql, std::uint64_t qr) {
		if (root == source) return root;
		if (ql <= l && l + width <= qr) return source;
		width >>= 2;
		auto old = nodes[root].child, from = nodes[source].child, child = old;
		for (int i = 0; i < 4; ++i) {
			auto start = l + i * width;
			if (start < qr && ql < start + width) child[i] = copy_impl(old[i], from[i], start, width, ql, qr);
		}
		if (child == old) return root;
		if (child == from) return source;
		return join(child);
	}
	template <bool Right, class Pred>
	std::uint64_t search(root_type root, std::uint64_t l, std::uint64_t width, std::uint64_t end, S &value, Pred &f) const {
		if (!root) return Right ? capacity() : 0;
		if (Right ? end <= l : l + width <= end) {
			S next = Right ? op(value, nodes[root].value) : op(nodes[root].value, value);
			if (f(next)) { value = std::move(next); return Right ? capacity() : 0; }
			if (width == 1) return Right ? l : l + 1;
		}
		width >>= 2;
		for (int step = 0; step < 4; ++step) {
			int i = Right ? step : 3 - step;
			auto start = l + i * width;
			if (Right ? start + width <= end : end <= start) continue;
			auto found = search<Right>(nodes[root].child[i], start, width, end, value, f);
			if (Right ? found != capacity() : found != 0) return found;
		}
		return Right ? capacity() : 0;
	}

  public:
	explicit PersistentSegtree(int n = 0) { initialize(n); }
	explicit PersistentSegtree(const std::vector<S> &a) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		initialize(static_cast<int>(a.size()));
		if (n_) {
			std::size_t count = 0;
			for (auto width = capacity(); width; width >>= 2) count += (a.size() + width - 1) / width;
			reserve(count);
			initial_ = build(a, 0, capacity());
		}
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	root_type set(root_type root, int p, S value) { return update<false>(root, p, std::move(value)); }
	root_type multiply(root_type root, int p, S value) { return update<true>(root, p, std::move(value)); }
	S get(root_type root, int p) const {
		check(root); assert(0 <= p && p < n_);
		for (int level = height - 1; root && level >= 0; --level) root = nodes[root].child[(static_cast<unsigned>(p) >> (2 * level)) & 3];
		return nodes[root].value;
	}
	S prod(root_type root, int l, int r) const {
		check(root); assert(0 <= l && l <= r && r <= n_);
		if (l == r) return e();
		if (!l && r == n_) return nodes[root].value;
		auto first = static_cast<std::uint64_t>(l), last = static_cast<std::uint64_t>(r), width = capacity();
		int level = height;
		while (root) {
			if (!first && last == width) return nodes[root].value;
			width >>= 2;
			int shift = 2 * --level;
			unsigned a = static_cast<unsigned>(first >> shift), b = static_cast<unsigned>((last - 1) >> shift);
			if (a == b) { root = nodes[root].child[a]; first -= a * width; last -= a * width; continue; }
			const auto &child = nodes[root].child;
			S result = suffix(child[a], level, first - a * width);
			for (unsigned i = a + 1; i < b; ++i) result = op(std::move(result), nodes[child[i]].value);
			return op(std::move(result), prefix(child[b], level, last - b * width));
		}
		return e();
	}
	S all_prod(root_type root) const { check(root); return nodes[root].value; }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		check(root); check(source); assert(0 <= l && l <= r && r <= n_);
		if (l == r || root == source) return root;
		if (!l && r == n_) return source;
		return copy_impl(root, source, 0, capacity(), l, r);
	}
	template <auto f> int max_right(root_type root, int l) const { return max_right(root, l, f); }
	template <class Pred> int max_right(root_type root, int l, Pred f) const {
		check(root); assert(0 <= l && l <= n_ && f(e()));
		S value = e();
		auto result = l == n_ ? static_cast<std::uint64_t>(n_) : search<true>(root, 0, capacity(), l, value, f);
		return result < static_cast<unsigned>(n_) ? static_cast<int>(result) : n_;
	}
	template <auto f> int min_left(root_type root, int r) const { return min_left(root, r, f); }
	template <class Pred> int min_left(root_type root, int r, Pred f) const {
		check(root); assert(0 <= r && r <= n_ && f(e()));
		S value = e();
		return r ? static_cast<int>(search<false>(root, 0, capacity(), r, value, f)) : 0;
	}
};

#endif
