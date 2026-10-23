#ifndef MULTIVARIATE_CONVOLUTION_CORE_HPP
#define MULTIVARIATE_CONVOLUTION_CORE_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>
#include "mint_arithmetic.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_convolution_detail {
struct Core : mint_detail::Ops {
	static int modulus(int k) {
		for (int m : {1, 2, 4, 7, 8, 14, 16, 17, 28, 32})
			if (m >= k) return m;
		return int(std::bit_ceil(unsigned(k)));
	}
	static bool available() {
#if (defined(__x86_64__) || defined(__i386__)) && !defined(MULTIVARIATE_CONVOLUTION_DISABLE_AVX2)
		return vectorized();
#else
		return false;
#endif
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void row8(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int row, const std::vector<int> &parts) {
		const auto p = _mm256_set1_epi32(mod), r = _mm256_set1_epi32(998244351), two = _mm256_set1_epi32(twice);
		for (std::size_t i = 0; i < out.size(); i += 8) {
			auto sum = _mm256_setzero_si256();
			for (std::size_t j = 0; j < parts.size(); j += 8) {
				auto even = _mm256_setzero_si256(), odd = even;
				for (std::size_t k = j; k < std::min(j + 8, parts.size()); ++k) {
					auto x = load8(reinterpret_cast<const U *>(a[parts[k]].data() + i));
					auto y = load8(reinterpret_cast<const U *>(b[row - parts[k]].data() + i));
					x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
					y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
					even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
					odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
				}
				even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, r), p));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, r), p));
				auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
				value = _mm256_min_epu32(value, _mm256_sub_epi32(value, two));
				sum = add8(sum, value);
			}
			store8(reinterpret_cast<U *>(out.data() + i), sum);
		}
	}
#endif
	static void row(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int index, const std::vector<int> &parts) {
#if defined(__x86_64__) || defined(__i386__)
		if (out.size() >= 8 && available()) return row8(out, a, b, index, parts);
#endif
		std::fill(out.begin(), out.end(), mint(0));
		for (int p : parts)
			for (std::size_t i = 0; i < out.size(); ++i) out[i] += a[p][i] * b[index - p][i];
	}
	template <class Transform, class Product>
	static std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b, std::vector<int> dims, Transform transform, Product product) {
		int n = a.size();
		std::vector<int> order(dims.size());
		for (int j = 0; j < int(order.size()); ++j) order[j] = j;
		std::stable_sort(order.begin(), order.end(), [&](int x, int y) { return dims[x] > dims[y]; });
		std::vector<int> sorted;
		for (int j : order) sorted.push_back(dims[j]);
		int split = sorted.size(), inner = n, outer = 1;
		double best = 1e300, pairs = 1;
		for (int k = int(sorted.size()), q = 1, size = n; k; size /= sorted[--k], q *= sorted[k]) {
			int length = std::bit_ceil(unsigned(2 * size - 1));
			double cost = modulus(k) * length * (3.0 * q * std::bit_width(unsigned(length)) + pairs * 5.0);
			if (cost < best) best = cost, split = k, inner = size, outer = q;
			pairs *= sorted[k - 1] * (sorted[k - 1] + 1.0) / 2;
			if (q > 128 / sorted[k - 1]) break;
		}
		std::vector<int> target(dims.size());
		for (int j = 0, s = 1; j < int(dims.size()); s *= sorted[j++]) target[order[j]] = s;
		std::vector<int> permutation(n), digit(dims.size());
		for (int i = 0, p = 0; i < n; ++i) {
			permutation[i] = p;
			for (int j = 0; j < int(dims.size()); ++j) {
				p += target[j];
				if (++digit[j] < dims[j]) break;
				digit[j] = 0, p -= target[j] * dims[j];
			}
		}
		std::vector<mint> f(n), g(n), result(n), output(n);
		for (int i = 0; i < n; ++i) f[permutation[i]] = a[i], g[permutation[i]] = b[i];
		int m = modulus(split), length = std::bit_ceil(unsigned(2 * inner - 1));
		std::vector<unsigned char> ranks(inner);
		for (int base = sorted[0], j = 1; j < split; base *= sorted[j++])
			for (int i = 0; i < inner; ++i) ranks[i] = (ranks[i] + i / base) % m;
		std::vector<std::vector<int>> parts(outer);
		for (int e = 0; e < outer; ++e)
			for (int x = 0; x <= e; ++x) {
				bool valid = true;
				for (int j = split, u = x, v = e; j < int(sorted.size()); u /= sorted[j], v /= sorted[j++])
					if (u % sorted[j] > v % sorted[j]) { valid = false; break; }
				if (valid) parts[e].push_back(x);
			}
		std::vector<std::vector<mint>> x(outer, std::vector<mint>(outer == 1 ? inner : length)), y = x;
		std::vector<mint> z(length), weights(m), inverse(m);
		mint root = mint(3).pow((mod - 1) / m), scale = mint(m).inv();
		for (int s = 0; s < m; ++s) {
			mint step = root.pow(s), back = step.inv();
			weights[0] = 1, inverse[0] = scale;
			for (int j = 1; j < m; ++j) weights[j] = weights[j - 1] * step, inverse[j] = inverse[j - 1] * back;
			for (int row = 0; row < outer; ++row) {
				std::fill(x[row].begin() + inner, x[row].end(), mint(0));
				std::fill(y[row].begin() + inner, y[row].end(), mint(0));
				for (int i = 0; i < inner; ++i) x[row][i] = f[row * inner + i] * weights[ranks[i]], y[row][i] = g[row * inner + i] * weights[ranks[i]];
				if (outer != 1) transform(x[row], false), transform(y[row], false);
			}
			for (int row = outer; row-- > 0;) {
				if (outer == 1) z = product(x[0], y[0]);
				else Core::row(z, x, y, row, parts[row]), transform(z, true);
				for (int i = 0; i < inner; ++i) result[row * inner + i] += z[i] * inverse[ranks[i]];
			}
		}
		for (int i = 0; i < n; ++i) output[i] = result[permutation[i]];
		return output;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
