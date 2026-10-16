#include "components/quad_bit_vector.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

int main() {
	std::mt19937 rng(121);
	for (int n : {0, 1, 2, 63, 64, 65, 127, 128, 129, 1000}) {
		std::vector<std::uint32_t> a(n);
		QuadBitVector bits(n);
		for (int shift = 0; shift <= 30; ++shift) {
			for (auto &x : a) x = rng();
			bits.build(a, shift);
			std::array<int, 4> counts{};
			for (int r = 0; r <= n; ++r) {
				assert(bits.ranks(r) == counts);
				int below = 0;
				for (int digit = 0; digit < 4; ++digit) {
					assert(bits.rank(r, digit) == counts[digit]);
					assert(bits.below(r, digit) == below);
					below += counts[digit];
				}
				if (r < n) ++counts[(a[r] >> shift) & 3];
			}
		}
	}
}
