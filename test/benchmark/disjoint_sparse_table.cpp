#include "data_structure/disjoint_sparse_table.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>
#include <random>
#include <ratio>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;
struct Affine {
	uint64_t a, b;
};
constexpr uint64_t dst_mod = 998244353;
Affine compose(Affine f, Affine g) { return {g.a * f.a % dst_mod, (g.a * f.b + g.b) % dst_mod}; }
uint64_t minimum(uint64_t a, uint64_t b) { return min(a, b); }
uint64_t sink = 0;
template <class F>
double timed(F f) {
	auto start = chrono::steady_clock::now();
	f();
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}
template <class S, auto op>
struct ReferenceDisjointSparseTable {
	vector<vector<S>> data;
	void build(const vector<S> &a) {
		int levels = max(1, int(bit_width(unsigned(max(0, static_cast<int>(a.size()) - 1)))));
		data.assign(levels, a);
		for (int k = 1; k < levels; ++k) {
			size_t half = size_t(1) << k, block = half * 2;
			for (size_t start = 0; start < a.size(); start += block) {
				size_t mid = min(start + half, a.size()), end = min(start + block, a.size());
				for (size_t i = mid - 1; i > start; --i) data[k][i - 1] = op(a[i - 1], data[k][i]);
				for (size_t i = mid + 1; i < end; ++i) data[k][i] = op(data[k][i - 1], a[i]);
			}
		}
	}
	[[gnu::always_inline]] S prod(int l, int r) const {
		if (l == --r) return data[0][l];
		const auto &row = data[bit_width(unsigned(l ^ r)) - 1];
		return op(row[l], row[r]);
	}
};
template <class S, auto op, bool reference = false>
void bench(int n, int q, bool narrow) {
	mt19937_64 random(87654321);
	vector<S> a(n);
	for (auto &x : a) {
		if constexpr (is_integral_v<S>) x = random() % dst_mod;
		else x = {random() % dst_mod, random() % dst_mod};
	}
	vector<pair<int, int>> queries(q);
	for (auto &[l, r] : queries) {
		l = random() % n;
		r = random() % n;
		if (l > r) swap(l, r);
		++r;
		if (narrow) r = min(n, l + 1 + int(random() % 8));
	}
	conditional_t<reference, ReferenceDisjointSparseTable<S, op>, DisjointSparseTable<S, op>> table;
	double build = timed([&] { table.build(a); });
	auto ask = [&] {
		uint64_t total = 0;
		for (auto [l, r] : queries) {
			auto v = table.prod(l, r);
			if constexpr (is_integral_v<S>) total += v;
			else total += v.a ^ v.b;
		}
		return total;
	};
	sink = ask();
	double query = timed([&] { sink += ask(); });
	cout << fixed << setprecision(3) << "build=" << build << " query=" << query << " checksum=" << sink << '\n';
}
int main(int argc, char **argv) {
	int n = argc > 1 ? atoi(argv[1]) : 200000, q = argc > 2 ? atoi(argv[2]) : 1000000;
	bool affine = argc > 3 && atoi(argv[3]), narrow = argc > 4 && atoi(argv[4]);
	if (n <= 0 || q < 0) return 1;
	if (argc > 5 && atoi(argv[5])) {
		if (affine) bench<Affine, compose, true>(n, q, narrow);
		else bench<uint64_t, minimum, true>(n, q, narrow);
	} else {
		if (affine) bench<Affine, compose>(n, q, narrow);
		else bench<uint64_t, minimum>(n, q, narrow);
	}
}
