#include "before.hpp"
#include "scalar.hpp"
#include "components/predecessor_set.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

struct Query { int type, key; };

template <class Set> void run(std::string_view bits, const std::vector<Query> &queries) {
	auto start = std::chrono::steady_clock::now();
	Set set;
	if constexpr (requires { set.assign(bits); }) set.assign(bits);
	else {
		set.assign(bits.size());
		for (int i = 0; i < int(bits.size()); ++i) if (bits[i] == '1') set.insert(i);
	}
	auto built = std::chrono::steady_clock::now();
	std::uint64_t checksum = 0;
	for (auto [type, key] : queries) {
		int answer;
		if (type == 0) answer = set.insert(key);
		else if (type == 1) answer = set.erase(key);
		else if (type == 2) answer = set.contains(key);
		else if (type == 3) answer = set.next(key);
		else answer = set.prev(key);
		checksum = checksum * 1000000007 + std::uint64_t(answer);
	}
	auto end = std::chrono::steady_clock::now();
	std::printf("{\"build_ms\":%.6f,\"query_ms\":%.6f,\"checksum\":\"%llu\",\"count\":%d,\"bytes\":%zu,\"object_bytes\":%zu}\n",
		std::chrono::duration<double, std::milli>(built - start).count(), std::chrono::duration<double, std::milli>(end - built).count(),
		(unsigned long long)checksum, set.size(), set.memory_usage(), sizeof(Set));
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	int variant = std::atoi(argv[1]), pattern = std::atoi(argv[2]);
	int n = 10000000, q = 1000000;
	std::mt19937_64 rng(403023);
	std::string bits(n, '0');
	if (pattern == 0) for (char &c : bits) c += rng() & 1;
	else if (pattern == 1) std::fill(bits.begin(), bits.end(), '1');
	else if (pattern == 3) for (int i = 0; i < 16; ++i) bits[rng() % n] = '1';
	std::vector<Query> queries;
	queries.reserve(q);
	for (int i = 0; i < q; ++i) queries.push_back({pattern < 2 ? int(rng() % 5) : 2 + int(rng() % 3), int(rng() % n)});
	if (variant == 0) run<BeforePredecessorSet>(bits, queries);
	else if (variant == 1) run<PredecessorSet>(bits, queries);
	else run<ScalarPredecessorSet>(bits, queries);
}
