#ifndef SQRT_FLOOR_HPP
#define SQRT_FLOOR_HPP

#include "../template.hpp"

inline tuple<long, long, vec<tuple<long, long, long>>> sqrt_floor(long n, long m, long a, long b) {
	if (n == 0) return {0, 0, {}};
	a %= m, b %= m;
	const long D = sqrtl(n);
	long dx = 1, best = m;
	for (long i = 1; i <= D; i++) {
		const long v = a * i % m;
		if (chmin(best, min(v, m - v))) dx = i;
	}
	long dy = a * dx % m;
	const bool flip = dy > m - dy;
	if (flip) {
		a = (m - a) % m;
		b = m - 1 - b;
		dy = m - dy;
	}
	vec<tuple<long, long, long>> ans;
	for (long di = 0; di < dx; di++) {
		const long y = (b + a * di) % m;
		const long cnt = (n - 1 - di) / dx + 1;
		if (dy == 0) {
			ans.emplace_back(di, y, cnt);
			continue;
		}
		const long lim = (dy * (cnt - 1) + y) / m;
		long le = 0;
		for (long k = 0; k <= lim; k++) {
			const long ri = k == lim ? cnt : (m * (k + 1) - y + dy - 1) / dy;
			ans.emplace_back(dx * le + di, (dy * le + y) % m, ri - le);
			le = ri;
		}
	}
	if (flip) {
		dy = -dy;
		for (auto &[x, y, len] : ans) y = m - 1 - y;
	}
	return {dx, dy, ans};
}

#endif
