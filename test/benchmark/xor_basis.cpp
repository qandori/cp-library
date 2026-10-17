#include "math/xor_basis.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

struct ScanningBasis {
	std::array<std::uint64_t, 64> row{};
	void insert(std::uint64_t x) {
		for (int i = 63; i >= 0; --i) if ((x >> i) & 1) {
			if (!row[i]) { row[i] = x; return; }
			x ^= row[i];
		}
	}
	std::uint64_t min_xor(std::uint64_t x) const { for (int i = 63; i >= 0; --i) x = std::min(x, x ^ row[i]); return x; }
	std::uint64_t max_xor(std::uint64_t x) const { for (int i = 63; i >= 0; --i) x = std::max(x, x ^ row[i]); return x; }
};

struct CompactBasis {
	std::array<std::uint64_t, 64> row{};
	int n = 0;
	void insert(std::uint64_t x) {
		for (int i = 0; i < n; ++i) x = std::min(x, x ^ row[i]);
		if (x) row[n++] = x;
	}
	std::uint64_t min_xor(std::uint64_t x) const { for (int i = 0; i < n; ++i) x = std::min(x, x ^ row[i]); return x; }
	std::uint64_t max_xor(std::uint64_t x) const { for (int i = 0; i < n; ++i) x = std::max(x, x ^ row[i]); return x; }
};

template <class Basis>
void run(const std::string &operation, int rank, int count) {
	std::mt19937_64 random(713);
	std::vector<std::uint64_t> generators;
	XorBasis<> independent;
	while (int(generators.size()) < rank) {
		auto x = random(); if (independent.insert(x)) generators.push_back(x);
	}
	std::vector<std::uint64_t> values;
	for (int i = 0; i < (operation == "build" ? 96 : count); ++i) {
		std::uint64_t x = 0, coefficients = random();
		for (int j = 0; j < rank; ++j) if ((coefficients >> j) & 1) x ^= generators[j];
		values.push_back(operation == "build" ? x : random());
	}
	Basis basis;
	for (auto x : generators) basis.insert(x);
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	if (operation == "build") {
		for (int round = 0; round < count; ++round) {
			Basis next;
			for (auto x : values) next.insert(x);
			checksum += next.max_xor(std::uint64_t(round));
		}
	} else for (auto x : values) checksum += basis.min_xor(x) + std::rotl(basis.max_xor(x), 17);
	auto end = std::chrono::steady_clock::now();
	std::cout << std::setprecision(12) << std::chrono::duration<double>(end - start).count() << '\n' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string variant = argv[1], operation = argv[2];
	int rank = std::atoi(argv[3]), count = std::atoi(argv[4]);
	if (variant == "scanning") run<ScanningBasis>(operation, rank, count);
	else if (variant == "compact") run<CompactBasis>(operation, rank, count);
	else if (variant == "library") run<XorBasis<>>(operation, rank, count);
	else return 1;
}
