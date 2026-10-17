#include "math/matrix.hpp"
#include "math/mint.hpp"
#ifdef MATRIX_BASELINE_HEADER
#undef MATRIX_HPP
#undef MATRIX_ARITHMETIC_HPP
#define Matrix BaselineMatrix
#define LinearSolution BaselineLinearSolution
#define matrix_detail baseline_matrix_detail
#include MATRIX_BASELINE_HEADER
#undef matrix_detail
#undef LinearSolution
#undef Matrix
#endif
#include <chrono>
#include <ctime>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
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
	std::uint32_t val() const { return v.val(); }
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


template <class T, template <class> class M>
void run(int n, const std::string &kind, int repetitions, std::uint64_t seed) {
	std::mt19937_64 rng(seed);
	M<T> a(n);
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		T x = T(rng() % 998244353);
		if (kind == "zero") x = T(0);
		if (kind == "max") x = T(-1);
		if (kind == "triangular" && i > j) x = T(0);
		if (kind == "hessenberg" && i > j + 1) x = T(0);
		if (kind == "sparse" && rng() % 16) x = T(0);
		if (kind == "cycle") x = T(i == (j + 1) % n);
		a(i, j) = x;
	}
	std::uint64_t checksum = 0;
	auto wall_begin = std::chrono::steady_clock::now();
	auto cpu_begin = std::clock();
	for (int r = 0; r < repetitions; ++r) {
		auto p = a.characteristic_polynomial();
		for (auto x : p) checksum = checksum * 1000000007 + x.val();
	}
	double cpu = double(std::clock() - cpu_begin) / CLOCKS_PER_SEC;
	double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - wall_begin).count();
	std::cout.precision(12);
	std::cout << cpu << ' ' << wall << ' ' << checksum << '\n';
}

template <class T>
void pair(int n, const std::string &kind, int repetitions, std::uint64_t seed, int trial) {
#ifdef MATRIX_BASELINE_HEADER
	if (trial % 2 == 0) {
		std::cout << "baseline "; run<T, BaselineMatrix>(n, kind, repetitions, seed);
		std::cout << "candidate "; run<T, Matrix>(n, kind, repetitions, seed);
	} else {
		std::cout << "candidate "; run<T, Matrix>(n, kind, repetitions, seed);
		std::cout << "baseline "; run<T, BaselineMatrix>(n, kind, repetitions, seed);
	}
#else
	std::cout << "candidate "; run<T, Matrix>(n, kind, repetitions, seed);
#endif
}

int main(int argc, char **argv) {
	if (argc != 7) return 1;
	std::string type = argv[1];
	int n = std::stoi(argv[2]), repetitions = std::stoi(argv[4]), trial = std::stoi(argv[6]);
	auto seed = std::stoull(argv[5]);
	if (type == "mint") pair<mint>(n, argv[3], repetitions, seed, trial);
	else if (type == "mod31") pair<Mod<2147483647>>(n, argv[3], repetitions, seed, trial);
	else if (type == "mod2") pair<Mod<2>>(n, argv[3], repetitions, seed, trial);
	else if (type == "generic") pair<Generic<mint>>(n, argv[3], repetitions, seed, trial);
	else return 1;
}
