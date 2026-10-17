#include "convolution/online_convolution.hpp"
#include "fps/fps.hpp"
#include <cassert>
#include <random>
#include <utility>
#include <vector>

struct OtherMint {
	unsigned value;
	OtherMint(unsigned x = 0) : value(x % mod()) {}
	static constexpr unsigned mod() { return 998244353; }
	static OtherMint raw(unsigned x) { return OtherMint(x); }
	unsigned val() const { return value; }
};

void check(const fps &a, const fps &b, bool bounded) {
	int n = a.size();
	OnlineConvolution queue(bounded ? n : 1 << 23);
	queue.reserve(n);
	auto expected = fps::multiply(a, b);
	for (int i = 0; i < n; ++i) {
		mint pending = 0;
		for (int j = 1; j < i; ++j) pending += a[j] * b[i - j];
		assert(queue.size() == i && queue.empty() == (i == 0));
		assert(queue.pending() == pending);
		assert(queue.push(a[i], b[i]) == expected[i]);
		assert(queue[i] == expected[i]);
	}
	for (int i = 0; i < n; ++i) assert(queue[i] == expected[i]);
	queue.clear();
	assert(queue.empty() && queue.size() == 0 && queue.max_size() == (bounded ? n : 1 << 23));
	if (n) assert(queue.pending() == 0 && queue.push(3, 4) == 12);
}

int main() {
	std::mt19937_64 rng(682341);
	OnlineConvolution<> empty(0);
	empty.reserve(0); empty.clear();
	assert(empty.empty() && empty.max_size() == 0);
	for (int n = 0; n <= 130; ++n) {
		fps a(n), b(n);
		for (int i = 0; i < n; ++i) { a[i] = rng(); b[i] = rng(); }
		check(a, b, true); check(a, b, false);
	}
	for (int n : {255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 2049}) {
		fps a(n), b(n);
		for (int i = 0; i < n; ++i) { a[i] = rng(); b[i] = rng(); }
		check(a, b, true);
		check(fps(n, mint(-1)), fps(n, mint(-1)), false);
		check(fps(n, mint(0)), b, true);
	}
	for (int n : {32767, 32768, 32769, 131073}) {
		fps a(n), b(n);
		for (int i = 0; i < n; ++i) { a[i] = rng(); b[i] = rng(); }
		auto expected = fps::multiply(a, b);
		OnlineConvolution queue(n);
		for (int i = 0; i < n; ++i) {
			mint interior = i ? expected[i] - a[i] * b[0] - a[0] * b[i] : mint(0);
			assert(queue.pending() == interior);
			assert(queue.push(a[i], b[i]) == expected[i]);
		}
	}
	OnlineConvolution<> queue(512);
	std::vector<mint> a, b;
	for (int i = 0; i < 512; ++i) {
		mint pending = 0;
		for (int j = 1; j < i; ++j) pending += a[j] * b[i - j];
		assert(queue.pending() == pending);
		mint x = pending + i + 1, y = pending * pending - i;
		a.push_back(x); b.push_back(y);
		mint expected = 0;
		for (int j = 0; j <= i; ++j) expected += a[j] * b[i - j];
		assert(queue.push(x, y) == expected);
	}
	OnlineConvolution<> copy = queue;
	queue.clear();
	for (int i = 0; i < 512; ++i) assert(copy[i] == [&] {
		mint value = 0;
		for (int j = 0; j <= i; ++j) value += a[j] * b[i - j];
		return value;
	}());
	OnlineConvolution<> prefix;
	for (int i = 0; i < 100; ++i) prefix.push(1, 1);
	auto branch = prefix;
	assert(branch.push(2, 3) == 104 && prefix.push(1, 1) == 101);
	auto moved = std::move(branch);
	assert(moved[100] == 104 && moved.push(0, 0) == 103);
	OnlineConvolution<OtherMint> other;
	for (int i = 0; i < 1000; ++i) assert(other.push(1, 1).val() == unsigned(i + 1));
	assert(other.pending().val() == 999);
}
