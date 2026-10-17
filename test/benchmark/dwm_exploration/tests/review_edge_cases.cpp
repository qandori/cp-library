#include "components/dynamic_bit_vector.hpp"
#include <cassert>
#include <numeric>
#include <tuple>
#include <vector>

using BV = DynamicBitVector<128, 4>;

void check(const BV &bits, const std::vector<bool> &a) {
	int ones = 0, zeros = 0;
	for (int p = 0; p < int(a.size()); ++p) {
		assert(bits.rank1(p) == ones);
		assert(bits[p] == a[p]);
		assert(bits.select(a[p], a[p] ? ones : zeros) == p);
		ones += a[p], zeros += !a[p];
	}
	assert(bits.size() == int(a.size()));
	assert(bits.rank1(int(a.size())) == ones && bits.count() == ones);
}

void move(BV &bits, std::vector<bool> &a, int from, int to, bool value) {
	int before = std::accumulate(a.begin(), a.begin() + from, 0);
	bool old = a[from];
	a.erase(a.begin() + from);
	int next = std::accumulate(a.begin(), a.begin() + to, 0);
	a.insert(a.begin() + to, value);
	assert(bits.move_rank(from, to, value) == std::tuple(before, old, next));
	check(bits, a);
}

int main() {
	for (bool forward : {false, true}) for (bool bit : {false, true}) {
		std::vector<bool> a(257, bit);
		a.front() = a.back() = !bit;
		BV bits(a);
		for (int i = 0; i < (forward ? 22 : 21); ++i) {
			int p = forward ? 1 : int(a.size()) - 2;
			bits.erase(p), a.erase(a.begin() + p);
		}
		for (int i = 0; i < (forward ? 43 : 42); ++i) {
			int p = forward ? int(a.size()) - 1 : 1;
			bits.insert(p, bit), a.insert(a.begin() + p, bit);
		}
		check(bits, a);
		int from = forward ? 10 : int(a.size()) - 10;
		int to = forward ? int(a.size()) - 10 : 10;
		move(bits, a, from, to, bit);
		move(bits, a, from, to, !bit);
		move(bits, a, to, from, bit);
	}
	for (bool value : {false, true}) {
		std::vector<bool> a(300);
		for (int i = 0; i < int(a.size()); ++i) a[i] = (i % 7) < 3;
		BV bits(a);
		for (auto [from, to] : {std::pair(63, 64), std::pair(64, 63), std::pair(288, 289),
			std::pair(289, 288), std::pair(225, 299), std::pair(299, 225)})
			move(bits, a, from, to, value);
	}
}
