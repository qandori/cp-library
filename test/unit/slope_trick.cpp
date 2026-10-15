#include "data_structure/slope_trick.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using I = std::int64_t;
constexpr int bound = 1024;
constexpr I inf = I(1) << 50;
struct Brute {
	std::vector<I> a = std::vector<I>(2 * bound + 1);
	void hinge(int x, bool left) {
		for (int i = -bound; i <= bound; ++i) a[i + bound] += std::max(0, left ? x - i : i - x);
	}
	void shift(int l, int r) {
		auto old = a;
		std::fill(a.begin(), a.end(), inf);
		for (int i = -bound; i <= bound; ++i)
			for (int j = l; j <= r; ++j)
				if (-bound <= i - j && i - j <= bound) a[i + bound] = std::min(a[i + bound], old[i - j + bound]);
	}
	void clear_left() {
		for (int i = int(a.size()) - 2; i >= 0; --i) a[i] = std::min(a[i], a[i + 1]);
	}
	void clear_right() {
		for (std::size_t i = 1; i < a.size(); ++i) a[i] = std::min(a[i], a[i - 1]);
	}
};
void check(const SlopeTrick<I> &f, const Brute &b) {
	I best = inf;
	int first = 0, last = 0;
	for (int x = -256; x <= 256; ++x) {
		I value = b.a[x + bound];
		if (value < best) best = value, first = last = x;
		else if (value == best) last = x;
	}
	assert(f.min() == best);
	auto [l, r] = f.argmin();
	assert(l == (first == -256 ? std::nullopt : std::optional<I>(first)));
	assert(r == (last == 256 ? std::nullopt : std::optional<I>(last)));
	for (int x = -250; x <= 250; x += 25) assert(f.eval(x) == b.a[x + bound]);
}
int main() {
	std::mt19937 rng(613);
	for (int rep = 0; rep < 400; ++rep) {
		SlopeTrick<I> f;
		f.reserve(100);
		Brute brute;
		for (int step = 0; step < 80; ++step) {
			int x = int(rng() % 101) - 50;
			switch (rng() % 10) {
			case 0:
				f.add_a_minus_x(x);
				brute.hinge(x, true);
				break;
			case 1:
				f.add_x_minus_a(x);
				brute.hinge(x, false);
				break;
			case 2:
			case 3:
				f.add_abs(x);
				brute.hinge(x, true);
				brute.hinge(x, false);
				break;
			case 4:
				f.add_all(x);
				for (auto &v : brute.a) v += x;
				break;
			case 5: {
				int l = int(rng() % 5) - 2, r = int(rng() % 5) - 2;
				if (l > r) std::swap(l, r);
				f.shift(l, r);
				brute.shift(l, r);
				break;
			}
			case 6:
				f.shift(x / 25);
				brute.shift(x / 25, x / 25);
				break;
			case 7:
				f.clear_left();
				brute.clear_left();
				break;
			case 8:
				f.clear_right();
				brute.clear_right();
				break;
			default: {
				SlopeTrick<I> other;
				Brute other_brute;
				for (int j = 0; j < 7; ++j) {
					int a = int(rng() % 101) - 50;
					other.add_abs(a);
					other_brute.hinge(a, true);
					other_brute.hinge(a, false);
				}
				other.shift(-1, 2);
				other_brute.shift(-1, 2);
				f.merge(other);
				for (std::size_t i = 0; i < brute.a.size(); ++i) brute.a[i] += other_brute.a[i];
				assert(other.empty() && other.min() == 0 && other.eval(19) == 0);
			}
			}
			check(f, brute);
		}
		auto copy = f;
		auto moved = std::move(copy);
		check(moved, brute);
		assert(copy.empty() && copy.eval(0) == 0);
		copy = std::move(moved);
		check(copy, brute);
		assert(moved.empty() && moved.min() == 0);
		copy.merge(copy);
		for (auto &x : brute.a) x *= 2;
		check(copy, brute);
		copy.clear();
		assert(copy.empty() && copy.min() == 0 && !copy.argmin().first && !copy.argmin().second);
	}
	SlopeTrick<I, __int128_t> wide;
	wide.add_abs(std::numeric_limits<I>::min());
	wide.add_abs(std::numeric_limits<I>::max());
	assert(wide.min() == (__int128_t(1) << 64) - 1);
	assert(wide.eval(0) == wide.min());
	assert(wide.argmin().first == std::numeric_limits<I>::min() && wide.argmin().second == std::numeric_limits<I>::max());
	SlopeTrick<double> real;
	real.add_abs(1.25);
	real.shift(-.5, .25);
	real.add_x_minus_a(.5);
	assert(std::abs(real.min() - .25) < 1e-12);
	assert(std::abs(real.eval(2.) - 2.) < 1e-12);
	SlopeTrick<> constant;
	constant.add_all(-17);
	SlopeTrick<> point;
	point.add_abs(100);
	constant.merge(std::move(point));
	assert(constant.min() == -17 && constant.eval(102) == -15 && point.empty());
}
