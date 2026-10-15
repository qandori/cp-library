#include "math/min_of_mod_of_linear.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;

U euclid(U n, U m, U a, U b) {
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	U ans = b;
	while (a && ans) {
		W z = W(a) * (n - 1) + b;
		if (z < m) break;
		n = z / m;
		b = U(z % m) % a;
		ans = std::min(ans, b);
		U t = m;
		m = a;
		a = t % m;
	}
	return ans;
}
U euclid64(U n, U m, U a, U b) {
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	U ans = b;
	while (a && ans) {
		W z = W(a) * (n - 1) + b;
		if (z < m) break;
		if (z >> 64) {
			n = z / m;
			b = z % m;
		} else {
			U v = z;
			n = v / m;
			b = v % m;
		}
		b %= a;
		ans = std::min(ans, b);
		U t = m;
		m = a;
		a = t % m;
	}
	return ans;
}
U stern(U n, U m, U a, U b) {
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	if (!a || !b) return b;
	U p = m - a, q = a, sp = 1, sq = 1, rem = n - 1;
	for (;;) {
		if (p <= b) {
			U k = std::min(b / p, rem / sp);
			b -= k * p;
			rem -= k * sp;
			if (!b || rem < sp) return b;
		}
		if (sp > rem || sq > rem - sp) return b;
		if (p > q) {
			U k = std::min(std::min((p - b - 1) / q + 1, p / q), (rem - sp) / sq);
			p -= k * q;
			sp += k * sq;
			if (!p) return b;
		} else {
			U k = q / p;
			if (k > (rem - sq) / sp) return b;
			q -= k * p;
			sq += k * sp;
			if (!q) return b;
		}
	}
}
U stern_fast(U n, U m, U a, U b) {
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	if (!a || !b) return b;
	U p = m - a, q = a, sp = 1, sq = 1, rem = n - 1;
	for (;;) {
		if (p <= b) {
			U k = b / p;
			if (W(k) * sp > rem) return b - (rem / sp) * p;
			b %= p;
			rem -= k * sp;
			if (!b) return 0;
		}
		if (sp > rem || sq > rem - sp) return b;
		if (p > q) {
			U k = (p - b - 1) / q + 1;
			U d = k * q;
			if (d == p) return b;
			if (d > p) --k;
			sp += k * sq;
			if (sp > rem) return b;
			p -= k * q;
		} else {
			U k = q / p;
			q %= p;
			if (!q) return b;
			sq += k * sp;
		}
	}
}

int main(int argc, char **argv) {
	std::string kind = argc > 1 ? argv[1] : "random";
	int count = argc > 2 ? std::atoi(argv[2]) : 300000;
	int repeats = argc > 3 ? std::atoi(argv[3]) : 5;
	if (count < 1 || repeats < 1) return 1;
	if (kind != "random" && kind != "short" && kind != "fibonacci" && kind != "wide" && kind != "wide_short" && kind != "fibonacci64" && kind != "period") return 1;
	std::mt19937_64 random(713);
	std::vector<std::array<U, 4>> data;
	data.reserve(count);
	for (int i = 0; i < count; ++i) {
		bool wide = kind == "wide" || kind == "wide_short" || kind == "fibonacci64";
		U m = wide ? (random() | 1) : random() % 1000000000 + 1;
		U a = random() % m, b = random() % m;
		if (kind == "fibonacci") m = 701408733, a = 433494437, b %= m;
		if (kind == "fibonacci64") m = 12200160415121876738ULL, a = 7540113804746346429ULL, b %= m;
		U n = kind == "short" || kind == "wide_short" ? random() % 64 + 1 : kind == "period"						   ? m
																		: kind == "fibonacci" || kind == "fibonacci64" ? random() % m + 1
																		: wide										   ? (random() | 1)
																													   : random() % 1000000000 + 1;
		data.push_back({n, m, a, b});
	}
	using F = U (*)(U, U, U, U);
	std::vector<std::pair<std::string, F>> functions{
		{"wide_euclid", euclid}, {"native_euclid", euclid64}, {"bounded_fractions", stern}, {"fractions", stern_fast}, {"library", min_of_mod_of_linear}};
	U expected = 0;
	bool initialized = false;
	for (int repeat = 0; repeat < repeats; ++repeat) {
		std::shuffle(functions.begin(), functions.end(), random);
		for (auto [name, function] : functions) {
			U checksum = 0;
			auto start = std::chrono::steady_clock::now();
			for (auto [n, m, a, b] : data) checksum += function(n, m, a, b);
			double milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			if (!initialized) expected = checksum, initialized = true;
			if (checksum != expected) {
				std::cerr << "checksum mismatch: " << name << '\n';
				return 1;
			}
			std::cout << kind << ' ' << name << ' ' << repeat << ' ' << std::fixed << std::setprecision(3) << milliseconds << ' ' << checksum << std::endl;
		}
	}
}
