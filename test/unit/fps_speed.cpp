#include "fps/fps.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

static void require(bool value, int line = __builtin_LINE()) {
	if (!value) {
		std::fprintf(stderr, "line %d\n", line);
		std::abort();
	}
}

static fps inverse(const fps &a, int n) {
	fps g(n);
	g[0] = a[0].inv();
	for (int i = 1; i < n; ++i) {
		for (int j = 1; j <= std::min<int>(i, a.size() - 1); ++j) g[i] -= a[j] * g[i - j];
		g[i] *= g[0];
	}
	return g;
}

static fps exponential(const fps &a, int n) {
	fps g(n);
	g[0] = 1;
	for (int i = 1; i < n; ++i) {
		for (int j = 1; j <= std::min<int>(i, a.size() - 1); ++j) g[i] += fps_mint(j) * a[j] * g[i - j];
		g[i] /= fps_mint(i);
	}
	return g;
}

#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx2"))) static void check_leaf() {
	using NTT = fps_detail::NTT;
	auto &plan = fps_detail::ntt_plan(4);
	for (unsigned int x : {0u, NTT::twice - 1, 4 * NTT::mod - 1})
		for (unsigned int y : {0u, NTT::mod - 1}) {
			unsigned int a[64], b[64], ws[8], qs[8];
			std::fill_n(a, 64, x);
			std::fill_n(b, 64, y);
			for (int k = 0; k < 8; ++k) {
				ws[k] = k & 1 ? NTT::mod - plan.roots[k / 2] : plan.roots[k / 2];
				qs[k] = NTT::quotient(ws[k]);
			}
			plan.inverse_leaf_products8<8, true>(a, b, ws, qs);
			for (int k = 0; k < 8; ++k)
				for (int i = 0; i < 8; ++i) {
					fps_mint expected = fps_mint::word(x) * fps_mint::word(y) * (fps_mint(i + 1) + fps_mint::raw(ws[k]) * (7 - i));
					require(fps_mint::word(a[8 * k + i]) == expected);
				}
		}
}
#endif

int main() {
	using Ops = fps_detail::Operations<fps_mint>;
	for (int count : {1, 6, 7, 8, 13, 14, 31, 32, 63, 64})
		for (unsigned int a : {0u, 1u, fps_mint::mod() - 1})
			for (unsigned int b : {0u, 1u, fps_mint::mod(), 2 * fps_mint::mod() - 1}) {
				std::vector<std::pair<int, fps_mint>> terms;
				for (int j = 1; j <= count; ++j) terms.emplace_back(j, fps_mint::word(a));
				fps g(count, fps_mint::word(b));
				require(Ops::sparse_dot(terms, g, count) == fps_mint::word(a) * fps_mint::word(b) * count);
			}

#if defined(__x86_64__) || defined(__i386__)
	if (fps_detail::NTT::vectorized()) check_leaf();
#endif

	std::mt19937 rng(20261018);
	for (int n : {65, 96, 97, 127, 128, 129, 160, 161, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 4095, 4096, 4097, 65537, 32768}) {
		for (int input : {37, n / 3, n}) {
			fps a(input);
			for (auto &x : a) x = rng();
			a[0] = 17;
			fps g = a.inv(n);
			require(g.size() == static_cast<unsigned>(n));
			if (n <= 1025) require(g == inverse(a, n));
			else {
				fps p = (a * g).pre(n);
				require(p[0] == fps_mint(1));
				for (int i = 1; i < n; ++i) require(p[i] == fps_mint(0));
			}
		}
	}
	for (int terms : {0, 1, 2, 6, 7, 8, 14, 31, 32, 33, 63, 64, 65})
		for (int stride : {1, 2, 7, 64}) {
			fps a(terms * stride + 1);
			a[0] = 17;
			for (int i = 1; i <= terms; ++i) a[i * stride] = fps_mint::word(2 * fps_mint::mod() - 1 - rng() % 3);
			for (int n : {1, 127, 129, 257, 513}) {
				a[0] = 17;
				require(a.inv(n) == inverse(a, n));
				a[0] = 0;
				require(a.exp(n) == exponential(a, n));
			}
		}
	for (int stride : {2, 7, 1024}) {
		fps a(32 * stride + 1);
		a[0] = 1;
		for (int i = 1; i <= 32; ++i) a[i * stride] = rng();
		const int n = 65537;
		fps inv = a.inv(n), p = (a * inv).pre(n);
		require(p[0] == fps_mint(1));
		for (int i = 1; i < n; ++i) require(p[i] == fps_mint(0));
		a[0] = 0;
		fps e = a.exp(n);
		for (int i = 1; i < n; ++i) if (i % stride) require(e[i] == fps_mint(0));
		require(e.log(n) == a.pre(n) + fps(n));
	}
	for (int stride : {1, 7}) {
		fps a(stride + 1);
		a[stride] = 17;
		fps e = a.exp(1025);
		a[stride] *= fps_mint(123456789);
		require(e.pow(123456789) == a.exp(1025));
	}

}
