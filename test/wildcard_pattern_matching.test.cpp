#define PROBLEM "https://judge.yosupo.jp/problem/wildcard_pattern_matching"
#include "../string/wildcard_pattern_matching.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string text, pattern;
	in.read(text);
	in.read(pattern);
	for (bool matched : wildcard_pattern_matching(text, pattern)) out.write(matched ? '1' : '0');
	out.write('\n');
}
