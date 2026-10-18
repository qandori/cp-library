#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#include <cassert>
#include <random>

struct PlainMint {
	unsigned int v = 0;
	static constexpr int mod() { return 998244353; }
	PlainMint() = default;
	PlainMint(long long x) { x %= mod(); v = static_cast<unsigned int>(x < 0 ? x + mod() : x); }
	static PlainMint raw(unsigned int x) { PlainMint r; r.v = x; return r; }
	unsigned int val() const { return v; }
	PlainMint &operator+=(PlainMint x) { v += x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator-=(PlainMint x) { v += mod() - x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator*=(PlainMint x) { v = static_cast<unsigned long long>(v) * x.v % mod(); return *this; }
	PlainMint &operator/=(PlainMint x) { return *this *= x.inv(); }
	PlainMint operator-() const { return raw(v ? mod() - v : 0); }
	PlainMint pow(long long k) const {
		PlainMint a = *this, r = 1;
		for (; k; k >>= 1, a *= a) if (k & 1) r *= a;
		return r;
	}
	PlainMint inv() const { return pow(mod() - 2); }
	friend PlainMint operator+(PlainMint a, PlainMint b) { return a += b; }
	friend PlainMint operator-(PlainMint a, PlainMint b) { return a -= b; }
	friend PlainMint operator*(PlainMint a, PlainMint b) { return a *= b; }
	friend PlainMint operator/(PlainMint a, PlainMint b) { return a /= b; }
	friend bool operator==(PlainMint, PlainMint) = default;
};

template<class T>
FormalPowerSeries<T> naive_compose(const FormalPowerSeries<T> &f, const FormalPowerSeries<T> &g, int n) {
	FormalPowerSeries<T> result(n);
	for (int i = static_cast<int>(f.size()) - 1; i >= 0; --i) {
		FormalPowerSeries<T> next(n);
		for (int j = 0; j < n; ++j)
			for (int k = 0; k < static_cast<int>(g.size()) && j + k < n; ++k) next[j + k] += result[j] * g[k];
		if (n) next[0] += f[i];
		result = std::move(next);
	}
	return result;
}

template<class T>
void check() {
	using F = FormalPowerSeries<T>;
	std::mt19937 random(731);
	for (int n : {0, 1, 2, 3, 8, 31, 32, 63, 64, 127, 128, 257}) {
		F f(n), g(n);
		std::vector<T> xs(n);
		for (int i = 0; i < n; ++i) { f[i] = random(); g[i] = random(); xs[i] = i * 7; }
		if (n) g[0] = 0;
		if (n > 1 && g[1] == T(0)) g[1] = 1;
		assert(compose(f, g) == naive_compose(f, g, n));
		assert(compose(f, g, n + 2) == naive_compose(f, g, n + 2));
		auto ys = multipoint_eval(f, xs);
		for (int i = 0; i < n; ++i) assert(ys[i] == f.eval(xs[i]));
		assert(interpolate(xs, ys) == f);
		assert(multipoint_eval(f, std::vector<T>(37, T(2))) == std::vector<T>(37, f.eval(T(2))));
		T c = 17;
		auto shifted = taylor_shift(f, c);
		for (int x = 0; x <= n; ++x) assert(shifted.eval(T(x)) == f.eval(T(x) + c));
		assert(taylor_shift(shifted, -c) == f && taylor_shift(f, 0) == f);
		if (n) {
			F a = f;
			a[0] = 1;
			a.resize(n + 3);
			assert(a.log(n + 3).exp(n + 3) == a);
		}
		if (n > 1) {
			auto inverse = compositional_inverse(g);
			F identity(n); identity[1] = 1;
			assert(naive_compose(g, inverse, n) == identity);
			assert(naive_compose(inverse, g, n) == identity);
			g[1] = 3;
			for (int i = 2; i < n; ++i) g[i] = 0;
			assert(compositional_inverse(g)[1] == T(3).inv());
		}
		assert(compose(f, F{}, 5) == F({n ? f[0] : T(0), 0, 0, 0, 0}));
	}
	for (int n : {0, 1, 2, 4, 7, 8, 15, 16, 31, 32, 33, 65, 257, 1025}) {
		for (int m : {0, 1, 7, 8, 9, 15, 16, 31, 32, 33, 63, 65, 257}) {
			F f(n);
			std::vector<T> xs(m);
			for (T &x : f) x = static_cast<long long>(random()) - 2147483648LL;
			for (int i = 0; i < m; ++i) xs[i] = i % 3 ? T(random()) : T(0);
			auto values = multipoint_eval(f, xs);
			assert(values.size() == xs.size());
			for (int i = 0; i < m; ++i) assert(values[i] == f.eval(xs[i]));
			assert(multipoint_eval(F(n), xs) == std::vector<T>(m));
			assert(multipoint_eval(f, std::vector<T>(m, T(-1))) == std::vector<T>(m, f.eval(T(-1))));
		}
	}
	for (int n : {4, 7, 8, 32}) {
		for (int m : {4095, 4096, 4097}) {
			F f(n);
			std::vector<T> xs(m);
			for (T &x : f) x = random();
			for (T &x : xs) x = random();
			auto values = multipoint_eval(f, xs);
			for (int i = 0; i < m; ++i) assert(values[i] == f.eval(xs[i]));
		}
	}
	assert(compose(F{}, F{}, 0).empty());
	assert(compositional_inverse(F{}, 0).empty());
	assert(compositional_inverse(F{}, 1) == F(1));
	assert(multipoint_eval(F{}, std::vector<T>{1, 2}) == std::vector<T>({0, 0}));
	for (int trial = 0; trial < 120; ++trial) {
		int n = random() % 90, m = 1 + random() % 80;
		F p(n), q(m), expected(513);
		for (T &x : p) x = random();
		for (T &x : q) x = random();
		if (q[0] == T(0)) q[0] = 1;
		if (trial % 3 == 0) { p.resize(n + 3); q.resize(m + 3); }
		for (int i = 0; i < 513; ++i) {
			expected[i] = i < static_cast<int>(p.size()) ? p[i] : T(0);
			for (int j = 1; j <= i && j < static_cast<int>(q.size()); ++j) expected[i] -= q[j] * expected[i - j];
			expected[i] /= q[0];
		}
		for (int k : {0, 1, 31, 32, 63, 64, 129, 512}) assert(bostan_mori(p, q, k) == expected[k]);
	}
	for (int d : {0, 1, 2, 3, 8, 31, 32, 64, 127}) {
		std::vector<T> initial(d), c(d), sequence(513);
		for (int i = 0; i < d; ++i) initial[i] = sequence[i] = random(), c[i] = random();
		if (d) c.back() = 0;
		for (int i = d; i < 513; ++i)
			for (int j = 0; j < d; ++j) sequence[i] += c[j] * sequence[i - j - 1];
		for (int k : {0, 1, d, d + 1, 31, 32, 128, 512}) assert(linear_recurrence(initial, c, k) == sequence[k]);
	}
	assert(bostan_mori(F{}, F{1}, 1000000000000000000LL) == T(0));
	assert(linear_recurrence(std::vector<T>{3}, std::vector<T>{2}, 1000000000000000000LL) == T(3) * T(2).pow(1000000000000000000LL));
}

int main() {
	check<mint>();
	check<PlainMint>();
}
