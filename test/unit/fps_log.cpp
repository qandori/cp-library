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

#if defined(__x86_64__) || defined(__i386__)
static void check_transform() {
	using N = fps_detail::NTT;
	using L = fps_detail::LogTransform;
	std::mt19937 rng(20261018);
	for (int n : {0, 1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023})
		for (int first : {1, 17, static_cast<int>(N::mod) - std::max(1, n)}) {
			auto a = std::make_unique<unsigned[]>(n), b = std::make_unique<unsigned[]>(n);
			const int size = (n + 31) / 32 * 32;
			std::vector<unsigned> prefix(size + 2, 0xdeadbeef);
			for (int i = 0; i < n; ++i) a[i] = i % 3 ? rng() % N::twice : N::twice - 1;
			L::integrate(b.get(), a.get(), prefix.data() + 1, first, n);
			require(prefix.front() == 0xdeadbeef && prefix.back() == 0xdeadbeef);
			for (int i = 0; i < n; ++i) require(fps_mint::word(b[i]) * fps_mint(first + i) == fps_mint::word(a[i]));
		}
	for (int n : {64, 128, 256, 512, 1024, 2048, 4096}) {
		N plan(n / 16, false);
		L t{plan};
		std::vector<unsigned> a(n), b(n), c(n);
		for (int i = 0; i < n; ++i) a[i] = rng() % N::twice;
		b = c = a;
		t.forward(b.data(), n);
		t.forward(c.data(), n);
		t.inverse_half<false>(b.data(), n);
		t.inverse_half<true>(c.data(), n);
		for (int i = 0; i < n; ++i) require(fps_mint::word(i < n / 2 ? b[i] : c[i]) == fps_mint::word(a[i]) * (n / 8));
		b = a;
		t.forward(b.data(), n);
		t.inverse(b.data(), n);
		for (int i = 0; i < n; ++i) require(fps_mint::word(b[i]) == fps_mint::word(a[i]) * (n / 8));
		std::vector<std::vector<unsigned>> inputs(5, std::vector<unsigned>(n));
		for (auto &v : inputs) for (auto &x : v) x = rng() % N::mod;
		const auto saved = inputs;
		const unsigned *left[] = {inputs[1].data(), inputs[0].data()}, *right[] = {inputs[3].data(), inputs[4].data()};
		t.products(a.data(), left, right, 2, n);
		left[0] = inputs[2].data();
		left[1] = inputs[1].data();
		t.products(b.data(), left, right, 2, n);
		require(inputs == saved);
		t.toeplitz(inputs[4].data(), inputs[2].data(), inputs[0].data(), inputs[1].data(), inputs[3].data(), inputs[4].data(), n);
		for (int i = 0; i < n; ++i) {
			require(a[i] % N::mod == inputs[4][i] % N::mod);
			require(b[i] % N::mod == inputs[2][i] % N::mod);
		}
		for (int i : {0, 1, 3}) require(inputs[i] == saved[i]);
	}
}
#endif

int main() {
#if defined(__x86_64__) || defined(__i386__)
	if (fps_detail::NTT::vectorized()) check_transform();
#endif
	std::mt19937 rng(20261018);
	for (int n : {255, 256, 257, 258, 385, 386, 511, 512, 513, 514, 769, 770, 1023, 1024, 1025, 1026, 1537, 1538, 4097, 16386, 500000})
		for (int input : {1, 37, 63, 64, 65, n / 3, n + 17}) {
			fps a(input);
			for (auto &x : a) x = fps_mint::word(fps_detail::NTT::twice - 1 - rng() % 7);
			a[0] = 1;
			const fps saved = a;
			fps l = a.log(n), d = l.diff();
			require(l.size() == static_cast<unsigned>(n) && l[0] == fps_mint(0));
			fps actual = (a * d).pre(n - 1), expected = a.pre(n).diff();
			actual.resize(n - 1);
			expected.resize(n - 1);
			require(actual == expected);
			for (int i = 0; i < input; ++i) require(a[i].v == saved[i].v);
			if (n <= 1026) {
				fps naive(n);
				for (int i = 1; i < n; ++i) {
					fps_mint sum = i < input ? a[i] * fps_mint(i) : fps_mint(0);
					for (int j = 1; j < i && j < input; ++j) sum -= a[j] * naive[i - j] * fps_mint(i - j);
					naive[i] = sum / fps_mint(i);
				}
				require(l == naive);
			}
		}
}
