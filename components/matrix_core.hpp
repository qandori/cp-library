#ifndef MATRIX_CORE_HPP
#define MATRIX_CORE_HPP

#include "matrix_base_arithmetic.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <class T>
struct LinearSolution {
	std::vector<T> particular;
	std::vector<std::vector<T>> basis;
};

namespace matrix_detail {
template <class T> struct FastMatrixArithmetic;
template <class T> struct PlainMatrixArithmetic;
#ifdef CP_MATRIX_LIGHT_API
template <class T> using DefaultMatrixArithmetic = PlainMatrixArithmetic<T>;
#else
template <class T> using DefaultMatrixArithmetic = FastMatrixArithmetic<T>;
#endif
}

template <class T, class Arithmetic = matrix_detail::DefaultMatrixArithmetic<T>>
class [[nodiscard, gnu::warn_unused]] Matrix {
	static_assert(!std::is_same_v<T, bool>);
	int h = 0, w = 0;
	std::vector<T> a;
	std::pair<T, std::vector<int>> reduce(int columns, bool reduced, long double eps) {
		assert(eps >= 0);
		return Arithmetic::field(a, [&]<class Ops>(auto &work) {
			auto res = [&] {
				if constexpr (requires { Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps); })
					return Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps);
				else return matrix_detail::eliminate<Ops, Arithmetic::modular>(work, h, w, columns, reduced, eps);
			}();
			return std::pair{T(res.determinant), std::move(res.pivots)};
		});
	}
	static std::vector<std::vector<T>> nullspace(const Matrix &r, const std::vector<int> &piv, int vars) {
		std::vector<bool> pivot(vars);
		for (int p : piv) pivot[p] = true;
		std::vector<std::vector<T>> basis;
		basis.reserve(vars - piv.size());
		for (int col = 0; col < vars; ++col) if (!pivot[col]) {
			std::vector<T> v(vars, T(0));
			v[col] = T{1};
			for (int i = 0; i < int(piv.size()); ++i) v[piv[i]] = -r(i, col);
			basis.push_back(std::move(v));
		}
		return basis;
	}
	Matrix augment() const {
		static_assert(!matrix_detail::integer<T>);
		assert(h == w && w <= std::numeric_limits<int>::max() / 2);
		Matrix work(h, w * 2);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w + i) = T{1};
		}
		return work;
	}

	std::pair<Matrix, std::vector<int>> echelon(bool reduced, long double eps) const {
		Matrix work = *this;
		auto piv = work.reduce(w, reduced, eps).second;
		return {std::move(work), std::move(piv)};
	}

  public:
	using value_type = T;
	using Solution = LinearSolution<T>;
	static constexpr long double default_epsilon = std::is_floating_point_v<T> ? 1e-12L : 0;
	Matrix() = default;
	explicit Matrix(int n) : Matrix(n, n) {}
	Matrix(int rows, int columns, T value = T(0)) : h(rows), w(columns) {
		assert(rows >= 0 && columns >= 0);
		a.assign(std::size_t(h) * w, value);
	}
	Matrix(std::initializer_list<std::initializer_list<T>> values) : Matrix(int(values.size()), values.size() ? int(values.begin()->size()) : 0) {
		int i = 0;
		for (const auto &row : values) {
			assert(int(row.size()) == w);
			std::copy(row.begin(), row.end(), (*this)[i++].begin());
		}
	}
	[[nodiscard]] explicit Matrix(const std::vector<std::vector<T>> &values) : Matrix(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h; ++i) {
			assert(int(values[i].size()) == w);
			std::copy(values[i].begin(), values[i].end(), (*this)[i].begin());
		}
	}
	Matrix(const Matrix &) = default;
	Matrix &operator=(const Matrix &) = default;
	Matrix(Matrix &&other) noexcept { swap(other); }
	Matrix &operator=(Matrix &&other) noexcept {
		if (this != &other) { Matrix next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(Matrix &other) noexcept { std::swap(h, other.h); std::swap(w, other.w); a.swap(other.a); }
	[[nodiscard]] int rows() const { return h; }
	[[nodiscard]] int cols() const { return w; }
	[[nodiscard]] bool empty() const { return !h || !w; }
	[[nodiscard]] T *data() { return a.data(); }
	[[nodiscard]] const T *data() const { return a.data(); }
	[[nodiscard]] std::span<T> operator[](int i) { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	[[nodiscard]] std::span<const T> operator[](int i) const { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	[[nodiscard]] T &operator()(int i, int j) { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	[[nodiscard]] const T &operator()(int i, int j) const { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	[[nodiscard]] bool operator==(const Matrix &) const = default;
	static Matrix identity(int n) {
		Matrix res(n);
		for (int i = 0; i < n; ++i) res(i, i) = T{1};
		return res;
	}
	void fill(T value) { std::fill(a.begin(), a.end(), value); }
	Matrix transpose() const {
		Matrix res(w, h);
		for (int i = 0; i < h; i += 32) for (int j = 0; j < w; j += 32)
			for (int x = i; x < std::min(h, i + 32); ++x) for (int y = j; y < std::min(w, j + 32); ++y) res(y, x) = (*this)(x, y);
		return res;
	}
	Matrix submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h && 0 <= col_l && col_l <= col_r && col_r <= w);
		Matrix res(row_r - row_l, col_r - col_l);
		for (int i = row_l; i < row_r; ++i) std::copy((*this)[i].begin() + col_l, (*this)[i].begin() + col_r, res[i - row_l].begin());
		return res;
	}
	Matrix operator-() const { Matrix res = *this; for (auto &x : res.a) x = -x; return res; }
	Matrix &operator+=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] += b.a[i]; return *this; }
	Matrix &operator-=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] -= b.a[i]; return *this; }
	Matrix &operator*=(const Matrix &b) { return *this = *this * b; }
	Matrix &operator*=(T x) { for (auto &v : a) v *= x; return *this; }
	Matrix &operator/=(T x) { static_assert(!matrix_detail::integer<T>); assert(x != T(0)); return *this *= T{1} / x; }
	friend Matrix operator+(Matrix a, const Matrix &b) { a += b; return a; }
	friend Matrix operator-(Matrix a, const Matrix &b) { a -= b; return a; }
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		assert(a.w == b.h);
		Matrix res;
		res.h = a.h; res.w = b.w;
		res.a = Arithmetic::multiply(a.a, b.a, a.h, a.w, b.w);
		return res;
	}
	friend Matrix operator*(Matrix a, T x) { a *= x; return a; }
	friend Matrix operator*(T x, Matrix a) { a *= x; return a; }
	friend Matrix operator/(Matrix a, T x) { a /= x; return a; }
	[[nodiscard]] std::vector<T> operator*(const std::vector<T> &v) const {
		assert(int(v.size()) == w);
		return Arithmetic::multiply(a, v, h, w, 1);
	}
	Matrix pow(std::uint64_t exponent) const {
		assert(h == w);
		if (auto power = Arithmetic::power(a, h, exponent)) {
			Matrix res(h);
			res.a = std::move(*power);
			return res;
		}
		return pow(exponent, [](Matrix &) {});
	}
	template <class Normalize>
	Matrix pow(std::uint64_t exponent, Normalize normalize) const {
		assert(h == w);
		Matrix res = identity(h), x = *this;
		normalize(res);
		normalize(x);
		while (exponent) {
			if (exponent & 1) {
				res *= x;
				normalize(res);
			}
			exponent >>= 1;
			if (exponent) {
				x *= x;
				normalize(x);
			}
		}
		return res;
	}
	[[nodiscard]] T trace() const { assert(h == w); T res = T(0); for (int i = 0; i < h; ++i) res += (*this)(i, i); return res; }
	[[nodiscard]] T det(long double eps = default_epsilon) const {
		assert(h == w);
		if constexpr (matrix_detail::integer<T>) {
			return matrix_detail::integer_determinant(a, h);
		} else {
			Matrix work = *this;
			auto [value, piv] = work.reduce(w, false, eps);
			return int(piv.size()) == h ? value : T(0);
		}
	}
	[[nodiscard]] int rank(long double eps = default_epsilon) const { Matrix work = *this; return int(work.reduce(w, false, eps).second.size()); }
	[[nodiscard]] std::pair<Matrix, std::vector<int>> row_echelon(long double eps = default_epsilon) const { return echelon(false, eps); }
	[[nodiscard]] std::pair<Matrix, std::vector<int>> rref(long double eps = default_epsilon) const { return echelon(true, eps); }

	[[nodiscard]] std::optional<Matrix> inverse(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::inverse(a, h, false); }) if (auto fast = Arithmetic::inverse(a, h, false)) {
			if (fast->second.empty()) return std::nullopt;
			Matrix res(h);
			res.a = std::move(fast->second);
			return res;
		}
		auto work = augment();
		auto piv = work.reduce(w, true, eps).second;
		if (int(piv.size()) < h) return std::nullopt;
		return work.submatrix(0, h, w, w * 2);
	}
	[[nodiscard]] std::vector<std::vector<T>> kernel(long double eps = default_epsilon) const {
		auto [work, piv] = rref(eps);
		return nullspace(work, piv, w);
	}
	[[nodiscard]] std::optional<LinearSolution<T>> solve(const std::vector<T> &b, long double eps = default_epsilon) const {
		assert(int(b.size()) == h && w < std::numeric_limits<int>::max());
		Matrix work(h, w + 1);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w) = b[i];
		}
		auto piv = work.reduce(w, true, eps).second;
		for (int i = int(piv.size()); i < h; ++i) if (!matrix_detail::Field<T>::zero(work(i, w), eps)) return std::nullopt;
		LinearSolution<T> res{std::vector<T>(w, T(0)), nullspace(work, piv, w)};
		for (int i = 0; i < int(piv.size()); ++i) res.particular[piv[i]] = work(i, w);
		return res;
	}
	[[nodiscard]] std::vector<T> characteristic_polynomial(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::characteristic(a, h); })
			if (auto res = Arithmetic::characteristic(a, h)) return std::move(*res);
		auto work = a;
		return Arithmetic::field(work, [&]<class Ops>(auto &b) {
			using V = typename Ops::V;
			auto at = [&](int i, int j) -> V& { return b[std::size_t(i) * w + j]; };
			std::vector<V> factors;
			for (int k = 1; k < h; ++k) {
				int pivot = -1;
				for (int i = k; i < h; ++i) if (!Ops::zero(at(i, k - 1), eps)
					&& (pivot < 0 || Ops::better(at(i, k - 1), at(pivot, k - 1)))) pivot = i;
				if (pivot < 0) continue;
				if (pivot != k) {
					for (int j = 0; j < w; ++j) std::swap(at(k, j), at(pivot, j));
					for (int i = 0; i < h; ++i) std::swap(at(i, k), at(i, pivot));
				}
				V inverse = Ops::inv(at(k, k - 1));
				if constexpr (requires { Arithmetic::template hessenberg<Ops>(b, h, k, inverse, factors); })
					if (Arithmetic::template hessenberg<Ops>(b, h, k, inverse, factors)) continue;

				for (int i = k + 1; i < h; ++i) {
					V factor = Ops::mul(at(i, k - 1), inverse);
					if (Ops::zero(factor, eps)) continue;
					Ops::axpy(b.data() + std::size_t(i) * w + k, b.data() + std::size_t(k) * w + k, w - k, Ops::neg(factor));
					at(i, k - 1) = V(0);
					for (int j = 0; j < h; ++j) at(j, k) = Ops::add(at(j, k), Ops::mul(factor, at(j, i)));
				}
			}
			std::vector<std::vector<V>> p(h + 1);
			p[0] = {V(1)};
			for (int n = 1; n <= h; ++n) {
				p[n].assign(n + 1, V(0));
				std::copy(p[n - 1].begin(), p[n - 1].end(), p[n].begin() + 1);
				Ops::axpy(p[n].data(), p[n - 1].data(), n, Ops::neg(at(n - 1, n - 1)));
				V product = 1;
				for (int i = n - 2; i >= 0; --i) {
					product = Ops::mul(product, at(i + 1, i));
					V factor = Ops::mul(product, at(i, n - 1));
					Ops::axpy(p[n].data(), p[i].data(), i + 1, Ops::neg(factor));
				}
			}
			std::vector<T> res(h + 1);
			for (int i = 0; i <= h; ++i) res[i] = T(p[h][i]);
			return res;
		});
	}
	Matrix adjugate(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::inverse(a, h, true); }) if (auto fast = Arithmetic::inverse(a, h, true)) {
			Matrix res(h);
			res.a = std::move(fast->second);
			return res;
		}
		auto work = augment();
		auto [det, piv] = work.reduce(w, true, eps);
		if (int(piv.size()) == h) return work.submatrix(0, h, w, w * 2) * det;
		Matrix res(h);
		if (int(piv.size()) + 1 < h) return res;
		auto right = nullspace(work, piv, w)[0];
		int col = 0;
		while (std::find(piv.begin(), piv.end(), col) != piv.end()) ++col;
		if ((h - 1 - col) & 1) det = -det;
		for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) res(i, j) = det * right[i] * work(h - 1, w + j);
		return res;
	}
};

#endif
