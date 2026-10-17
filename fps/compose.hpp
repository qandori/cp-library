#ifndef CP_FPS_COMPOSE_HPP
#define CP_FPS_COMPOSE_HPP

#include "fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> compose(const FormalPowerSeries<T> &outer, const FormalPowerSeries<T> &inner, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(outer.size());
	assert(deg >= 0 && (inner.empty() || inner[0] == T(0)));
	if (deg == 0) return {};
	F f = outer.pre(deg), g = inner.pre(deg);
	O::trim(f);
	O::trim(g);
	if (f.size() <= 1 || g.size() <= 1) {
		F result(deg);
		if (!f.empty()) result[0] = f[0];
		return result;
	}
	int first = 1;
	while (g[first] == T(0)) ++first;
	if (first + 1 == static_cast<int>(g.size())) {
		F result(deg);
		T power = 1;
		for (int i = 0; i < static_cast<int>(f.size()) && (long long)i * first < deg; ++i)
			result[i * first] = f[i] * power, power *= g[first];
		return result;
	}
	if (first > 1) {
		int terms = (deg - 1) / first + 1;
		if (f.size() > std::size_t(terms)) { f.resize(terms); O::trim(f); }
	}
	if (deg <= 32 || f.size() <= 8) {
		F result;
		for (int i = (static_cast<int>(f.size())) - 1; i >= 0; --i) {
			result = F::multiply(result, g).pre(deg);
			result += f[i];
		}
		result.resize(deg);
		return result;
	}
	int size = 1;
	while (size < deg) size <<= 1;
	assert(size <= (1 << 22));
	fps_detail::NTT plan(size);
	std::vector<unsigned int> p(2 * size), q(size);
	for (int i = 0; i < static_cast<int>(f.size()); ++i) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) p[i] = f[i].v;
		else p[i] = plan.in(f[i].val());
	}
	for (int i = 0; i < static_cast<int>(g.size()); ++i) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) q[i] = g[i].v;
		else q[i] = plan.in(g[i].val());
	}
	unsigned int a = plan.one, b = plan.one;
	plan.compose(p.data(), q.data(), 1, size, a, b);
	F result(deg);
	for (int i = 0; i < deg; ++i) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) result[i] = T::word(p[i]);
		else result[i] = T::raw(plan.out(p[i]));
	}
	return result;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
