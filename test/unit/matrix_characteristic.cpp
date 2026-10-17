#include "math/matrix.hpp"
#include "math/mint.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>
template <std::uint32_t P>
struct Mod {
	std::uint32_t x = 0;
	Mod() = default;
	Mod(long long v) : x((v % P + P) % P) {}
	static constexpr std::uint32_t mod() { return P; }
	static Mod raw(std::uint32_t v) { Mod a; a.x = v; return a; }
	std::uint32_t val() const { return x; }
	Mod operator-() const { return raw(x ? P - x : 0); }
	Mod &operator+=(Mod b) { x += b.x; if (x >= P) x -= P; return *this; }
	Mod &operator-=(Mod b) { x = x >= b.x ? x - b.x : x + P - b.x; return *this; }
	Mod &operator*=(Mod b) { x = std::uint64_t(x) * b.x % P; return *this; }
	Mod &operator/=(Mod b) { Mod p = 1; for (auto k = P - 2; k; k >>= 1, b *= b) if (k & 1) p *= b; return *this *= p; }
	friend Mod operator+(Mod a, Mod b) { return a += b; }
	friend Mod operator-(Mod a, Mod b) { return a -= b; }
	friend Mod operator*(Mod a, Mod b) { return a *= b; }
	friend Mod operator/(Mod a, Mod b) { return a /= b; }
	bool operator==(const Mod &) const = default;
};

template <class T>
struct Generic {
	T v;
	Generic(long long x = 0) : v(x) {}
	Generic(T x) : v(x) {}
	Generic operator-() const { return Generic(-v); }
	Generic &operator+=(Generic b) { v += b.v; return *this; }
	Generic &operator-=(Generic b) { v -= b.v; return *this; }
	Generic &operator*=(Generic b) { v *= b.v; return *this; }
	Generic &operator/=(Generic b) { v /= b.v; return *this; }
	friend Generic operator+(Generic a, Generic b) { return a += b; }
	friend Generic operator-(Generic a, Generic b) { return a -= b; }
	friend Generic operator*(Generic a, Generic b) { return a *= b; }
	friend Generic operator/(Generic a, Generic b) { return a /= b; }
	bool operator==(const Generic &) const = default;
};

template <class T>
void check() {
	std::mt19937_64 rng(914771);
	for (int n : {0, 1, 2, 3, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65}) for (int kind = 0; kind < 9; ++kind) {
		Matrix<T> a(n);
		Matrix<Generic<T>> b(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			T x = T(rng() % T::mod());
			if (kind == 1) x = T(0);
			if (kind == 2) x = T(T::mod() - 1);
			if (kind == 3 && i > j) x = T(0);
			if (kind == 4 && i > j + 1) x = T(0);
			if (kind == 5 && rng() % 8) x = T(0);
			if (kind == 6) x = T(i == (j + 1) % n);
			if (kind == 7) x = T((i + 1) * (j + 2));
			if (kind == 8 && j == 0 && i < n - 1) x = T(0);
			a(i, j) = x;
			b(i, j) = Generic<T>(x);
		}
		auto original = a;
		auto p = a.characteristic_polynomial();
		assert(a == original);
		auto q = b.characteristic_polynomial();
		assert(p.size() == q.size());
		for (int i = 0; i <= n; ++i) assert(p[i] == q[i].v);
		for (int x : {0, 1, 3}) {
			T value = 0;
			for (int i = n; i >= 0; --i) value = value * T(x) + p[i];
			assert(value == (Matrix<T>::identity(n) * T(x) - a).det());
		}
	}
	for (int n : {0, 1, 3, 4, 5, 15, 16, 17, 63, 64, 65, 511}) {
		std::vector<std::uint32_t> a(n, T::mod() - 1);
		assert(matrix_detail::ModularField<T>::dot(a.data(), a.data(), n) == std::uint32_t(n) % T::mod());
	}
}

template <std::uint32_t P>
void dot_boundary() {
	for (int n : {0, 1, 3, 4, 5, 15, 16, 17, 31, 32, 33, 63, 64, 65, 511}) {
		std::vector<std::uint32_t> a(n, P - 1);
		assert(matrix_detail::ModularField<Mod<P>>::dot(a.data(), a.data(), n) == std::uint32_t(n) % P);
	}
}

int main() {
	dot_boundary<(1u << 30)>();
	dot_boundary<(1u << 30) + 1>();
	check<mint>();
	check<Mod<2>>();
	check<Mod<3>>();
	check<Mod<1000000007>>();
	check<Mod<1073741789>>();
	check<Mod<2147483647>>();
}
