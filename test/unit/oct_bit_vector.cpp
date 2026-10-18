#include "components/oct_bit_vector.hpp"
#include <algorithm>
#include <cstdlib>
#include <random>

void require(bool ok) {
	if (!ok) std::abort();
}

void check(const OctBitVector &bits, const std::vector<std::uint32_t> &a, int shift) {
	for (int r = 0; r <= int(a.size()); ++r) {
		std::array<int, 8> counts{};
		for (int i = 0; i < r; ++i) ++counts[(a[i] >> shift) & 7];
		require(bits.ranks(r) == counts);
		int below = 0;
		for (int digit = 0; digit < 8; ++digit) {
			require(bits.rank(r, digit) == counts[digit]);
			require(bits.below(r, digit) == below);
			below += counts[digit];
		}
	}
}

int main() {
	std::mt19937 random(20261019);
	for (int n : {0, 1, 2, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257})
		for (int shift : {0, 3, 6, 15, 27, 30}) {
			std::vector<std::uint32_t> a(n);
			for (auto &x : a) x = random();
			OctBitVector bits(n), scalar(n);
			bits.build(a, shift);
			scalar.build_scalar(a, shift);
			check(bits, a, shift);
			check(scalar, a, shift);
			for (int repeat = 0; repeat < 100 && n; ++repeat) {
				int l = random() % n, r = random() % n;
				if (l > r) std::swap(l, r);
				++r;
				int k = random() % (r - l);
				std::vector<int> sorted;
				for (int i = l; i < r; ++i) sorted.push_back((a[i] >> shift) & 7);
				std::sort(sorted.begin(), sorted.end());
				auto step = bits.route_scalar(l, r, k);
				require(step.digit == sorted[k]);
				int count = 0, before = 0, below = 0;
				for (int i = 0; i < r; ++i) {
					int digit = (a[i] >> shift) & 7;
					count += digit == step.digit;
					if (i < l) before += digit == step.digit;
					else below += digit < step.digit;
				}
				require(step.a == before && step.b == count && step.k == k - below);
			}
		}
}
