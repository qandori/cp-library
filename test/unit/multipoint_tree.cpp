#include "fps/multipoint_eval.hpp"
#include <cstdint>
#include <random>
#include <cassert>
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
void check(int n, int m, std::mt19937 &random, bool zero = false) {
	FormalPowerSeries<T> f(n);
	std::vector<T> xs(m);
	if (!zero) for (T &v : f) v = random();
	for (int j = 0; j < m; ++j)
		xs[j] = j % 4 == 0 ? 0 : j % 4 == 1 ? T::mod() - 1 : j % 4 == 2 ? 7 : random();
	auto values = multipoint_eval(f, xs);
	assert(values.size() == xs.size());
	for (int j = 0; j < m; ++j) {
		std::uint64_t value = 0;
		for (int i = n - 1; i >= 0; --i) value = (value * xs[j].val() + f[i].val()) % T::mod();
		assert(value == values[j].val());
	}
	if (n > 32 && m > 32) {
		int length = 8;
		while (length < std::max(n, m)) length *= 2;
		int old_size = length / 8, new_size = 1;
		std::vector<std::vector<T>> old_frequency, new_frequency;
		auto old_tree = fps_detail::ProductTree<T>::build(xs, old_size, 8, true, &old_frequency);
		auto new_tree = fps_detail::ProductTree<T>::build(xs, new_size, 8, true, &new_frequency);
		assert(old_tree[1] == new_tree[1]);
	}
}

template<class T>
void run() {
	std::mt19937 random(89341);
	for (int n : {0, 1, 7, 8, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023, 1024, 1025, 4096, 4097})
		for (int m : {0, 1, 7, 8, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023, 1024, 1025})
			check<T>(n, m, random);
	for (auto [n, m] : std::vector<std::pair<int, int>>{{32768, 33}, {131072, 129}, {33, 32768}, {1025, 1025}}) {
		check<T>(n, m, random);
		check<T>(n, m, random, true);
	}
}

int main() {
	run<fps_detail::Mint>();
	run<PlainMint>();
}
