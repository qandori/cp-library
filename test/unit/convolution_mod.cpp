#include "convolution/convolution_mod.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

template<class T>
std::vector<std::uint32_t> naive(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t mod) {
	if (a.empty() || b.empty()) return {};
	std::vector<std::uint32_t> result(a.size() + b.size() - 1);
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) {
			__int128 x = (__int128(a[i]) % mod + mod) % mod;
			__int128 y = (__int128(b[j]) % mod + mod) % mod;
			result[i + j] = (result[i + j] + x * y) % mod;
		}
	return result;
}

template<class T>
void check() {
	std::mt19937_64 random(897);
	for (std::uint32_t mod : {1u, 2u, 17u, 998244353u, 1000000007u, 4294967295u})
		for (int n : {0, 1, 2, 7, 8, 16, 17, 31, 32, 33, 127, 128, 129, 257}) {
			std::vector<T> a(n);
			for (auto &value : a) value = T(random());
			if (n) a[0] = std::numeric_limits<T>::max();
			if (n > 1) a[1] = std::numeric_limits<T>::lowest();
			auto b = a;
			auto expected = naive(a, a, mod);
			assert(convolution_mod(a, a, mod) == expected);
			assert(convolution_mod(a, b, mod) == expected);
			assert(a == b);
			b.resize(n / 2);
			assert(convolution_mod(a, b, mod) == naive(a, b, mod));
			assert(convolution_mod(b, a, mod) == naive(b, a, mod));
		}
}

int main() {
	check<std::int64_t>();
	check<std::uint64_t>();
	check<std::int32_t>();
	check<std::uint32_t>();
}
