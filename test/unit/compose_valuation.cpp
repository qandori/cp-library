#include "fps/compose.hpp"
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
void check_valuation() {
	using F = FormalPowerSeries<T>;
	std::mt19937 random(81237);
	for (int n : {0, 1, 2, 3, 8, 9, 16, 31, 32, 33, 63, 64, 65, 127, 129}) {
		for (int valuation : {1, 2, 3, 4, 7, 8, 9, 16, 32, 64, 128}) {
			if (valuation > n + 1) continue;
			for (int kind = 0; kind < 6; ++kind) {
				F f(n + 3), g(n + 3);
				for (auto &x : f) x = random();
				for (int i = valuation; i < static_cast<int>(g.size()); ++i) g[i] = random();
				if (kind == 1) g.resize(std::min<int>(g.size(), valuation + 1));
				if (kind == 2) {
					for (int i = 0; i < std::min<int>(f.size(), (n + valuation - 1) / valuation); ++i) f[i] = 0;
				}
				if (kind == 3) f.resize(std::min<int>(f.size(), 8));
				if (kind == 4) { f.resize(f.size() + 5); g.resize(g.size() + 5); }
				if (kind == 5) for (auto &x : f) x = T(-1);
				assert(compose(f, g, n) == naive_compose(f, g, n));
			}
		}
	}
}

int main() {
	check_valuation<mint>();
	check_valuation<PlainMint>();
}
