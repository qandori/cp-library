#ifndef RECTANGLE_ADD_RECTANGLE_SUM_HPP
#define RECTANGLE_ADD_RECTANGLE_SUM_HPP

#include "../components/rectangle_sum_polynomial.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, class Coordinate = std::int64_t>
class RectangleAddRectangleSum {
	static_assert(std::is_integral_v<Coordinate> && !std::is_same_v<Coordinate, bool> && sizeof(Coordinate) <= 8);
	using Node = rectangle_sum_detail::Polynomial<T>;
	std::vector<std::pair<Coordinate, Coordinate>> points_;
	std::vector<Coordinate> xs_, ys_;
	std::vector<std::size_t> offsets_;
	std::vector<Node> tree_;
	bool built_ = false;
	void add_edge(Coordinate x, Coordinate down, Coordinate up, const T &value) {
		auto found = std::lower_bound(xs_.begin(), xs_.end(), x);
		assert(found != xs_.end() && *found == x);
		Node low = Node::corner(T(x), T(down), value), high = Node::corner(T(x), T(up), T{} - value), both = low;
		both += high;
		for (std::size_t i = std::size_t(found - xs_.begin()) + 1; i <= xs_.size(); i += i & -i) {
			auto begin = ys_.begin() + offsets_[i] + 1, end = ys_.begin() + offsets_[i + 1];
			auto lower = std::lower_bound(begin, end, down), upper = std::lower_bound(begin, end, up);
			assert(lower != end && *lower == down && upper != end && *upper == up);
			std::size_t a = lower - begin + 1, b = upper - begin + 1, size = end - begin;
			while (a != b && (a <= size || b <= size)) {
				if (a < b) { tree_[offsets_[i] + a] += low; a += a & -a; }
				else { tree_[offsets_[i] + b] += high; b += b & -b; }
			}
			for (; a <= size; a += a & -a) tree_[offsets_[i] + a] += both;
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
		for (const auto &[x, y] : points_)
			if (xs_.empty() || xs_.back() != x) xs_.push_back(x);
		std::sort(points_.begin(), points_.end(), [](const auto &a, const auto &b) { return a.second < b.second; });
		std::vector<std::vector<Coordinate>> rows(xs_.size() + 1);
		for (const auto &[x, y] : points_)
			for (std::size_t i = std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin() + 1; i <= xs_.size(); i += i & -i)
				if (rows[i].empty() || rows[i].back() != y) rows[i].push_back(y);
		offsets_.assign(xs_.size() + 2, 0);
		for (std::size_t i = 1; i <= xs_.size(); ++i) offsets_[i + 1] = offsets_[i] + rows[i].size() + 1;
		ys_.resize(offsets_.back());
		for (std::size_t i = 1; i <= xs_.size(); ++i)
			std::copy(rows[i].begin(), rows[i].end(), ys_.begin() + offsets_[i] + 1);
		tree_.assign(ys_.size(), Node{});
		built_ = true;
	}
	void add(Coordinate left, Coordinate down, Coordinate right, Coordinate up, const T &value) {
		assert(built_ && left <= right && down <= up);
		if (left == right || down == up) return;
		add_edge(left, down, up, value);
		add_edge(right, down, up, T{} - value);
	}
	T prefix_sum(Coordinate x, Coordinate y) const {
		assert(built_);
		return prefix(x, y).evaluate(T(x), T(y));
	}
	T sum(Coordinate left, Coordinate down, Coordinate right, Coordinate up) const {
		assert(built_ && left <= right && down <= up);
		if (left == right || down == up) return T{};
		return prefix_sum(right, up) - prefix_sum(left, up) - prefix_sum(right, down) + prefix_sum(left, down);
	}
	void clear_values() { assert(built_); std::fill(tree_.begin(), tree_.end(), Node{}); }
};

#endif
