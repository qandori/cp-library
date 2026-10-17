#include "convolution/online_convolution.hpp"

int main() {
	OnlineConvolution<> queue;
	const int n = queue.max_size();
	for (int i = 0; i < n; ++i) {
		if (queue.pending() != mint(i ? i - 1 : 0)) return 1;
		if (queue.push(1, 1) != mint(i + 1)) return 2;
	}
	if (queue.size() != n || queue[n - 1] != mint(n)) return 3;
	queue.clear();
	if (!queue.empty() || queue.pending() != mint(0) || queue.push(2, 3) != mint(6)) return 4;
}
