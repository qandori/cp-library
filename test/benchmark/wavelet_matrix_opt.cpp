#ifndef WM_OPT_HEADER
#define WM_OPT_HEADER "data_structure/wavelet_matrix.hpp"
#endif
#include WM_OPT_HEADER
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

struct Query {
	int l, r, k;
	std::int64_t x, y;
};

template <class T>
void run(int n, int q, int mode, int length, const std::string &op, unsigned seed) {
	std::mt19937_64 random(seed);
	std::vector<T> a(n);
	for (auto &x : a) {
		if (mode == 1) x = T(random() % 256);
		else if (mode == 2) x = T(random());
		else if (mode == 5) x = T(7);
		else if (mode == 6) x = T(random() % 2);
		else if (mode == 7) x = T(random() % 4096);
		else if (mode == 8) x = T(2 * (random() % 1000000000));
		else if (mode >= 9) x = T(random() % (mode == 9 ? 512 : mode == 10 ? 1024 : mode == 11 ? 16384 : 65536));
		else x = T(random() % 1000000000);
	}
	if (mode == 13) {
		for (int i = 0; i < n; ++i) a[i] = T(3LL * (i % std::max(1, n / 2)));
		std::shuffle(a.begin(), a.end(), random);
	}
	if (mode == 3 || mode == 4) std::sort(a.begin(), a.end());
	if (mode == 4) std::reverse(a.begin(), a.end());
	std::vector<Query> queries(q);
	for (auto &z : queries) {
		z.l = random() % n;
		z.r = random() % n;
		if (z.l > z.r) std::swap(z.l, z.r);
		++z.r;
		if (length) z.r = std::min(n, z.l + 1 + int(random() % length));
		z.k = random() % (z.r - z.l);
		z.x = std::int64_t(a[random() % n]);
		z.y = std::int64_t(a[random() % n]);
		if (mode == 8 && (random() & 1)) ++z.x;
		if (z.x > z.y) std::swap(z.x, z.y);
	}
	std::uint64_t checksum = 0;
	auto add = [&](auto value) {
		__uint128_t x = __uint128_t(value);
		checksum = (checksum ^ std::uint64_t(x) ^ std::uint64_t(x >> 64)) * 0x9e3779b97f4a7c15;
	};
	std::optional<WaveletMatrix<T>> wm;
	if (op != "build" && op != "eager") wm.emplace(a);
	if (op == "small" || op == "large" || op == "sum") wm->prepare_sum();
	if (op == "prepared_count" || op == "prepared_select") {
		if constexpr (requires { wm->prepare_count(); }) wm->prepare_count();
	}
	auto start = std::chrono::steady_clock::now();
	if (op == "build" || op == "eager") {
		if (op == "build") wm.emplace(a);
		else wm.emplace(a, true);
		add(wm->size());
	} else if (op == "prepare") wm->prepare_sum(), add(wm->range_sum(0, n));
	else if (op == "prepare_count") {
		if constexpr (requires { wm->prepare_count(); }) wm->prepare_count();
		add(wm->size());
	}
	else if (op == "kth") for (auto z : queries) add(wm->kth_smallest(z.l, z.r, z.k));
	else if (op == "freq") for (auto z : queries) add(wm->range_freq(z.l, z.r, T(z.x)));
	else if (op == "between") for (auto z : queries) add(wm->range_freq(z.l, z.r, T(z.x), T(z.y)));
	else if (op == "count" || op == "prepared_count") for (auto z : queries) add(wm->count(z.l, z.r, T(z.x)));
	else if (op == "prev") for (auto z : queries) { auto x = wm->prev_value(z.l, z.r, T(z.x)); add(x ? __int128_t(*x) : -13); }
	else if (op == "next") for (auto z : queries) { auto x = wm->next_value(z.l, z.r, T(z.x)); add(x ? __int128_t(*x) : -17); }
	else if (op == "small") for (auto z : queries) add(wm->sum_smallest(z.l, z.r, z.k));
	else if (op == "large") for (auto z : queries) add(wm->sum_largest(z.l, z.r, z.k));
	else if (op == "sum") for (auto z : queries) add(wm->range_sum(z.l, z.r));
	else if (op == "select" || op == "prepared_select") for (auto z : queries) add(wm->select(T(z.x), 0));
	else if (op == "access") for (auto z : queries) add(wm->access(z.l));
	else std::exit(2);
	double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	std::cout.precision(12);
	std::cout << "{\"ms\":" << ms << ",\"checksum\":\"" << checksum << "\"}\n";
}

int main(int argc, char **argv) {
	if (argc != 8) return 2;
	int n = std::atoi(argv[1]), q = std::atoi(argv[2]), mode = std::atoi(argv[3]), length = std::atoi(argv[4]);
	if (n <= 0 || q <= 0 || mode < 0 || mode > 13 || length < 0) return 2;
	unsigned seed = unsigned(std::stoul(argv[7]));
	if (std::string(argv[6]) == "32") run<std::int32_t>(n, q, mode, length, argv[5], seed);
	else run<std::int64_t>(n, q, mode, length, argv[5], seed);
}
