#ifndef GCD_LCM_CONVOLUTION_HPP
#define GCD_LCM_CONVOLUTION_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace gcd_lcm_convolution_detail {
inline std::vector<std::size_t> primes(std::size_t n) {
	std::vector<bool> composite(n);
	std::vector<std::size_t> result;
	for (std::size_t p = 2; p < n; ++p) {
		if (composite[p]) continue;
		result.push_back(p);
		if (p > (n - 1) / p) continue;
		for (std::size_t j = p * p; j < n; j += p) composite[j] = true;
	}
	return result;
}
template <bool inverse, bool multiples, class T>
void transform(std::vector<T> &a, const std::vector<std::size_t> &ps) {
	for (std::size_t p : ps) {
		std::size_t end = (a.size() - 1) / p;
		for (std::size_t step = 1; step <= end; ++step) {
			std::size_t i = inverse == multiples ? step : end + 1 - step;
			T &to = a[multiples ? i : i * p];
			const T &from = a[multiples ? i * p : i];
			if constexpr (inverse) to -= from;
			else to += from;
		}
	}
}
template <bool gcd, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	assert(a[0] == T(0) && b[0] == T(0));
	std::size_t n = std::max(a.size(), b.size());
	a.resize(n, T(0));
	b.resize(n, T(0));
	auto ps = primes(n);
	transform<false, gcd>(a, ps);
	transform<false, gcd>(b, ps);
	for (std::size_t i = 1; i < n; ++i) a[i] *= b[i];
	transform<true, gcd>(a, ps);
	return a;
}
}

template <class T>
std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<true>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<false>(std::move(a), std::move(b));
}

#endif
