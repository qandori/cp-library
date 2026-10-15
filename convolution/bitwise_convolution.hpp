#ifndef BITWISE_CONVOLUTION_HPP
#define BITWISE_CONVOLUTION_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <bool inverse = false, bool superset = false, class T>
void bitwise_zeta(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	for (std::size_t half = 1; half < a.size(); half *= 2)
		for (std::size_t start = 0; start < a.size(); start += 2 * half)
			for (std::size_t j = 0; j < half; ++j) {
				T &to = a[start + j + (superset ? 0 : half)];
				const T &from = a[start + j + (superset ? half : 0)];
				if constexpr (inverse) to -= from;
				else to += from;
			}
}

template <class T>
void walsh_hadamard(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	for (std::size_t half = 1; half < a.size(); half *= 2)
		for (std::size_t start = 0; start < a.size(); start += 2 * half)
			for (std::size_t j = 0; j < half; ++j) {
				T x = a[start + j], y = a[start + j + half];
				a[start + j] = x + y;
				a[start + j + half] = x - y;
			}
}

namespace bitwise_convolution_detail {
template <int kind, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	a.resize(n, T(0));
	b.resize(n, T(0));
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		walsh_hadamard(b);
	} else {
		bitwise_zeta<false, kind == 0>(a);
		bitwise_zeta<false, kind == 0>(b);
	}
	for (std::size_t i = 0; i < n; ++i) a[i] *= b[i];
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		if constexpr (std::numeric_limits<T>::is_integer)
			for (T &x : a) x /= std::int64_t(n);
		else {
			T inverse = T(1) / T(n);
			for (T &x : a) x *= inverse;
		}
	} else bitwise_zeta<true, kind == 0>(a);
	return a;
}
}

template <class T>
std::vector<T> and_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<0>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> or_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<1>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> xor_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<2>(std::move(a), std::move(b));
}

#endif
