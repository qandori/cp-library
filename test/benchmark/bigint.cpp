#include "math/bigint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using BoostInt = boost::multiprecision::cpp_int;
using Clock = std::chrono::steady_clock;
std::string random_decimal(std::mt19937_64 &rng, int n) {
	std::string s(n, '0');
	for (char &c : s) c += rng() % 10;
	s[0] = '1' + rng() % 9;
	return s;
}
template <class T>
std::string string_of(const T &x) { return x.str(); }
template <class T, class F>
double measure(const std::vector<T> &a, const std::vector<T> &b, int repetitions, F operation, std::string &last, std::uint64_t &checksum) {
	std::vector<T> result(repetitions);
	auto begin = Clock::now();
	for (int i = 0; i < repetitions; ++i) result[i] = operation(a[i % a.size()], b[i % b.size()]);
	auto end = Clock::now();
	checksum = 0;
	for (const auto &value : result) checksum = checksum * 1000000007 + static_cast<std::uint64_t>(value);
	last = string_of(result.back());
	return std::chrono::duration<double, std::milli>(end - begin).count() / repetitions;
}
int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 10000;
	int repetitions = argc > 2 ? std::atoi(argv[2]) : std::max(1, 1000000 / n);
	bool reverse = argc > 3 && std::atoi(argv[3]);
	std::string filter = argc > 4 ? argv[4] : "";
	std::mt19937_64 rng(876);
	std::vector<std::string> sa, sb;
	std::vector<BigInt> a, b, half;
	std::vector<BoostInt> x, y, z;
	for (int i = 0; i < 4; ++i) {
		sa.push_back(random_decimal(rng, n));
		sb.push_back(random_decimal(rng, n));
		a.emplace_back(sa.back()); b.emplace_back(sb.back());
		x.emplace_back(sa.back()); y.emplace_back(sb.back());
		auto small = random_decimal(rng, std::max(1, n / 2));
		half.emplace_back(small); z.emplace_back(small);
	}
	auto arithmetic = [&](const char *name, auto f, const auto &bb, const auto &yy) {
		if (!filter.empty() && name != filter) return;
		std::string mine, boost;
		double t, u;
		std::uint64_t check_mine, check_boost;
		if (!reverse) {
			t = measure(a, bb, repetitions, f, mine, check_mine);
			u = measure(x, yy, repetitions, f, boost, check_boost);
		} else {
			u = measure(x, yy, repetitions, f, boost, check_boost);
			t = measure(a, bb, repetitions, f, mine, check_mine);
		}
		if (mine != boost || check_mine != check_boost) std::abort();
		std::cout << n << ',' << name << ',' << t << ',' << u << ',' << u / t << '\n';
	};
	std::cout << std::fixed << std::setprecision(6);
	arithmetic("add", [](const auto &a, const auto &b) { using T = std::decay_t<decltype(a)>; return T(a + b); }, b, y);
	arithmetic("multiply", [](const auto &a, const auto &b) { using T = std::decay_t<decltype(a)>; return T(a * b); }, b, y);
	arithmetic("square", [](const auto &a, const auto &) { using T = std::decay_t<decltype(a)>; return T(a * a); }, b, y);
	arithmetic("divide", [](const auto &a, const auto &b) { using T = std::decay_t<decltype(a)>; return T(a / b); }, half, z);
	arithmetic("divide_small", [](const auto &a, const auto &) { using T = std::decay_t<decltype(a)>; return T(a / 998244353); }, b, y);
	if (!filter.empty()) return 0;
	auto conversions = [&]<class T>(const std::vector<T> &v) {
		std::vector<T> parsed(repetitions);
		auto begin = Clock::now();
		for (int i = 0; i < repetitions; ++i) parsed[i] = T(sa[i % sa.size()]);
		auto middle = Clock::now();
		std::vector<std::string> strings(repetitions);
		for (int i = 0; i < repetitions; ++i) strings[i] = string_of(v[i % v.size()]);
		auto end = Clock::now();
		std::uint64_t checksum = 0, expected = 0;
		for (int i = 0; i < repetitions; ++i) {
			checksum = checksum * 1000000007 + static_cast<std::uint64_t>(parsed[i]);
			expected = expected * 1000000007 + static_cast<std::uint64_t>(v[i % v.size()]);
			if (strings[i] != sa[i % sa.size()]) std::abort();
		}
		if (checksum != expected) std::abort();
		return std::pair(std::chrono::duration<double, std::milli>(middle - begin).count() / repetitions,
			std::chrono::duration<double, std::milli>(end - middle).count() / repetitions);
	};
	std::pair<double, double> t, u;
	if (!reverse) t = conversions(a), u = conversions(x);
	else u = conversions(x), t = conversions(a);
	std::cout << n << ",parse," << t.first << ',' << u.first << ',' << u.first / t.first << '\n';
	std::cout << n << ",format," << t.second << ',' << u.second << ',' << u.second / t.second << '\n';
}
