#include "components/fft.hpp"
#include "components/ntt.hpp"
#include "components/ntt_mod.hpp"
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <random>
#include <vector>

template <std::uint32_t mod, std::uint32_t root>
void modular() {
	std::mt19937 rng(mod);
	for (int n = 1; n <= 128; n *= 2) {
		std::vector<std::uint32_t> a(n), expected(n);
		for (auto &x : a) x = rng() % mod;
		auto power = [](std::uint32_t a, std::uint32_t k) {
			std::uint32_t r = 1;
			for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
				if (k & 1) r = std::uint64_t(r) * a % mod;
			return r;
		};
		auto omega = power(root, (mod - 1) / n);
		for (int i = 0; i < n; ++i) {
			std::uint64_t w = 1, step = power(omega, i);
			for (int j = 0; j < n; ++j, w = w * step % mod) expected[i] = (expected[i] + w * a[j]) % mod;
		}
		auto b = a;
		ntt_mod<mod, root>(b);
		assert(b == expected);
		ntt_mod<mod, root>(b, true);
		assert(b == a);
		if constexpr (mod == 998244353) {
			std::vector<fps_mint> c(a.begin(), a.end());
			ntt(c);
			for (int i = 0; i < n; ++i) assert(c[i].val() == expected[i]);
			ntt(c, true);
			for (int i = 0; i < n; ++i) assert(c[i].val() == a[i]);
		}
	}
}
int main() {
	modular<998244353, 3>();
	modular<754974721, 11>();
	modular<1224736769, 3>();
	modular<2013265921, 31>();
	std::mt19937 rng(88);
	for (int n = 1; n <= 128; n *= 2) {
		std::vector<std::complex<double>> a(n), expected(n);
		for (auto &x : a) x = {double(int(rng() % 100) - 50), double(int(rng() % 100) - 50)};
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) expected[i] += a[j] * std::polar(1., 2 * std::numbers::pi * i * j / n);
		auto b = a;
		fft(b);
		for (int i = 0; i < n; ++i) assert(std::abs(b[i] - expected[i]) < 1e-7);
		fft(b, true);
		for (int i = 0; i < n; ++i) assert(std::abs(b[i] - a[i]) < 1e-9);
	}
	for (int n : {8, 1024, 32768}) {
		std::vector<fps_mint> a(n);
		for (auto &x : a) x = rng();
		auto original = a;
		ntt(a);
		ntt(a, true);
		assert(a == original);
	}
	std::vector<std::complex<double>> empty;
	fft(empty);
	fft(empty, true);
}
