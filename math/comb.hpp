#ifndef COMB_998244353_HPP
#define COMB_998244353_HPP

#include <cassert>

#include "mint.hpp"

#ifndef COMB_MAX
#define COMB_MAX 500000 + 2022
#endif
constexpr int COMB_INF = static_cast<int>(COMB_MAX);
static_assert(COMB_INF >= 0 && COMB_INF < mint::mod());
inline mint fact[COMB_INF + 1], finv[COMB_INF + 1];
inline void init_comb() {
	fact[0] = 1;
	for (int i = 1; i <= COMB_INF; ++i) fact[i] = i * fact[i - 1];
	finv[COMB_INF] = fact[COMB_INF].inv();
	for (int i = COMB_INF; i >= 1; --i) finv[i - 1] = i * finv[i];
}
inline mint comb(int n, int k) {
	if (n < 0 || k < 0 || k > n) return 0;
	return fact[n] * finv[k] * finv[n - k];
}

#endif
