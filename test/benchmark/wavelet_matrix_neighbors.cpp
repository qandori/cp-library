#if defined(WAVELET_CANDIDATE_FIRST)
#include "data_structure/wavelet_matrix.hpp"
#undef WAVELET_MATRIX_HPP
#endif
#define WaveletMatrix WaveletMatrixBaseline
#include WAVELET_BASELINE_HEADER
#undef WaveletMatrix
#undef WAVELET_MATRIX_HPP
#if !defined(WAVELET_CANDIDATE_FIRST)
#include "data_structure/wavelet_matrix.hpp"
#endif
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
using namespace std;

volatile uint64_t sink;
struct Query {
	int l, r, k;
	int64_t x, y;
};

template <class W>
uint64_t run(W &wm, const vector<int64_t> &a, const vector<Query> &queries, int op) {
	int n = a.size();
	uint64_t sum = 0;
	switch (op) {
	case 0: {
		W other(a);
		sum += other.size();
		break;
	}
	case 1:
		for (auto z : queries) sum += uint64_t(wm.kth_smallest(z.l, z.r, z.k));
		break;
	case 2:
		for (auto z : queries) sum += wm.range_freq(z.l, z.r, z.x);
		break;
	case 3:
		for (auto z : queries) sum += wm.range_freq(z.l, z.r, min(z.x, z.y), max(z.x, z.y));
		break;
	case 4:
		for (auto z : queries) {
			auto x = wm.prev_value(z.l, z.r, z.x);
			sum += x ? uint64_t(*x) : 13;
		}
		break;
	case 5:
		for (auto z : queries) {
			auto x = wm.next_value(z.l, z.r, z.x);
			sum += x ? uint64_t(*x) : 17;
		}
		break;
	case 6:
		for (auto z : queries) sum += wm.count(z.l, z.r, z.x);
		break;
	case 7:
		for (auto z : queries) sum += uint64_t(wm.sum_smallest(z.l, z.r, z.k));
		break;
	case 8:
		for (auto z : queries) sum += uint64_t(wm.sum_largest(z.l, z.r, z.k));
		break;
	case 9:
		for (auto z : queries) sum += uint64_t(wm.range_sum(z.l, z.r));
		break;
	case 10:
		for (auto z : queries) sum += wm.select(z.x, 0);
		break;
	case 11:
		for (auto z : queries) sum += uint64_t(wm.access(z.l));
		break;
	case 12: {
		W other(a, true);
		sum += uint64_t(other.range_sum(0, n));
		break;
	}
	case 13: {
		W other(a);
		other.prepare_sum();
		sum += uint64_t(other.range_sum(0, n));
		break;
	}
	}
	sink = sum;
	return sum;
}

int main(int argc, char **argv) {
	if (argc != 7 && argc != 8) return 2;
	int n = atoi(argv[1]), mode = atoi(argv[2]), op = atoi(argv[3]), length = atoi(argv[4]);
	double duration = atof(argv[5]);
	int pairs = atoi(argv[6]);
	if (n <= 0 || mode < 0 || mode > 8 || op < 0 || op > 13 || length < 0 || duration <= 0 || pairs <= 0) return 2;
	cout << setprecision(9);
	mt19937_64 rng(814253);
	vector<int64_t> a(n);
	for (int i = 0; i < n; ++i)
		a[i] = mode == 1 ? rng() % 256 : mode == 2 ? int64_t(rng()) : mode == 5 ? 7 : mode >= 6 ? rng() % (1u << (mode == 6 ? 1 : mode == 7 ? 2 : 4)) : rng() % 1000000000;
	if (mode == 3 || mode == 4) sort(a.begin(), a.end());
	if (mode == 4) reverse(a.begin(), a.end());
	vector<Query> queries(20000);
	for (auto &z : queries) {
		z.l = rng() % n;
		z.r = rng() % n;
		if (z.l > z.r) swap(z.l, z.r);
		++z.r;
		if (length) z.r = min(n, z.l + 1 + int(rng() % length));
		z.k = rng() % (z.r - z.l);
		z.x = a[rng() % n];
		z.y = a[rng() % n];
		if (argc == 7 && z.x > z.y) swap(z.x, z.y);
	}
	WaveletMatrixBaseline<int64_t> baseline(a);
	WaveletMatrix<int64_t> candidate(a);
	if (op >= 7 && op <= 9) {
		baseline.prepare_sum();
		candidate.prepare_sum();
	}
	for (int pair = 0; pair < pairs; ++pair) {
		double cpu[2]{}, wall[2]{};
		uint64_t checksum[2]{};
		int repetitions = 0;
		do {
			for (int j = 0; j < 2; ++j) {
				int variant = (pair + repetitions + j) & 1;
				auto start = chrono::steady_clock::now();
				auto ticks = clock();
				checksum[variant] = variant ? run(candidate, a, queries, op) : run(baseline, a, queries, op);
				cpu[variant] += double(clock() - ticks) / CLOCKS_PER_SEC;
				wall[variant] += chrono::duration<double>(chrono::steady_clock::now() - start).count();
			}
			if (checksum[0] != checksum[1]) return 1;
			++repetitions;
		} while (min(cpu[0], cpu[1]) < duration);
		for (int variant = 0; variant < 2; ++variant)
			cout << pair << ' ' << variant << ' ' << cpu[variant] / repetitions << ' '
				 << wall[variant] / repetitions << ' ' << checksum[variant] << ' ' << repetitions << '\n';
	}
}
