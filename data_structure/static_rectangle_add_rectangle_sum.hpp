#ifndef STATIC_RECTANGLE_ADD_RECTANGLE_SUM_HPP
#define STATIC_RECTANGLE_ADD_RECTANGLE_SUM_HPP

#include "../components/fenwick_sum.hpp"
#include "../components/radix_sort.hpp"
#include "../components/rectangle_sum_polynomial.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

template <class T = std::int64_t, class Coordinate = std::int64_t>
class StaticRectangleAddRectangleSum {
	static_assert(std::is_integral_v<Coordinate> && !std::is_same_v<Coordinate, bool> && sizeof(Coordinate) <= 8);
	using Node = rectangle_sum_detail::Polynomial<T>;
	struct Rectangle { Coordinate left, down, right, up; };
	struct Update { Rectangle rectangle; T value; };
	std::vector<Update> updates_;
	std::vector<Rectangle> queries_;
	void check_count() const { assert(updates_.size() + queries_.size() < std::size_t(std::numeric_limits<int>::max() / 2)); }

  public:
	void add_rectangle(Coordinate left, Coordinate down, Coordinate right, Coordinate up, const T &value) {
		assert(left <= right && down <= up);
		if (left == right || down == up) return;
		check_count();
		updates_.push_back({{left, down, right, up}, value});
	}
	int add_query(Coordinate left, Coordinate down, Coordinate right, Coordinate up) {
		assert(left <= right && down <= up);
		check_count();
		queries_.push_back({left, down, right, up});
		return static_cast<int>(queries_.size()) - 1;
	}
	std::vector<T> solve() const {
		std::vector<T> answers(queries_.size());
		if (updates_.empty() || queries_.empty()) return answers;
		if (std::min(updates_.size(), queries_.size()) <= 4 || updates_.size() <= 4096 / queries_.size()) {
			for (std::size_t i = 0; i < queries_.size(); ++i)
				for (const auto &update : updates_) {
					const auto &a = queries_[i], &b = update.rectangle;
					Coordinate left = std::max(a.left, b.left), right = std::min(a.right, b.right);
					Coordinate down = std::max(a.down, b.down), up = std::min(a.up, b.up);
					if (left < right && down < up) answers[i] += update.value * (T(right) - T(left)) * (T(up) - T(down));
				}
			return answers;
		}
		int n = static_cast<int>(updates_.size()), count = n + static_cast<int>(queries_.size());
		auto rectangle = [&](int id) -> const Rectangle & { return id < n ? updates_[id].rectangle : queries_[id - n]; };
		struct Endpoint { Coordinate y; int id; };
		struct Event { Coordinate x; T down, up, weight; int low, high, id; };
		std::vector<int> ranks(2 * std::size_t(count));
		std::vector<Event> events;
		int size = 0;
		{
			std::vector<Endpoint> endpoints;
			endpoints.reserve(2 * std::size_t(count));
			for (int i = 0; i < count; ++i) {
				const auto &r = rectangle(i);
				endpoints.push_back({r.down, 2 * i}); endpoints.push_back({r.up, 2 * i + 1});
			}
			radix_sort_detail::sort_by_key(endpoints, [](const Endpoint &e) { return e.y; });
			for (std::size_t first = 0; first < endpoints.size();) {
				std::size_t last = first;
				bool update = false;
				while (last < endpoints.size() && endpoints[last].y == endpoints[first].y) {
					ranks[endpoints[last].id] = size;
					update = update || endpoints[last].id < 2 * n;
					++last;
				}
				if (update) ++size;
				first = last;
			}
		}
		events.reserve(2 * std::size_t(count));
		for (int i = 0; i < count; ++i) {
			const auto &r = rectangle(i);
			if (r.left == r.right || r.down == r.up) continue;
			T weight = i < n ? updates_[i].value : T{};
			int id = i < n ? 0 : i - n + 1;
			events.push_back({r.left, T(r.down), T(r.up), weight, ranks[2 * i], ranks[2 * i + 1], -id});
			events.push_back({r.right, T(r.down), T(r.up), T{} - weight, ranks[2 * i], ranks[2 * i + 1], id});
		}
		radix_sort_detail::sort_by_key(events, [](const Event &e) { return e.x; });
		FenwickSum<Node> tree(size);
		for (const auto &event : events) {
			T x(event.x);
			if (!event.id) {
				tree.add_pair(event.low, Node::corner(x, event.down, event.weight), event.high, Node::corner(x, event.up, T{} - event.weight));
			} else {
				T value = tree.prefix(event.high).evaluate(x, event.up) - tree.prefix(event.low).evaluate(x, event.down);
				if (event.id < 0) answers[-event.id - 1] -= value;
				else answers[event.id - 1] += value;
			}
		}
		return answers;
	}
};

#endif
