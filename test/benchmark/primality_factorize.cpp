#include "math/factorize.hpp"
#include "primality_factorize_baseline.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;

std::vector<U> inputs(const std::string &kind, int count) {
	std::mt19937_64 random(20261017);
	std::vector<U> a;
	a.reserve(count);
	for (int i = 0; i < count; ++i) {
		U n = random();
		if (kind.find("32") != std::string::npos) n = std::uint32_t(n);
		if (kind == "small") n = i;
		if (kind == "odd32" || kind == "odd64") n |= 1;
		if (kind == "prime32" || kind == "prime64") {
			n |= 1;
			while (!primality_factorize_baseline::is_prime(n)) {
				n += 2;
				if (kind == "prime32") n = std::uint32_t(n);
			}
		}
		if (kind == "semiprime" || kind == "hard") {
			U width = kind == "hard" ? 1000000000 : 100000;
			U lower = kind == "hard" ? 3000000000 : 100000;
			U p = (random() % width + lower) | 1, q = (random() % width + lower) | 1;
			while (!primality_factorize_baseline::is_prime(p)) p += 2;
			while (!primality_factorize_baseline::is_prime(q)) q += 2;
			n = p * q;
		}
		if (kind == "power") {
			n = 1;
			U p = primality_factorize_baseline::is_prime_detail::small_primes[random() % 12];
			while (n <= ~U(0) / p) {
				n *= p;
				if (random() % 3 == 0) break;
			}
		}
		a.push_back(n);
	}
	return a;
}

int main(int argc, char **argv) {
	std::string kind = argc > 1 ? argv[1] : "random32";
	int count = argc > 2 ? std::atoi(argv[2]) : 10000;
	int rounds = argc > 3 ? std::atoi(argv[3]) : 15;
	if (count <= 0 || rounds <= 0 || !(rounds & 1)) return 2;
	const auto a = inputs(kind, count);
	const bool factoring = kind == "factor32" || kind == "factor64" || kind == "semiprime" || kind == "hard" || kind == "power";
	U checksums[2]{};
	std::vector<double> times[2];
	for (int round = -1; round < rounds; ++round) {
		for (int order = 0; order < 2; ++order) {
			int impl = (order + (round & 1)) & 1;
			U checksum = 0;
			auto start = std::chrono::steady_clock::now();
			for (U n : a) {
				if (factoring) {
					auto factors = impl ? factorize(n ? n : 1) : primality_factorize_baseline::factorize(n ? n : 1);
					for (U p : factors) checksum = checksum * 1000000007 + p;
				} else checksum += impl ? is_prime(n) : primality_factorize_baseline::is_prime(n);
			}
			auto end = std::chrono::steady_clock::now();
			checksums[impl] = checksum;
			if (round >= 0) times[impl].push_back(std::chrono::duration<double, std::milli>(end - start).count());
		}
		if (checksums[0] != checksums[1]) return 1;
	}
	for (auto &t : times) std::sort(t.begin(), t.end());
	std::cout << kind << ',' << count << ',' << std::fixed << std::setprecision(6)
			  << times[0][rounds / 2] << ',' << times[1][rounds / 2] << ',' << checksums[0] << ',' << checksums[1] << '\n';
}
