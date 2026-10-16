#include "components/dynamic_bit_vector.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

template <class BV>
void check(const BV &bits, const std::vector<unsigned char> &a) {
	assert(bits.size() == int(a.size()) && bits.empty() == a.empty());
	std::vector<int> prefix(a.size() + 1);
	for (int i = 0; i < int(a.size()); ++i) prefix[i + 1] = prefix[i] + a[i];
	assert(bits.count() == prefix.back());
	for (int i = 0; i <= int(a.size()); ++i) {
		assert(bits.rank1(i) == prefix[i]);
		assert(bits.rank0(i) == i - prefix[i]);
		if (i == int(a.size())) continue;
		assert(bits[i] == bool(a[i]));
		assert(bits.access_rank(i) == std::pair(prefix[i], bool(a[i])));
		if (i % 17 == 0) assert(bits.select(a[i], a[i] ? prefix[i] : i - prefix[i]) == i);
	}
	assert(bits.select0(-1) == -1 && bits.select1(-1) == -1);
	assert(bits.select1(prefix.back()) == -1);
	assert(bits.select0(int(a.size()) - prefix.back()) == -1);
	std::mt19937 random(987);
	for (int i = 0; i < 200; ++i) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		assert(bits.rank1(l, r) == std::pair(prefix[l], prefix[r]));
	}
}

template <int Block, int Branch>
void random_test() {
	using BV = DynamicBitVector<Block, Branch>;
	std::mt19937 random(Block * 37 + Branch);
	std::vector<unsigned char> a(3000);
	for (auto &x : a) x = random() & 1;
	BV bits(std::vector<bool>(a.begin(), a.end()));
	check(bits, a);
	for (int i = 0; i < 6000; ++i) {
		int op = random() % 7, p = random() % (a.size() + 1);
		if (!a.empty() && p == int(a.size()) && op > 1) --p;
		bool value = random() & 1;
		int before = std::accumulate(a.begin(), a.begin() + p, 0);
		if (op <= 1 || a.empty()) {
			assert(bits.insert(p, value) == before);
			a.insert(a.begin() + p, value);
		} else if (op <= 3) {
			assert(bits.erase_rank(p) == std::pair(before, bool(a[p])));
			a.erase(a.begin() + p);
		} else if (op == 4) {
			assert(bits.set_rank(p, value) == std::pair(before, bool(a[p])));
			a[p] = value;
		} else {
			int to = op == 5 ? random() % a.size() : (i & 1) ? 0
															 : int(a.size()) - 1;
			bool old = a[p];
			a.erase(a.begin() + p);
			int next = std::accumulate(a.begin(), a.begin() + to, 0);
			a.insert(a.begin() + to, value);
			assert(bits.move_rank(p, to, value) == std::tuple(before, old, next));
		}
		if (i % 311 == 0) check(bits, a);
	}
	check(bits, a);
	BV copy = bits;
	check(copy, a);
	BV moved = std::move(copy);
	assert(copy.empty());
	copy.push_back(true);
	assert(copy.pop_back() && copy.empty());
	check(moved, a);
	copy = std::move(moved);
	assert(moved.empty());
	check(copy, a);
	bits.clear();
	assert(bits.empty() && bits.rank1(0) == 0 && bits.select0(0) == -1);
	for (int size : {0, 1, 63, 64, 65, Block - 1, Block, Block + 1, Block * Branch + 1}) {
		std::vector<std::uint64_t> packed((size + 63) / 64, ~std::uint64_t(0));
		bits.build(size, packed);
		std::vector<unsigned char> reference(size, 1);
		check(bits, reference);
		for (int i = 0; i < size; ++i) bits.set(i, false);
		std::fill(reference.begin(), reference.end(), 0);
		check(bits, reference);
	}
}

int main() {
	for (int size = 1; size <= 8; ++size)
		for (unsigned mask = 0; mask < (1U << size); ++mask) {
			std::vector<bool> a(size);
			for (int i = 0; i < size; ++i) a[i] = (mask >> i) & 1;
			DynamicBitVector<128, 4> base(a);
			for (int from = 0; from < size; ++from)
				for (int to = 0; to < size; ++to)
					for (bool value : {false, true}) {
						auto bits = base;
						std::vector<unsigned char> b(a.begin(), a.end());
						int before = std::accumulate(b.begin(), b.begin() + from, 0);
						bool old = b[from];
						b.erase(b.begin() + from);
						int next = std::accumulate(b.begin(), b.begin() + to, 0);
						b.insert(b.begin() + to, value);
						assert(bits.move_rank(from, to, value) == std::tuple(before, old, next));
						assert(bits.size() == size && bits.count() == std::accumulate(b.begin(), b.end(), 0));
						for (int i = 0; i < size; ++i) assert(bits[i] == bool(b[i]));
					}
		}
	random_test<128, 4>();
	random_test<256, 8>();
	random_test<512, 8>();
	random_test<1024, 8>();
	random_test<1024, 16>();
	std::mt19937 random(713);
	DynamicBitVector<128, 4> bits;
	bits.reserve(50000);
	auto memory = bits.memory_usage();
	for (int pass = 0; pass < 3; ++pass) {
		std::vector<unsigned char> a;
		for (int i = 0; i < 40000; ++i) {
			int p = pass == 0 ? i : pass == 1 ? 0
											  : random() % (i + 1);
			bool value = random() & 1;
			bits.insert(p, value);
			a.insert(a.begin() + p, value);
		}
		check(bits, a);
		for (int i = 40000; i; --i) {
			int p = pass == 0 ? 0 : pass == 1 ? i - 1
											  : random() % i;
			assert(bits.erase(p) == bool(a[p]));
			a.erase(a.begin() + p);
			if (i % 8192 == 0) check(bits, a);
		}
		assert(bits.empty() && bits.count() == 0);
		assert(bits.memory_usage() == memory);
	}
}
