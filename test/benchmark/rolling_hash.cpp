#include "string/rolling_hash.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Hash = RollingHash::Hash;
template <bool remainder>
class Reference {
	inline static std::vector<Hash> powers{1};
	std::vector<Hash> prefix;
	static Hash reduce(__uint128_t value) {
		if constexpr (remainder) return Hash(value % RollingHash::mod);
		Hash result = (Hash(value) & RollingHash::mod) + Hash(value >> 61);
		return result >= RollingHash::mod ? result - RollingHash::mod : result;
	}

  public:
	explicit Reference(const std::string &s) : prefix(s.size() + 1) {
		auto old = powers.size();
		if (old <= s.size()) {
			powers.resize(s.size() + 1);
			for (auto i = old; i < powers.size(); ++i) powers[i] = reduce(__uint128_t(powers[i - 1]) * RollingHash::base());
		}
		Hash value = 0, b = RollingHash::base();
		for (std::size_t i = 0; i < s.size(); ++i) {
			if constexpr (remainder) value = reduce(__uint128_t(value) * b + static_cast<unsigned char>(s[i]) + 1);
			else {
				value = reduce(__uint128_t(value) * b) + static_cast<unsigned char>(s[i]) + 1;
				if (value >= RollingHash::mod) value -= RollingHash::mod;
			}
			prefix[i + 1] = value;
		}
	}
	int size() const { return int(prefix.size() - 1); }
	Hash get(int l, int r) const {
		if (!l) return r ? prefix[r] : 0;
		Hash value = prefix[r] + RollingHash::mod - reduce(__uint128_t(prefix[l]) * powers[r - l]);
		return value >= RollingHash::mod ? value - RollingHash::mod : value;
	}
	int lcp(int l, const Reference &other, int j) const {
		int lo = 0, hi = std::min(size() - l, other.size() - j);
		while (lo < hi) {
			int m = hi - (hi - lo) / 2;
			if (get(l, l + m) == other.get(j, j + m)) lo = m;
			else hi = m - 1;
		}
		return lo;
	}
};
struct Result {
	double build, query, lcp;
	Hash checksum;
};
template <class H>
Result run(const std::string &s, const std::vector<std::pair<int, int>> &ranges) {
	auto start = std::chrono::steady_clock::now();
	H hash(s);
	auto built = std::chrono::steady_clock::now();
	Hash checksum = 0;
	for (auto [l, r] : ranges) checksum += hash.get(l, r);
	auto queried = std::chrono::steady_clock::now();
	for (int i = 0; i < std::min(100000, int(ranges.size())); ++i) {
		auto [l, r] = ranges[i];
		checksum += hash.lcp(l, hash, r);
	}
	auto done = std::chrono::steady_clock::now();
	return {std::chrono::duration<double, std::milli>(built - start).count(), std::chrono::duration<double, std::milli>(queried - built).count(), std::chrono::duration<double, std::milli>(done - queried).count(), checksum};
}
int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 1000000, q = argc > 2 ? std::atoi(argv[2]) : 1000000, repeats = argc > 3 ? std::atoi(argv[3]) : 7;
	std::string kind = argc > 4 ? argv[4] : "random";
	if (n < 1 || q < 1 || repeats < 1) return 1;
	std::mt19937 random(713);
	std::string s(n, 'a');
	if (kind != "same")
		for (char &c : s) c = char('a' + random() % 26);
	std::vector<std::pair<int, int>> ranges(q);
	for (auto &[l, r] : ranges) {
		l = random() % (n + 1);
		r = random() % (n + 1);
		if (l > r) std::swap(l, r);
	}
	RollingHash warm0(s);
	Reference<true> warm1(s);
	Reference<false> warm2(s);
	std::vector<int> order;
	for (int i = 0; i < repeats; ++i)
		for (int v = 0; v < 3; ++v) order.push_back(v);
	std::shuffle(order.begin(), order.end(), random);
	Hash expected = 0;
	bool first = true;
	for (int mode : order) {
		Result result = mode == 0 ? run<RollingHash>(s, ranges) : mode == 1 ? run<Reference<true>>(s, ranges)
																			: run<Reference<false>>(s, ranges);
		if (!first && result.checksum != expected) return 2;
		first = false;
		expected = result.checksum;
		std::cout << mode << ' ' << std::fixed << std::setprecision(3) << result.build << ' ' << result.query << ' ' << result.lcp << ' ' << result.checksum << '\n';
	}
}
