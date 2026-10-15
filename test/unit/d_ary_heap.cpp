#include "data_structure/d_ary_heap.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <random>
#include <set>
#include <vector>

template <std::size_t D, class Compare>
void check(Compare compare) {
	DaryHeap<int, Compare, D> heap(compare);
	std::multiset<int, Compare> ref(compare);
	std::mt19937 rng(73 + D);
	for (int step = 0; step < 100000; ++step) {
		int value = int(rng() % 10000) - 5000;
		switch (rng() % 5) {
		case 0:
		case 1:
			heap.push(value);
			ref.insert(value);
			break;
		case 2:
			if (!ref.empty()) {
				int top = *ref.rbegin();
				assert(heap.replace_top(value) == top);
				ref.erase(std::prev(ref.end()));
				ref.insert(value);
			}
			break;
		case 3: {
			ref.insert(value);
			int top = *ref.rbegin();
			ref.erase(std::prev(ref.end()));
			assert(heap.push_pop(value) == top);
			break;
		}
		default:
			if (!ref.empty()) {
				assert(heap.pop_value() == *ref.rbegin());
				ref.erase(std::prev(ref.end()));
			}
		}
		assert(heap.size() == ref.size() && heap.empty() == ref.empty());
		if (!ref.empty()) assert(heap.top() == *ref.rbegin());
	}
	auto copy = heap;
	while (!ref.empty()) {
		assert(copy.pop_value() == *ref.rbegin());
		ref.erase(std::prev(ref.end()));
	}
	heap.clear();
	assert(heap.empty());
	for (int n = 0; n < 200; ++n) {
		std::vector<int> values(n);
		for (auto &x : values) x = rng() % 100;
		auto expected = values;
		std::sort(expected.begin(), expected.end(), compare);
		DaryHeap<int, Compare, D> built(std::move(values), compare);
		for (auto it = expected.rbegin(); it != expected.rend(); ++it) assert(built.pop_value() == *it);
		assert(built.empty());
	}
}
int main() {
	DaryHeap<bool> bits(std::vector<bool>{false, true, false, true});
	assert(bits.pop_value());
	assert(bits.pop_value());
	assert(!bits.pop_value());
	assert(!bits.pop_value());
	DaryHeap<int, std::less<int>, std::size_t(-1)> flat(std::vector<int>{1, 7, 2, 5});
	for (int x : {7, 5, 2, 1}) assert(flat.pop_value() == x);

	check<2>(std::less<int>{});
	check<4>(std::less<int>{});
	check<8>(std::less<int>{});
	check<3>(std::greater<int>{});
	check<4>(std::greater<int>{});
	check<16>(std::greater<int>{});
	auto compare = [](const std::unique_ptr<int> &a, const std::unique_ptr<int> &b) { return *a < *b; };
	DaryHeap<std::unique_ptr<int>, decltype(compare)> heap(compare);
	for (int i = 0; i < 100; ++i) heap.push(std::make_unique<int>(i));
	assert(*heap.replace_top(std::make_unique<int>(7)) == 99);
	assert(*heap.push_pop(std::make_unique<int>(1000)) == 1000);
	for (int i = 98; i >= 8; --i) assert(*heap.pop_value() == i);
	assert(*heap.pop_value() == 7);
	assert(*heap.pop_value() == 7);
	auto moved = std::move(heap);
	assert(heap.empty());
	assert(*moved.top() == 6);
	moved.pop();
	assert(*moved.top() == 5);
	heap.swap(moved);
	assert(moved.empty() && *heap.top() == 5);
}
