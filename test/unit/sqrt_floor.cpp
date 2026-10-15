#include "math/sqrt_floor.hpp"
#include <cassert>
#include <random>
#include <vector>

using namespace std;

void check(long n, long m, long a, long b) {
	auto [dx, dy, segments] = sqrt_floor(n, m, a, b);
	if (!n) {
		assert(dx == 0 && dy == 0 && segments.empty());
		return;
	}
	vector<int> seen(n);
	for (auto [x, y, len] : segments) {
		assert(len > 0);
		for (long j = 0; j < len; ++j) {
			long i = x + dx * j;
			assert(0 <= i && i < n && ++seen[i] == 1);
			assert(y + dy * j == (a * i + b) % m);
		}
	}
	for (int count : seen) assert(count == 1);
}

int main() {
	for (int n = 0; n < 21; ++n)
		for (int m = 1; m < 20; ++m)
			for (int a = 0; a < m; ++a)
				for (int b = 0; b < m; ++b) check(n, m, a, b);
	mt19937_64 random(123);
	for (int repeat_index = 0; repeat_index < 1000; ++repeat_index) check(random() % 1000, 1 + random() % 1000000000, random() % 1000000000, random() % 1000000000);
}
