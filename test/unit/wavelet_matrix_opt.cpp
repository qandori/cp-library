#include "data_structure/wavelet_matrix.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <vector>

void require(bool ok) {
	if (!ok) std::abort();
}

template <class T>
void check_range(const WaveletMatrix<T> &wm, const std::vector<T> &a, int l, int r, const std::vector<T> &probes) {
	std::vector<T> sorted(a.begin() + l, a.begin() + r);
	std::sort(sorted.begin(), sorted.end());
	for (const auto &probe : probes) {
		T x = probe;
		auto it = std::lower_bound(sorted.begin(), sorted.end(), x);
		int below = int(it - sorted.begin());
		require(wm.range_freq(l, r, x) == below);
		require(wm.count(l, r, x) == std::count(sorted.begin(), sorted.end(), x));
		require(wm.rank(x, r) == std::count(a.begin(), a.begin() + r, x));
		std::optional<T> previous, next;
		if (below) previous = T(sorted[below - 1]);
		if (below < r - l) next = T(sorted[below]);
		require(wm.prev_value(l, r, x) == previous);
		require(wm.next_value(l, r, x) == next);
	}
	for (int i = 1; i < int(probes.size()); ++i) {
		int expected = 0;
		for (const auto &v : sorted) expected += !(v < probes[i - 1]) && v < probes[i];
		require(wm.range_freq(l, r, probes[i - 1], probes[i]) == expected);
		require(wm.range_freq(l, r, probes[i], probes[i - 1]) == 0);
	}
	std::vector<int> ks{0, r - l};
	if (r - l <= 40) {
		for (int k = 1; k < r - l; ++k) ks.push_back(k);
	} else for (int k : {1, 7, 8, 9, (r - l) / 2, r - l - 1})
		if (k <= r - l) ks.push_back(k);
	for (int k : ks) {
		if (k < r - l) {
			require(wm.kth_smallest(l, r, k) == sorted[k]);
			require(wm.kth_largest(l, r, k) == sorted[r - l - k - 1]);
		}
		if constexpr (requires { wm.sum_smallest(l, r, k); }) {
			using S = typename WaveletMatrix<T>::sum_type;
			S low = 0, high = 0;
			for (int i = 0; i < k; ++i) low += S(sorted[i]), high += S(sorted[r - l - i - 1]);
			require(wm.sum_smallest(l, r, k) == low);
			require(wm.sum_largest(l, r, k) == high);
		}
	}
	if constexpr (requires { wm.range_sum(l, r); }) {
		using S = typename WaveletMatrix<T>::sum_type;
		S sum = 0;
		for (const auto &v : sorted) sum += S(v);
		require(wm.range_sum(l, r) == sum);
	}
}

template <class T>
void check(const std::vector<T> &a, const std::vector<T> &probes) {
	int n = int(a.size());
	WaveletMatrix<T> lazy(a), copy(lazy), rebuilt;
	auto prepared = lazy;
	prepared.prepare_count();
	prepared.prepare_count();
	auto prepared_copy = prepared;
	std::optional<WaveletMatrix<T>> eager;
	if constexpr (requires { WaveletMatrix<T>(a, true); }) eager.emplace(a, true);
	if (eager) eager->prepare_count();
	std::vector<int> endpoints{0, n};
	for (int x : {1, 8, 9, 15, 16, 17, 30, 31, 32, 33, 47, 48, 49, 62, 63, 64, 65, 95, 96, 97})
		if (x <= n) endpoints.push_back(x);
	std::sort(endpoints.begin(), endpoints.end());
	endpoints.erase(std::unique(endpoints.begin(), endpoints.end()), endpoints.end());
	for (int l : endpoints)
		for (int r : endpoints)
			if (l <= r) check_range(lazy, a, l, r, probes);
	for (int l : endpoints) check_range(prepared, a, l, n, probes);
	if (eager) {
		for (int l : endpoints) check_range(*eager, a, l, n, probes);
		auto eager_copy = *eager;
		check_range(eager_copy, a, 0, n, probes);
	}
	for (int i = 0; i < n; ++i) require(lazy.access(i) == a[i] && lazy[i] == a[i]);
	for (const auto &probe : probes) {
		T x = probe;
		int k = 0;
		require(lazy.select(x, -1) == -1);
		for (int i = 0; i < n; ++i)
			if (a[i] == x) {
				require(lazy.select(x, k) == i);
				require(prepared_copy.select(x, k++) == i);
			}
		require(lazy.select(x, k) == -1);
		require(prepared_copy.select(x, k) == -1);
	}
	lazy.build({});
	prepared.build({});
	prepared.prepare_count();
	require(lazy.empty());
	check_range(prepared, std::vector<T>{}, 0, 0, probes);
	check_range(prepared_copy, a, 0, n, probes);
	check_range(copy, a, 0, n, probes);
	for (int cardinality : {1, 7, 8, 9, 63, 64, 65, 0}) {
		std::vector<T> b;
		for (int i = 0; i < std::min(n, cardinality); ++i) b.push_back(a[i]);
		rebuilt.build(b);
		rebuilt.prepare_count();
		check_range(rebuilt, b, 0, int(b.size()), probes);
	}
	rebuilt.build(a);
	rebuilt.prepare_count();
	check_range(rebuilt, a, 0, n, probes);
	std::vector<T> subset(a.begin(), a.begin() + n / 2);
	rebuilt.build(subset);
	rebuilt.prepare_count();
	check_range(rebuilt, subset, 0, int(subset.size()), probes);
}

struct Key {
	int value;
	explicit Key(int value) : value(value) {}
	bool operator<(const Key &other) const { return value < other.value; }
	bool operator==(const Key &other) const { return value == other.value; }
};

int main() {
	std::mt19937_64 random(20261019);
	for (int n : {0, 1, 31, 32, 33, 63, 64, 65}) {
		std::vector<int> a(n);
		for (int i = 0; i < n; ++i) a[i] = 2 * i - n;
		std::shuffle(a.begin(), a.end(), random);
		check(a, std::vector<int>{-1000, -65, -64, -63, -1, 0, 1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 1000});
		std::vector<bool> flags(n);
		for (int i = 0; i < n; ++i) flags[i] = (i % 7) < 3;
		check(flags, std::vector<bool>{false, true});
	}
	for (int sigma : {7, 8, 9, 63, 64, 65, 511, 512, 513, 4095, 4096, 4097}) {
		std::vector<int> a(sigma + 97), probes{-1, 0, 1, 2 * sigma - 1, 2 * sigma, 2 * sigma + 1};
		for (int i = 0; i < int(a.size()); ++i) a[i] = 2 * (i % sigma);
		std::shuffle(a.begin(), a.end(), random);
		for (int power = 1; power < sigma; power *= 8)
			for (int delta = -2; delta <= 2; ++delta) probes.push_back(2 * power + delta);
		std::sort(probes.begin(), probes.end());
		probes.erase(std::unique(probes.begin(), probes.end()), probes.end());
		check(a, probes);
	}
	for (bool skewed : {false, true}) {
		std::vector<int> a(22000);
		for (int i = 0; i < int(a.size()); ++i) a[i] = skewed && i >= 1024 ? 0 : 2 * (i % 1024);
		std::shuffle(a.begin(), a.end(), random);
		WaveletMatrix<int> wm(a);
		for (int repeat = 0; repeat < 1000; ++repeat) {
			int l = random() % a.size(), r = random() % a.size();
			if (l > r) std::swap(l, r);
			++r;
			int x = 2 * int(random() % 1025);
			require(wm.count(l, r, x) == std::count(a.begin() + l, a.begin() + r, x));
		}
	}
	std::vector<std::uint64_t> unsigned_values(65);
	for (int i = 0; i < 65; ++i)
		unsigned_values[i] = i % 2 ? std::numeric_limits<std::uint64_t>::max() - i : std::uint64_t(i);
	std::shuffle(unsigned_values.begin(), unsigned_values.end(), random);
	check(unsigned_values, std::vector<std::uint64_t>{0, 1, 31, 32, 33, std::uint64_t(1) << 63,
			std::numeric_limits<std::uint64_t>::max() - 32, std::numeric_limits<std::uint64_t>::max() - 1,
			std::numeric_limits<std::uint64_t>::max()});
	std::vector<std::int64_t> signed_values(65);
	for (int i = 0; i < 65; ++i)
		signed_values[i] = i % 2 ? std::numeric_limits<std::int64_t>::min() + i : std::numeric_limits<std::int64_t>::max() - i;
	std::shuffle(signed_values.begin(), signed_values.end(), random);
	check(signed_values, std::vector<std::int64_t>{std::numeric_limits<std::int64_t>::min(),
			std::numeric_limits<std::int64_t>::min() + 32, -1, 0, 1,
			std::numeric_limits<std::int64_t>::max() - 32, std::numeric_limits<std::int64_t>::max()});
	std::vector<std::string> words;
	std::vector<Key> keys;
	for (int i = 0; i < 97; ++i) {
		int x = (i * 37) % 65;
		words.push_back(std::to_string(1000 + 2 * x));
		keys.emplace_back(2 * x);
	}
	check(words, std::vector<std::string>{"", "1000", "1001", "1063", "1064", "1065", "1128", "1129", "z"});
	check(keys, std::vector<Key>{Key(-1), Key(0), Key(1), Key(63), Key(64), Key(65), Key(127), Key(128), Key(129)});
	for (int sigma : {1023, 1024, 1025}) {
		int n = 33 * sigma + 1;
		std::vector<int> a(n);
		for (int i = 0; i < n; ++i) a[i] = 2 * (i % sigma);
		std::shuffle(a.begin(), a.end(), random);
		WaveletMatrix wm(a);
		for (int x : {-1, 0, 1, sigma - 1, sigma, sigma + 1, 2 * sigma - 2, 2 * sigma - 1}) {
			int k = 0;
			for (int i = 0; i < n; ++i)
				if (a[i] == x) require(wm.select(x, k++) == i);
			require(wm.select(x, k) == -1);
			for (int l : {0, 1, 31, 32, n / 2, n - 32, n})
				for (int r : {0, 1, 31, 32, n / 2, n - 32, n})
					if (l <= r) require(wm.count(l, r, x) == std::count(a.begin() + l, a.begin() + r, x));
		}
	}
	std::vector<std::uint64_t> shifted_unsigned(2049);
	std::vector<std::int64_t> shifted_signed(2049);
	for (int i = 0; i < 2049; ++i) {
		std::int64_t offset = 257 * (i % 1025) - 131072;
		shifted_unsigned[i] = (std::uint64_t(1) << 63) + offset;
		shifted_signed[i] = offset;
	}
	std::shuffle(shifted_unsigned.begin(), shifted_unsigned.end(), random);
	std::shuffle(shifted_signed.begin(), shifted_signed.end(), random);
	check(shifted_unsigned, std::vector<std::uint64_t>{0, (std::uint64_t(1) << 63) - 131073,
			(std::uint64_t(1) << 63) - 131072, std::uint64_t(1) << 63,
			(std::uint64_t(1) << 63) + 132096, std::numeric_limits<std::uint64_t>::max()});
	check(shifted_signed, std::vector<std::int64_t>{-131073, -131072, -1, 0, 1, 132096, 132097});
	std::vector<int> dense(2049);
	for (int i = 0; i < int(dense.size()); ++i) dense[i] = i;
	std::shuffle(dense.begin(), dense.end(), random);
	check(dense, std::vector<int>{-1, 0, 1, 1024, 2048, 2049});
	std::puts("Wavelet Matrix boundaries OK");
}
