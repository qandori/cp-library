#if defined(BENCH_BOOST_FIXED) || defined(BENCH_BOOST_DYNAMIC)
#include <boost/multiprecision/cpp_int.hpp>
#if defined(BENCH_BOOST_FIXED)
using Integer = boost::multiprecision::uint512_t;
#else
using Integer = boost::multiprecision::cpp_int;
#endif
#else
#include "math/bigint.hpp"
using Integer = BigInt;
#endif

#ifndef BENCH_DIJKSTRA_HEADER
#define BENCH_DIJKSTRA_HEADER "graph/dijkstra.hpp"
#endif
#include BENCH_DIJKSTRA_HEADER
#include "components/fast_io.hpp"
#include <chrono>
#include <iostream>
#include <numeric>

Integer gcd_value(Integer a, Integer b) {
#if defined(BENCH_BOOST_FIXED) || defined(BENCH_BOOST_DYNAMIC) || defined(BENCH_HAS_GCD)
	return gcd(a, b);
#else
	while (b != 0) { Integer r = a % b; a = std::move(b); b = std::move(r); }
	return a;
#endif
}

int main() {
	using Clock = std::chrono::steady_clock;
	auto start = Clock::now();
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	if (!input.read(n)) return 0;
	input.read(m);
	struct Edge { int u, v, a, b; };
	std::vector<Edge> edges(m);
	for (auto &e : edges) input.read(e.u), input.read(e.v), input.read(e.a), input.read(e.b);
	Integer denominator = 1;
	for (int b = 2; b <= 300; ++b) denominator *= b / std::gcd(b, static_cast<int>(Integer(denominator % b)));
	std::vector<std::vector<std::pair<int, Integer>>> graph(n);
	std::vector<Integer> unit(301);
	for (int b = 1; b <= 300; ++b) unit[b] = denominator / b;
	for (auto e : edges) {
		Integer weight = unit[e.b] * e.a;
		graph[e.u - 1].emplace_back(e.v - 1, weight);
		graph[e.v - 1].emplace_back(e.u - 1, std::move(weight));
	}
	auto setup = Clock::now();
	auto distance = dijkstra(graph, 0, Integer(denominator * (300LL * n + 1)));
	auto shortest = Clock::now();
	std::vector<Integer> divisors(n), numerator(n), reduced_denominator(n);
	for (int i = 1; i < n; ++i) divisors[i] = gcd_value(distance[i], denominator);
	auto reduced = Clock::now();
	for (int i = 1; i < n; ++i) {
		numerator[i] = distance[i] / divisors[i];
		reduced_denominator[i] = denominator / divisors[i];
	}
	auto divided = Clock::now();
	for (int i = 1; i < n; ++i) {
		output.write(numerator[i].str()); output.write(' '); output.println(reduced_denominator[i].str());
	}
	output.flush();
	auto end = Clock::now();
	auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
	std::cerr << "{\"setup\":" << ms(start, setup) << ",\"dijkstra\":" << ms(setup, shortest)
		<< ",\"gcd\":" << ms(shortest, reduced) << ",\"division\":" << ms(reduced, divided)
		<< ",\"output\":" << ms(divided, end) << "}\n";
}
