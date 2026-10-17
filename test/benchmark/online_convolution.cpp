#include "components/fast_io.hpp"
#include <string_view>
#include <vector>

#if defined(BENCH_HITONA)
#include ".verify-helper/cache/online_convolution_reference/hitonanode/convolution/relaxed_multiplication.hpp"
using M = ModInt<998244353>;
struct Queue : relaxed_multiplication<M> {
	explicit Queue(int) {}
	M push(M a, M b) { return add(a, b); }
};
#else
#ifndef BENCH_ONLINE_HEADER
#define BENCH_ONLINE_HEADER "convolution/online_convolution.hpp"
#endif
#include BENCH_ONLINE_HEADER
using M = mint;
using Queue = OnlineConvolution<M>;
#endif

int main(int argc, char **argv) {
	FastInput in;
	FastOutput out;
	int n;
	if (!in.read(n)) return 0;
	std::vector<M> a(n), b(n);
	unsigned x;
	for (auto &v : a) { in.read(x); v = M(x); }
	bool exponential = argc > 1 && std::string_view(argv[1]) == "exp";
	bool feedback = argc > 1 && std::string_view(argv[1]) == "feedback";
	if (!exponential) for (auto &v : b) { in.read(x); v = M(x); }
	Queue queue(n);
	if (exponential) {
		std::vector<M> inverse(n, M(1));
		for (int i = 2; i < n; ++i) inverse[i] = -M(M::mod() / i) * inverse[M::mod() % i];
		M value = 1;
		for (int i = 0; i < n; ++i) {
			if (i) out.write(' ');
			out.write(value.val());
			if (i + 1 < n) value = queue.push(a[i + 1] * (i + 1), value) * inverse[i + 1];
		}
	} else {
		M last = 0;
		for (int i = 0; i < n; ++i) {
			if (i) out.write(' ');
			M x = a[i], y = b[i];
			if (feedback) { x += last; y += last * last; }
			last = queue.push(x, y);
			out.write(last.val());
		}
	}
	out.write('\n');
}
