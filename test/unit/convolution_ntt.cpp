#include "convolution/convolution_ntt.hpp"
#include "math/modint.hpp"
#include <cassert>
#include <limits>
#include <random>

template<std::uint32_t Mod, class T>
void check(int n, int m) {
	std::mt19937_64 random(Mod + n);
	std::vector<T> a(n), b(m);
	for (auto &v : a) v = T(random());
	for (auto &v : b) v = T(random());
	a[0] = std::numeric_limits<T>::lowest();
	b[0] = std::numeric_limits<T>::max();
	auto naive = [](const auto &x, const auto &y) {
		std::vector<std::uint32_t> result(x.size() + y.size() - 1);
		for (std::size_t i = 0; i < x.size(); ++i) for (std::size_t j = 0; j < y.size(); ++j) {
			__int128_t u = (__int128_t(x[i]) % Mod + Mod) % Mod;
			__int128_t v = (__int128_t(y[j]) % Mod + Mod) % Mod;
			result[i + j] = (result[i + j] + u * v) % Mod;
		}
		return result;
	};
	auto expected = naive(a, b);
	assert(convolution_ntt<Mod>(a, b) == expected);
	assert(convolution_ntt<Mod>(b, a) == expected);
	using M = StaticModint<Mod>;
	std::vector<M> x(a.begin(), a.end()), y(b.begin(), b.end());
	assert(convolution_ntt(x, y) == std::vector<M>(expected.begin(), expected.end()));
	auto square = naive(a, a);
	assert(convolution_ntt<Mod>(a, a) == square);
	assert(convolution_ntt(x, x) == std::vector<M>(square.begin(), square.end()));
}

int main() {
	assert(convolution_ntt<924844033>(std::vector<int>{}, std::vector<int>{-3}).empty());
	check<17, int>(8, 9);
	check<257, std::uint64_t>(127, 130);
	check<998244353, std::int64_t>(511, 513);
	check<924844033, std::int64_t>(512, 513);
	check<924844033, std::uint64_t>(257, 3001);
	check<1811939329, std::uint64_t>(513, 515);
	check<2013265921, int>(259, 517);
}
