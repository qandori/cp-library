#include "../../components/small_vector.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <sstream>
#include <vector>

int main() {
	static_assert(sizeof(SmallVector<std::uint64_t>) == 3 * sizeof(std::uint64_t));
	std::mt19937 rng(5678);
	SmallVector<int> a;
	std::vector<int> b;
	for (int trial = 0; trial < 20000; ++trial) {
		int value = rng() % 1000, count = rng() % 20, pos = rng() % (b.size() + 1);
		switch (rng() % 10) {
			case 0: a.push_back(value); b.push_back(value); break;
			case 1: if (!b.empty()) a.pop_back(), b.pop_back(); break;
			case 2: a.resize(count); b.resize(count); break;
			case 3: a.reserve(count); b.reserve(count); break;
			case 4: a.insert(a.begin() + pos, count, value); b.insert(b.begin() + pos, count, value); break;
			case 5: {
				int end = pos + rng() % (b.size() - pos + 1);
				a.erase(a.begin() + pos, a.begin() + end);
				b.erase(b.begin() + pos, b.begin() + end);
				break;
			}
			case 6: a.assign(count, value); b.assign(count, value); break;
			case 7: {
				auto c = a;
				a = std::move(c);
				assert(c.empty());
				c.push_back(17);
				assert(c[0] == 17);
				break;
			}
			case 8: {
				SmallVector<int> c(std::move(a));
				assert(a.empty());
				a.assign(2, 7);
				a = c;
				break;
			}
			case 9: {
				std::vector<int> c(count, value);
				a.insert(a.begin() + pos, c.begin(), c.end());
				b.insert(b.begin() + pos, c.begin(), c.end());
				break;
			}
		}
		assert(a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin()));
		assert(a.capacity() >= a.size());
	}
	a.resize_for_overwrite(100);
	std::fill(a.begin(), a.end(), 23);
	assert(a.size() == 100 && a.back() == 23);
	std::istringstream input("1 2 3 4 5");
	SmallVector<int, 1> c{std::istream_iterator<int>(input), std::istream_iterator<int>()};
	assert(c.size() == 5 && c.back() == 5);
	SmallVector<int, 16> d{1, 2, 3};
	auto e = std::move(d);
	assert(d.empty() && e.size() == 3 && e.back() == 3);
}
