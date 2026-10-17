#ifndef CP_COMPONENTS_POLYNOMIAL_HALF_GCD_HPP
#define CP_COMPONENTS_POLYNOMIAL_HALF_GCD_HPP

#include "../fps/fps.hpp"
#include <array>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct PolynomialHalfGCD {
	using T = Mint;
	using F = FormalPowerSeries<T>;
	using O = Operations<T>;
	struct Matrix {
		std::array<F, 4> a{F{1}, F{}, F{}, F{1}};
		int shift = 0;
		bool identity = true;
	};
	static F high(const F& a, int k) {
		return k >= int(a.size()) ? F{} : F(a.begin() + k, a.end());
	}
	static F fold(const F& a, int n) {
		if (int(a.size()) <= n) { F result = a; result.resize(n); return result; }
		F result(n);
		for (int i = 0; i < int(a.size()); ++i) result[i & (n - 1)] += a[i];
		return result;
	}
	static F product_sum(const F& a, const F& b, const F& c, const F& d, int n) {
		F result(n), temporary(n);
		O::pointwise(result, a, b, n), O::pointwise(temporary, c, d, n);
		result += temporary;
		O::intt(result), O::trim(result);
		return result;
	}
	static Matrix multiply(const Matrix& a, const Matrix& b) {
		if (a.identity) return b;
		if (b.identity) return a;
		Matrix result;
		result.shift = a.shift + b.shift, result.identity = false;
		if (result.shift <= 64) {
			for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) {
				result.a[2 * i + j] = a.a[2 * i] * b.a[j] + a.a[2 * i + 1] * b.a[2 + j];
				O::trim(result.a[2 * i + j]);
			}
			return result;
		}
		int n = 1;
		while (n <= result.shift) n <<= 1;
		std::array<F, 4> x, y;
		for (int i = 0; i < 4; ++i) {
			x[i] = fold(a.a[i], n), y[i] = fold(b.a[i], n);
			O::ntt(x[i]), O::ntt(y[i]);
		}
		for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j)
			result.a[2 * i + j] = product_sum(x[2 * i], y[j], x[2 * i + 1], y[2 + j], n);
		return result;
	}
	static void step(Matrix& matrix, const F& q) {
		F c = matrix.a[0] - q * matrix.a[2], d = matrix.a[1] - q * matrix.a[3];
		O::trim(c), O::trim(d);
		matrix.a[0] = std::move(matrix.a[2]), matrix.a[1] = std::move(matrix.a[3]);
		matrix.a[2] = std::move(c), matrix.a[3] = std::move(d);
		matrix.shift += std::max(0, int(q.size()) - 1), matrix.identity = false;
	}
	static void apply(const Matrix& matrix, F& a, F& b) {
		if (matrix.identity) return;
		if (a.size() <= 128) {
			F c = matrix.a[0] * a + matrix.a[1] * b;
			b = matrix.a[2] * a + matrix.a[3] * b, a = std::move(c);
			O::trim(a), O::trim(b);
			return;
		}
		int size = int(a.size()) - matrix.shift, n = 1;
		assert(size > 0);
		while (n < size) n <<= 1;
		F x = fold(a, n), y = fold(b, n);
		O::ntt(x), O::ntt(y);
		std::array<F, 4> m;
		for (int i = 0; i < 4; ++i) m[i] = fold(matrix.a[i], n), O::ntt(m[i]);
		a = product_sum(m[0], x, m[1], y, n);
		b = product_sum(m[2], x, m[3], y, n);
		assert(int(a.size()) == size && b.size() < a.size());
	}
	static Matrix half(F a, F b) {
		assert(a.size() > b.size());
		int m = int(a.size()) / 2;
		Matrix result;
		if (int(b.size()) <= m) return result;
		if (a.size() <= 128) {
			while (int(b.size()) > m) {
				auto [q, r] = a.divmod(b);
				step(result, q), a = std::move(b), b = std::move(r);
			}
			return result;
		}
		result = half(high(a, m), high(b, m));
		apply(result, a, b);
		if (int(b.size()) <= m) return result;
		auto [q, r] = a.divmod(b);
		step(result, q), a = std::move(b), b = std::move(r);
		if (int(b.size()) <= m) return result;
		int k = 2 * m + 1 - int(a.size());
		return multiply(half(high(a, k), high(b, k)), result);
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
