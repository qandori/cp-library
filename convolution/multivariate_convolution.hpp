#ifndef MULTIVARIATE_CONVOLUTION_HPP
#define MULTIVARIATE_CONVOLUTION_HPP

#include <cassert>
#include <cstddef>
#include <vector>
#include "../components/multivariate_convolution_core.hpp"
#include "../components/multivariate_ranked.hpp"

#include "convolution.hpp"
#include "../components/ntt.hpp"

template <class T>
std::vector<T> multivariate_convolution(const std::vector<T> &a, const std::vector<T> &b, const std::vector<int> &dimensions) {
	std::size_t size = 1;
	std::vector<std::size_t> bases;
	for (int d : dimensions) {
		assert(d > 0 && size <= std::size_t(-1) / d);
		if (d == 1) continue;
		size *= d;
		bases.push_back(size);
	}
	assert(a.size() == size && b.size() == size);
	if (bases.empty()) return {a[0] * b[0]};
	if (bases.size() == 1) {
		auto result = convolution(a, b);
		result.resize(size);
		return result;
	}
	int colors = bases.size();

	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			assert(size <= std::size_t(1) << 22);
			std::vector<mint> x(size), y(size);
			std::vector<int> dims;
			for (int d : dimensions) if (d > 1) dims.push_back(d);
			for (std::size_t i = 0; i < size; ++i) x[i] = mint::raw(a[i].val()), y[i] = mint::raw(b[i].val());
			std::unique_ptr<fps_detail::NTT> ntt;
			auto transform = [&](std::vector<mint> &v, bool inverse) {
				using NTT = fps_detail::NTT;
				if (!ntt) ntt = std::make_unique<NTT>(v.size() / 2, false);
				auto *p = reinterpret_cast<NTT::U *>(v.data());
				if (inverse) ntt->fft<true>(p, v.size()), ntt->scale(p, v.size(), 998244353 - 998244352 / v.size());
				else ntt->fft<false>(p, v.size());
			};
			auto product = [&] {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
				int degree = 0;
				bool small = true;
				for (int d : dims) degree += d - 1, small &= d <= 3;
				if (small && degree < 24 && std::count(dims.begin(), dims.end(), 2) >= 3 && multivariate_convolution_detail::Core::available())
					return multivariate_ranked_detail::multiply(x, y, dims);
#endif
				return multivariate_convolution_detail::Core::multiply(x, y, dims, transform, convolution<mint>);
			}();
			std::vector<T> result;
			result.reserve(size);
			for (auto v : product) result.push_back(T::raw(v.val()));
			return result;
		}
	}
	std::vector<int> rank(size);
	for (std::size_t i = 0; i < size; ++i) {
		std::size_t value = 0;
		for (std::size_t base : bases) value += i / base;
		rank[i] = value % colors;
	}
	std::size_t stride = 2 * colors - 1;
	assert(size <= std::size_t(-1) / stride / 2);
	std::vector<T> x(size * stride, T(0)), y(size * stride, T(0));
	for (std::size_t i = 0; i < size; ++i) x[i * stride + rank[i]] = a[i], y[i * stride + rank[i]] = b[i];
	auto product = convolution(x, y);
	std::vector<T> result(size, T(0));
	for (std::size_t i = 0; i < size; ++i) {
		result[i] = product[i * stride + rank[i]];
		if (rank[i] + colors < int(stride)) result[i] += product[i * stride + rank[i] + colors];
	}
	return result;
}

#endif
