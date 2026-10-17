#ifndef SUBSET_CONVOLUTION_HPP
#define SUBSET_CONVOLUTION_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class T>
std::vector<T> subset_convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	int levels = std::bit_width(n) - 1, width = levels + 1;
	std::vector<T> x(n * width, T(0)), y(n * width, T(0)), result(n, T(0));
	for (std::size_t i = 0; i < a.size(); ++i) x[i * width + std::popcount(i)] = a[i];
	for (std::size_t i = 0; i < b.size(); ++i) y[i * width + std::popcount(i)] = b[i];
	auto zeta = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start);
				for (std::size_t j = start; j < start + half; ++j)
					for (int k = low, end = std::popcount(j); k <= end; ++k) {
						x[(j + half) * width + k] += x[j * width + k];
						y[(j + half) * width + k] += y[j * width + k];
					}
			}
	};
	std::size_t block = std::min<std::size_t>(n, 256);
	for (std::size_t start = 0; start < n; start += block) zeta(start, block, 1);
	zeta(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) {
		int count = std::popcount(mask);
		for (int k = std::min(levels, 2 * count); k >= count; --k) {
			T sum(0);
			for (int i = k - count; i <= count; ++i)
				sum += x[mask * width + i] * y[mask * width + k - i];
			x[mask * width + k] = sum;
		}
	}
	auto mobius = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start) + 1, high = levels - std::countr_zero(half);
				for (std::size_t j = start; j < start + half; ++j) {
					int count = std::popcount(j - start);
					for (int k = low + count; k <= high + count; ++k)
						x[(j + half) * width + k] -= x[j * width + k];
				}
			}
	};
	for (std::size_t start = 0; start < n; start += block) mobius(start, block, 1);
	mobius(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) result[mask] = x[mask * width + std::popcount(mask)];
	return result;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
