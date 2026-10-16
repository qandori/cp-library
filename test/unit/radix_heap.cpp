#include "components/RadixHeap.hpp"
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <random>
#include <type_traits>
#include <vector>

template <class K>
void check(K minimum) {
	RadixHeap<K> heap(minimum);
	std::priority_queue<K, std::vector<K>, std::greater<K>> ref;
	std::mt19937_64 rng(71);
	K last = minimum;
	for (int step = 0; step < 100000; ++step) {
		if (ref.empty() || rng() % 3) {
			K key = last + K(rng() % 10000);
			heap.push({key, step});
			ref.push(key);
		} else {
			assert(heap.top().first == ref.top());
			last = ref.top();
			assert(heap.last() == last);
			if (step & 1) assert(heap.pop_value().first == last);
			else heap.pop();
			ref.pop();
		}
		assert(heap.size() == ref.size());
	}
	while (!ref.empty()) {
		assert(heap.pop_value().first == ref.top());
		ref.pop();
	}
	assert(heap.empty());
	heap.clear(minimum);
	heap.push({minimum, 1});
	heap.pop();
	assert(heap.empty());
	heap.clear();
	for (int bit = 0; bit < std::numeric_limits<K>::digits; ++bit) heap.push({K(K(1) << bit), bit});
	if constexpr (std::is_unsigned_v<K>) heap.push({std::numeric_limits<K>::max(), 99});
	K previous = 0;
	while (!heap.empty()) {
		auto s = heap.pop_value();
		assert(s.first >= previous);
		previous = s.first;
	}
}
int main() {
	check<std::uint32_t>(0);
	check<std::uint64_t>(0);
	check<std::int64_t>(-100000);
	RadixHeap<std::int64_t> signed_heap(std::numeric_limits<std::int64_t>::min());
	for (auto key : {std::numeric_limits<std::int64_t>::max(), std::int64_t(0), std::int64_t(-1), std::numeric_limits<std::int64_t>::min()}) signed_heap.push({key, 0});
	assert(signed_heap.pop_value().first == std::numeric_limits<std::int64_t>::min());
	assert(signed_heap.pop_value().first == -1);
	assert(signed_heap.pop_value().first == 0);
	assert(signed_heap.pop_value().first == std::numeric_limits<std::int64_t>::max());
	RadixHeap<std::uint64_t, std::unique_ptr<int>> heap;
	heap.push({7, std::make_unique<int>(42)});
	heap.push({9, std::make_unique<int>(17)});
	auto moved = std::move(heap);
	assert(heap.empty());
	heap = std::move(moved);
	assert(moved.empty());
	assert(*heap.pop_value().second == 42);
	assert(*heap.pop_value().second == 17);
}
