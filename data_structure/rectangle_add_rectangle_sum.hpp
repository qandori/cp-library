#ifndef RECTANGLE_ADD_RECTANGLE_SUM_HPP
#define RECTANGLE_ADD_RECTANGLE_SUM_HPP

#include "../components/rectangle_sum_polynomial.hpp"
#include <algorithm>
#include <cassert>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, class Coordinate = std::int64_t>
class RectangleAddRectangleSum {
	static_assert(std::is_integral_v<Coordinate> && !std::is_same_v<Coordinate, bool> && sizeof(Coordinate) <= 8);
	using Value = typename rectangle_sum_detail::Storage<T>::type;
	using Node = rectangle_sum_detail::Polynomial<Value>;
	std::vector<std::pair<Coordinate, Coordinate>> points_;
	std::vector<Coordinate> xs_, ys_;
	std::vector<std::size_t> offsets_;
	std::vector<std::uint32_t> parents_;
	std::vector<Node> tree_;
	bool built_ = false;
	void add_edge(Coordinate x, Coordinate down, Coordinate up, const Value &value) {
		auto found = std::lower_bound(xs_.begin(), xs_.end(), x);
		assert(found != xs_.end() && *found == x);
		Node low = Node::corner(Value(x), Value(down), value), high = Node::corner(Value(x), Value(up), Value{} - value), both = low;
		both += high;
		std::size_t i = std::size_t(found - xs_.begin()) + 1;
		auto begin = ys_.begin() + offsets_[i] + 1, end = ys_.begin() + offsets_[i + 1];
		auto lower = std::lower_bound(begin, end, down), upper = std::lower_bound(begin, end, up);
		assert(lower != end && *lower == down && upper != end && *upper == up);
		std::size_t a = lower - begin + 1, b = upper - begin + 1;
		for (; i <= xs_.size(); i += i & -i) {
			std::size_t next_a = parents_[offsets_[i] + a], next_b = parents_[offsets_[i] + b];
			std::size_t size = offsets_[i + 1] - offsets_[i] - 1;
			while (a != b && (a <= size || b <= size)) {
				if (a < b) { tree_[offsets_[i] + a] += low; a += a & -a; }
				else { tree_[offsets_[i] + b] += high; b += b & -b; }
			}
			for (; a <= size; a += a & -a) tree_[offsets_[i] + a] += both;
			a = next_a; b = next_b;
		}
	}
	Node prefix(Coordinate x, Coordinate y) const {
		Node result;
		for (std::size_t i = std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin(); i; i &= i - 1) {
			auto begin = ys_.begin() + offsets_[i] + 1, end = ys_.begin() + offsets_[i + 1];
			for (std::size_t j = std::lower_bound(begin, end, y) - begin; j; j &= j - 1)
				result += tree_[offsets_[i] + j];
		}
		return result;
	}

	Value strip(std::size_t i, std::size_t stop, const Value &x, Coordinate down, Coordinate up) const {
		Node low, high;
		Value weight{}, offset{};
		for (; i != stop; i &= i - 1) {
			auto begin = ys_.begin() + offsets_[i] + 1, end = ys_.begin() + offsets_[i + 1];
			std::size_t a = std::lower_bound(begin, end, down) - begin, b = std::lower_bound(begin, end, up) - begin;
			while (a != b) {
				if (a < b) { high += tree_[offsets_[i] + b]; b &= b - 1; }
				else { low += tree_[offsets_[i] + a]; a &= a - 1; }
			}
			for (; a; a &= a - 1) {
				const auto &node = tree_[offsets_[i] + a];
				weight += node.weight; offset += node.x;
			}
		}
		Value lower(down), upper(up);
		return high.evaluate(x, upper) - low.evaluate(x, lower) + (weight * x - offset) * (upper - lower);
	}
	Value common_strip(std::size_t i, Coordinate down, Coordinate up) const {
		Value low_weight{}, low_y{}, high_weight{}, high_y{}, common_weight{};
		for (; i; i &= i - 1) {
			auto begin = ys_.begin() + offsets_[i] + 1, end = ys_.begin() + offsets_[i + 1];
			std::size_t a = std::lower_bound(begin, end, down) - begin, b = std::lower_bound(begin, end, up) - begin;
			while (a != b) {
				if (a < b) {
					const auto &node = tree_[offsets_[i] + b];
					high_weight += node.weight; high_y += node.y; b &= b - 1;
				} else {
					const auto &node = tree_[offsets_[i] + a];
					low_weight += node.weight; low_y += node.y; a &= a - 1;
				}
			}
			for (; a; a &= a - 1) common_weight += tree_[offsets_[i] + a].weight;
		}
		Value lower(down), upper(up);
		return high_weight * upper - high_y - low_weight * lower + low_y + common_weight * (upper - lower);
	}

  public:
	void reserve(Coordinate left, Coordinate down, Coordinate right, Coordinate up) {
		assert(!built_ && left <= right && down <= up);
		if (left == right || down == up) return;
		assert(points_.size() <= std::size_t(std::numeric_limits<int>::max()) - 4);
		points_.emplace_back(left, down); points_.emplace_back(left, up);
		points_.emplace_back(right, down); points_.emplace_back(right, up);
	}
	void build() {
		std::sort(points_.begin(), points_.end());
		points_.erase(std::unique(points_.begin(), points_.end()), points_.end());
		xs_.clear();
		{
			struct Point { Coordinate y; std::uint32_t rank; };
			std::vector<Point> ordered;
			ordered.reserve(points_.size());
			for (const auto &[x, y] : points_) {
				if (xs_.empty() || xs_.back() != x) xs_.push_back(x);
				ordered.push_back({y, static_cast<std::uint32_t>(xs_.size())});
			}
			std::sort(ordered.begin(), ordered.end(), [](const Point &a, const Point &b) { return a.y < b.y; });
			std::vector<std::uint32_t> count(xs_.size() + 1);
			std::vector<Coordinate> last(xs_.size() + 1);
			auto visit = [&](auto write) {
				for (const auto &[y, rank] : ordered)
					for (std::size_t i = rank; i <= xs_.size(); i += i & -i)
						if (!count[i] || last[i] != y) {
							++count[i]; last[i] = y;
							write(i, y);
						}
			};
			visit([](std::size_t, Coordinate) {});
			offsets_.assign(xs_.size() + 2, 0);
			for (std::size_t i = 1; i <= xs_.size(); ++i) offsets_[i + 1] = offsets_[i] + count[i] + 1;
			ys_.resize(offsets_.back());
			parents_.resize(offsets_.back());
			std::fill(count.begin(), count.end(), 0);
			visit([&](std::size_t i, Coordinate y) {
				std::size_t parent = i + (i & -i), position = offsets_[i] + count[i];
				ys_[position] = y;
				parents_[position] = parent <= xs_.size() ? count[parent] + (!count[parent] || last[parent] != y) : 0;
			});
		}
		tree_.assign(ys_.size(), Node{});
		built_ = true;
	}
	void add(Coordinate left, Coordinate down, Coordinate right, Coordinate up, const T &value) {
		assert(built_ && left <= right && down <= up);
		if (left == right || down == up) return;
		add_edge(left, down, up, Value(value));
		add_edge(right, down, up, Value{} - Value(value));
	}
	void add_rectangle(Coordinate left, Coordinate down, Coordinate right, Coordinate up, const T &value) {
		add(left, down, right, up, value);
	}
	T prefix_sum(Coordinate x, Coordinate y) const {
		assert(built_);
		return T(prefix(x, y).evaluate(Value(x), Value(y)));
	}
	T sum(Coordinate left, Coordinate down, Coordinate right, Coordinate up) const {
		assert(built_ && left <= right && down <= up);
		if (left == right || down == up) return T{};
		if constexpr (!(std::is_unsigned_v<Value> || std::is_same_v<Value, __uint128_t> || requires { Value::mod(); }))
			return prefix_sum(right, up) - prefix_sum(left, up) - prefix_sum(right, down) + prefix_sum(left, down);
		std::size_t a = std::lower_bound(xs_.begin(), xs_.end(), left) - xs_.begin();
		std::size_t b = std::lower_bound(xs_.begin(), xs_.end(), right) - xs_.begin();
		std::size_t common = a & (~std::size_t(0) << std::bit_width(a ^ b));
		Value lower(left), upper(right);
		return T(strip(b, common, upper, down, up) - strip(a, common, lower, down, up)
			+ (upper - lower) * common_strip(common, down, up));
	}
	void clear_values() { assert(built_); std::fill(tree_.begin(), tree_.end(), Node{}); }
};

#endif
