#ifndef MOD_SQRT_BASELINE_HPP
#define MOD_SQRT_BASELINE_HPP

#include "../../components/montgomery.hpp"
#include "../../components/montgomery32.hpp"
#include "../../components/splitmix64.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>

namespace mod_sqrt_baseline_detail {
template <class M>
std::optional<std::uint64_t> sqrt(std::uint64_t a, const M &m) {
	using U = typename M::U;
	U p = m.mod, value = m.in(U(a)), root;
	if ((p & 3) == 3) {
		root = m.pow(value, (p >> 2) + 1);
		if (m.mul(root, root) != value) return std::nullopt;
	} else {
		int s = std::countr_zero(U(p - 1));
		U q = (p - 1) >> s;
		U t = m.pow(value, q >> 1);
		root = m.mul(value, t);
		U b = m.mul(root, t), v = b;
		int k = 0;
		while (v != m.one && k < s) v = m.mul(v, v), ++k;
		if (k == s) return std::nullopt;
		if (k) {
			std::array<U, std::numeric_limits<U>::digits> powers;
			SplitMix64 random(a ^ p ^ 0x9e3779b97f4a7c15ULL);
			for (;;) {
				U z = (p & 7) == 5 ? 2 : U(random() % (p - 2) + 2);
				powers[0] = m.pow(m.in(z), q);
				for (int i = 1; i < s; ++i) powers[i] = m.mul(powers[i - 1], powers[i - 1]);
				if (powers[s - 1] == p - m.one) break;
			}
			int limit = s;
			while (k) {
				root = m.mul(root, powers[s - k - 1]);
				b = m.mul(b, powers[s - k]);
				limit = k;
				v = b; k = 0;
				while (v != m.one && k < limit) v = m.mul(v, v), ++k;
				assert(k < limit);
			}
		}
	}
	std::uint64_t result = m.reduce(root);
	return std::min(result, m.mod - result);
}
}

inline std::optional<std::uint64_t> mod_sqrt_baseline(std::uint64_t a, std::uint64_t prime) {
	assert(prime >= 2 && (prime == 2 || (prime & 1)));
	if (prime == 2) return a & 1;
	a %= prime;
	if (a <= 1) return a;
	if (prime <= std::numeric_limits<std::uint32_t>::max())
		return mod_sqrt_baseline_detail::sqrt(a, Montgomery32(std::uint32_t(prime)));
	return mod_sqrt_baseline_detail::sqrt(a, Montgomery64(prime));
}

#endif
