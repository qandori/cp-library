#ifndef DYNAMIC_SEGTREE_HPP
#define DYNAMIC_SEGTREE_HPP

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class Coord = std::int64_t>
class DynamicSegtree {
	static_assert(std::is_integral_v<Coord> && !std::is_same_v<Coord, bool> && sizeof(Coord) <= 8);
	using U = std::make_unsigned_t<Coord>;
	static constexpr std::uint32_t mask = (std::uint32_t(1) << 26) - 1;
	static constexpr U sign = std::is_signed_v<Coord> ? U(1) << (std::numeric_limits<U>::digits - 1) : U(0);
	struct Node {
		U key;
		S value, product;
		std::uint32_t left = 0, right = 0;
		Node(U key, S value) : key(key), value(std::move(value)), product(this->value) {}
	};
	std::vector<Node> nodes;
	std::uint32_t root = 0;
	Coord first, last;
	static U encode(Coord x) { return U(x) ^ sign; }
	static Coord decode(U x) { return Coord(x ^ sign); }
	std::uint32_t left(std::uint32_t k) const { return nodes[k].left & mask; }
	void pull(std::uint32_t k) {
		nodes[k].product = op(op(nodes[left(k)].product, nodes[k].value), nodes[nodes[k].right].product);
	}
	std::uint32_t make(U p, S value) {
		assert(nodes.size() <= mask);
		auto k = std::uint32_t(nodes.size());
		nodes.emplace_back(p, std::move(value));
		return k;
	}
	template <bool append>
	std::uint32_t update(std::uint32_t k, U p, S value) {
		if (!k) return make(p, std::move(value));
		std::array<std::uint32_t, 64> path;
		unsigned depth = 0;
		auto result = k;
		bool combine = append;
		for (;;) {
			if (p == nodes[k].key) {
				if (combine) nodes[k].value = op(nodes[k].value, value);
				else nodes[k].value = std::move(value);
				pull(k);
				break;
			}
			unsigned h = nodes[k].left >> 26;
			U difference = p ^ nodes[k].key;
			if (!(left(k) | nodes[k].right)) {
				h = std::bit_width(difference) - 1;
				nodes[k].left = h << 26;
			} else if ((difference >> h) > 1) {
				h = std::bit_width(difference) - 1;
				auto parent = make(p, std::move(value));
				nodes[parent].left = h << 26;
				if (nodes[k].key < p) nodes[parent].left |= k;
				else nodes[parent].right = k;
				pull(parent);
				if (!depth) result = parent;
				else {
					auto prev = path[depth - 1];
					if (left(prev) == k) nodes[prev].left = (nodes[prev].left & ~mask) | parent;
					else nodes[prev].right = parent;
				}
				break;
			}
			bool right = (p >> h) & 1;
			if (right ? p < nodes[k].key : nodes[k].key < p) {
				std::swap(p, nodes[k].key);
				std::swap(value, nodes[k].value);
				combine = false;
			}
			assert(depth < path.size());
			path[depth++] = k;
			auto c = right ? nodes[k].right : left(k);
			if (!c) {
				c = make(p, std::move(value));
				if (right) nodes[k].right = c;
				else nodes[k].left = (nodes[k].left & ~mask) | c;
				break;
			}
			k = c;
		}
		while (depth) pull(path[--depth]);
		return result;
	}
	S suffix(std::uint32_t k, U l) const {
		S result = e();
		while (k) {
			if (nodes[k].key < l) k = nodes[k].right;
			else {
				S part = op(nodes[k].value, nodes[nodes[k].right].product);
				result = op(part, result);
				k = left(k);
			}
		}
		return result;
	}
	S prefix(std::uint32_t k, U r) const {
		S result = e();
		while (k) {
			if (r <= nodes[k].key) k = left(k);
			else {
				S part = op(nodes[left(k)].product, nodes[k].value);
				result = op(result, part);
				k = nodes[k].right;
			}
		}
		return result;
	}
	template <class F>
	bool search_right(std::uint32_t k, U l, S &value, F &f, Coord &answer) const {
		if (!k) return false;
		if (nodes[k].key < l) return search_right(nodes[k].right, l, value, f, answer);
		if (search_right(left(k), l, value, f, answer)) return true;
		S next = op(value, nodes[k].value);
		if (!f(next)) {
			answer = decode(nodes[k].key);
			return true;
		}
		value = std::move(next);
		k = nodes[k].right;
		if (!k) return false;
		next = op(value, nodes[k].product);
		if (f(next)) {
			value = std::move(next);
			return false;
		}
		return search_right(k, l, value, f, answer);
	}
	template <class F>
	bool search_left(std::uint32_t k, U r, S &value, F &f, Coord &answer) const {
		if (!k) return false;
		if (r <= nodes[k].key) return search_left(left(k), r, value, f, answer);
		if (search_left(nodes[k].right, r, value, f, answer)) return true;
		S next = op(nodes[k].value, value);
		if (!f(next)) {
			answer = decode(nodes[k].key) + 1;
			return true;
		}
		value = std::move(next);
		k = left(k);
		if (!k) return false;
		next = op(nodes[k].product, value);
		if (f(next)) {
			value = std::move(next);
			return false;
		}
		return search_left(k, r, value, f, answer);
	}

  public:
	using value_type = S;
	using coordinate_type = Coord;
	explicit DynamicSegtree(Coord n = 0) : DynamicSegtree(0, n) {}
	DynamicSegtree(Coord l, Coord r) : first(l), last(r) {
		assert(l <= r);
		nodes.emplace_back(U(0), e());
	}
	DynamicSegtree(const DynamicSegtree &) = default;
	DynamicSegtree &operator=(const DynamicSegtree &) = default;
	DynamicSegtree(DynamicSegtree &&other) : DynamicSegtree(other.first, other.last) { swap(other); }
	DynamicSegtree &operator=(DynamicSegtree &&other) {
		if (this != &other) {
			DynamicSegtree moved(std::move(other));
			swap(moved);
		}
		return *this;
	}
	void swap(DynamicSegtree &other) noexcept {
		nodes.swap(other.nodes);
		std::swap(root, other.root);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void reserve(std::size_t n) {
		assert(n <= mask);
		nodes.reserve(n + 1);
	}
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	U size() const { return U(last) - U(first); }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	void clear() {
		nodes.erase(nodes.begin() + 1, nodes.end());
		root = 0;
	}
	void set(Coord p, S value) {
		assert(first <= p && p < last);
		root = update<false>(root, encode(p), std::move(value));
	}
	void multiply(Coord p, S value) {
		assert(first <= p && p < last);
		root = update<true>(root, encode(p), std::move(value));
	}
	S get(Coord p) const {
		assert(first <= p && p < last);
		U key = encode(p);
		auto k = root;
		while (k && nodes[k].key != key) k = key < nodes[k].key ? left(k) : nodes[k].right;
		return k ? nodes[k].value : e();
	}
	S prod(Coord l, Coord r) const {
		assert(first <= l && l <= r && r <= last);
		if (l == r) return e();
		if (l == first && r == last) return all_prod();
		U a = encode(l), b = encode(r);
		auto k = root;
		while (k) {
			if (nodes[k].key < a) k = nodes[k].right;
			else if (b <= nodes[k].key) k = left(k);
			else {
				S result = nodes[k].value;
				if (auto c = left(k)) result = op(suffix(c, a), result);
				if (auto c = nodes[k].right) result = op(result, prefix(c, b));
				return result;
			}
		}
		return e();
	}
	S all_prod() const { return root ? nodes[root].product : e(); }
	template <class F>
	Coord max_right(Coord l, F f) const {
		assert(first <= l && l <= last && f(e()));
		if (l == last) return last;
		S value = e();
		Coord answer = last;
		search_right(root, encode(l), value, f, answer);
		return answer;
	}
	template <class F>
	Coord min_left(Coord r, F f) const {
		assert(first <= r && r <= last && f(e()));
		if (r == first) return first;
		S value = e();
		Coord answer = first;
		search_left(root, encode(r), value, f, answer);
		return answer;
	}
};

#endif
