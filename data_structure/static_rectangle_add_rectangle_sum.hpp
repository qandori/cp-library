#ifndef STATIC_RECTANGLE_ADD_RECTANGLE_SUM_HPP
#define STATIC_RECTANGLE_ADD_RECTANGLE_SUM_HPP

#include "../components/radix_sort.hpp"
#include "../components/rectangle_sum_polynomial.hpp"
#include <algorithm>
#include <bit>
#include <utility>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

template <class T = std::int64_t, class Coordinate = std::int64_t>
class StaticRectangleAddRectangleSum {
	static_assert(std::is_integral_v<Coordinate> && !std::is_same_v<Coordinate, bool> && sizeof(Coordinate) <= 8);
	using Value = typename rectangle_sum_detail::Storage<T>::type;
	using Node = rectangle_sum_detail::Polynomial<Value>;
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
		std::vector<Value> answers(queries_.size());
		auto result = [&]() -> std::vector<T> {
			if constexpr (std::is_same_v<T, Value>) return std::move(answers);
			else return std::vector<T>(answers.begin(), answers.end());
		};
		if (updates_.empty() || queries_.empty()) return result();
		if (std::min(updates_.size(), queries_.size()) <= 4 || updates_.size() <= 4096 / queries_.size()) {
			for (std::size_t i = 0; i < queries_.size(); ++i)
				for (const auto &update : updates_) {
					const auto &a = queries_[i], &b = update.rectangle;
					Coordinate left = std::max(a.left, b.left), right = std::min(a.right, b.right);
					Coordinate down = std::max(a.down, b.down), up = std::min(a.up, b.up);
					if (left < right && down < up) answers[i] += Value(update.value) * (Value(right) - Value(left)) * (Value(up) - Value(down));
				}
			return result();
		}
		int n = static_cast<int>(updates_.size()), count = n + static_cast<int>(queries_.size());
		auto rectangle = [&](int id) -> const Rectangle & { return id < n ? updates_[id].rectangle : queries_[id - n]; };
		struct Endpoint { Coordinate coordinate; int id; };
		struct Event { Value x, down, up, weight; int low, high, id; };
		std::vector<Event> events;
		int size = 0;
		{
			std::vector<int> ranks(2 * std::size_t(count));
			std::vector<Endpoint> endpoints;
			endpoints.reserve(2 * std::size_t(count));
			for (int i = 0; i < count; ++i) {
				const auto &r = rectangle(i);
				if (r.left == r.right || r.down == r.up) continue;
				endpoints.push_back({r.down, 2 * i}); endpoints.push_back({r.up, 2 * i + 1});
			}
			radix_sort_detail::sort_by_key<11>(endpoints, [](const Endpoint &e) { return e.coordinate; });
			for (std::size_t first = 0; first < endpoints.size();) {
				std::size_t last = first;
				bool update = false;
				while (last < endpoints.size() && endpoints[last].coordinate == endpoints[first].coordinate) {
					ranks[endpoints[last].id] = size;
					update = update || endpoints[last].id < 2 * n;
					++last;
				}
				if (update) ++size;
				first = last;
			}
			endpoints.clear();
			for (int i = 0; i < count; ++i) {
				const auto &r = rectangle(i);
				if (r.left == r.right || r.down == r.up) continue;
				endpoints.push_back({r.left, 2 * i}); endpoints.push_back({r.right, 2 * i + 1});
			}
			radix_sort_detail::sort_by_key<11>(endpoints, [](const Endpoint &e) { return e.coordinate; });
			while (!endpoints.empty() && endpoints.back().id < 2 * n) endpoints.pop_back();
			events.reserve(endpoints.size());
			for (const auto &endpoint : endpoints) {
				int i = endpoint.id / 2;
				bool right = endpoint.id & 1;
				const auto &r = rectangle(i);
				Value weight = i < n ? Value(updates_[i].value) : Value{};
				int id = i < n ? 0 : i - n + 1;
				events.push_back({Value(endpoint.coordinate), Value(r.down), Value(r.up), right ? Value{} - weight : weight,
					ranks[2 * i], ranks[2 * i + 1], right ? id : -id});
			}
		}
		std::vector<Node> tree(size);
		for (const auto &event : events) {
			const Value &x = event.x;
			if (!event.id) {
				Node low = Node::corner(x, event.down, event.weight), high = Node::corner(x, event.up, Value{} - event.weight), both = low;
				both += high;
				unsigned a = event.low + 1, b = event.high + 1;
				while (a != b && (a <= unsigned(size) || b <= unsigned(size))) {
					if (a < b) { tree[a - 1] += low; a += a & -a; }
					else { tree[b - 1] += high; b += b & -b; }
				}
				for (; a <= unsigned(size); a += a & -a) tree[a - 1] += both;
			} else {
				Node low, high;
				unsigned a = event.low, b = event.high;
				if constexpr (std::is_unsigned_v<Value> || std::is_same_v<Value, __uint128_t> || requires { Value::mod(); }) {
					unsigned common = a & (std::numeric_limits<unsigned>::max() << std::bit_width(a ^ b));
					for (; a != common; a &= a - 1) low += tree[a - 1];
					for (; b != common; b &= b - 1) high += tree[b - 1];
					for (; common; common &= common - 1) {
						const auto &node = tree[common - 1];
						low.weight += node.weight; high.weight += node.weight;
						low.x += node.x; high.x += node.x;
					}
				} else {
					for (; a; a &= a - 1) low += tree[a - 1];
					for (; b; b &= b - 1) high += tree[b - 1];
				}
				Value value = high.evaluate(x, event.up) - low.evaluate(x, event.down);
				if (event.id < 0) answers[-event.id - 1] -= value;
				else answers[event.id - 1] += value;
			}
		}
		return result();
	}
};

#endif
