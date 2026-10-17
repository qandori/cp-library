#include CP_POLYNOMIAL_GCD_VARIANTS
#include <algorithm>
#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sched.h>
#include <string>
#include <time.h>

struct Result { fps d, x, y; };
#define RUN(Name) \
	if (op == 0) return {Name##_polynomial_gcd(a, b), {}, {}}; \
	if (op == 1) { auto z = Name##_polynomial_extended_gcd(a, b); return {std::move(z.gcd), std::move(z.x), std::move(z.y)}; } \
	{ auto z = Name##_polynomial_inv_mod(b, a); return {z ? *z : fps{998244352}, {}, {}}; }

Result run(int variant, int op, const fps& a, const fps& b) {
	switch (variant) {
		case 0: { RUN(Base) }
		case 1: { RUN(Column) }
		default: { RUN(FusedColumn) }
	}
}
#undef RUN

unsigned long long digest(const Result& result) {
	unsigned long long hash = 0;
	for (auto* f : {&result.d, &result.x, &result.y}) {
		hash = hash * 1000000007 + f->size();
		for (auto value : *f) hash = hash * 1000000007 + value.val();
	}
	return hash;
}

double cpu() {
	timespec value;
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &value)) std::abort();
	return value.tv_sec + value.tv_nsec * 1e-9;
}

int main(int argc, char** argv) {
	if (argc != 7) return 1;
	int n = std::stoi(argv[1]), op = std::stoi(argv[2]), mode = std::stoi(argv[3]);
	int trials = std::stoi(argv[4]), processor = std::stoi(argv[5]);
	double minimum = std::stod(argv[6]);
	if (minimum < 0.15 || n < 1 || trials < 1) return 1;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(processor, &mask);
	if (sched_setaffinity(0, sizeof(mask), &mask)) return 2;
	std::mt19937 random(294384);
	fps a(n), b(n - 1);
	for (auto* f : {&a, &b}) {
		for (auto& x : *f) x = random();
		if (!f->empty()) f->back() = 1;
	}
	if (mode == 1) {
		a.assign(n, 0), b.assign(n / 2, 0);
		a.front() = b.front() = -1, a.back() = b.back() = 1;
	}
	if (mode == 2) b = a;
	if (mode == 3) b = {7};
	if (mode == 4) b.resize(n / 8);
	if (mode == 5) {
		fps common(n / 3);
		for (auto& x : common) x = random();
		common.back() = 1;
		a *= common, b *= common;
	}
	auto expected = digest(run(0, op, a, b));
	for (int variant = 1; variant < 3; ++variant)
		if (digest(run(variant, op, a, b)) != expected) return 3;
	std::cout << std::setprecision(17);
	for (int trial = 0; trial < trials; ++trial) {
		std::array<int, 3> order{0, 1, 2};
		std::rotate(order.begin(), order.begin() + (trial / 2) % 3, order.end());
		if (trial & 1) std::reverse(order.begin(), order.end());
		for (int variant : order) {
			double total = 0;
			int count = 0;
			const int batch = n <= 256 ? 64 : 1;
			std::vector<Result> results(batch);
			do {
				double start = cpu();
				for (auto& result : results) result = run(variant, op, a, b);
				total += cpu() - start;
				count += batch;
				for (const auto& result : results) if (digest(result) != expected) return 4;
			} while (total < minimum);
			std::cout << n << ',' << op << ',' << mode << ',' << trial << ',' << variant << ',' << count
				<< ',' << total << ',' << total / count * 1000 << ',' << expected << '\n';
		}
	}
}
