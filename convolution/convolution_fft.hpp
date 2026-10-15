#ifndef CONVOLUTION_FFT_HPP
#define CONVOLUTION_FFT_HPP

#include <algorithm>
#include <bit>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <type_traits>
#include <vector>

namespace convolution_fft_detail {
template <class Real>
void transform(std::vector<std::complex<Real>> &a, bool inverse) {
	std::size_t n = a.size();
	for (std::size_t i = 1, j = 0; i < n; ++i) {
		std::size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	for (std::size_t width = 2; width <= n; width *= 2) {
		Real angle = (inverse ? -2 : 2) * std::numbers::pi_v<Real> / Real(width);
		std::complex<Real> step(std::cos(angle), std::sin(angle));
		for (std::size_t start = 0; start < n; start += width) {
			std::complex<Real> w(1);
			for (std::size_t j = 0; j < width / 2; ++j, w *= step) {
				auto x = a[start + j], y = a[start + j + width / 2] * w;
				a[start + j] = x + y;
				a[start + j + width / 2] = x - y;
			}
		}
	}
	if (inverse)
		for (auto &x : a) x /= Real(n);
}
}

template <class Real>
std::vector<std::complex<Real>> convolution_fft(const std::vector<std::complex<Real>> &a, const std::vector<std::complex<Real>> &b) {
	if (a.empty() || b.empty()) return {};
	std::size_t length = a.size() + b.size() - 1, n = std::bit_ceil(length);
	std::vector<std::complex<Real>> x(a.begin(), a.end()), y(b.begin(), b.end());
	x.resize(n);
	y.resize(n);
	convolution_fft_detail::transform(x, false);
	if (&a == &b) y = x;
	else convolution_fft_detail::transform(y, false);
	for (std::size_t i = 0; i < n; ++i) x[i] *= y[i];
	convolution_fft_detail::transform(x, true);
	x.resize(length);
	return x;
}
template <class Real>
	requires std::is_floating_point_v<Real>
std::vector<Real> convolution_fft(const std::vector<Real> &a, const std::vector<Real> &b) {
	std::vector<std::complex<Real>> x(a.begin(), a.end()), y(b.begin(), b.end());
	auto values = &a == &b ? convolution_fft(x, x) : convolution_fft(x, y);
	std::vector<Real> result;
	result.reserve(values.size());
	for (const auto &x : values) result.push_back(x.real());
	return result;
}

#endif
