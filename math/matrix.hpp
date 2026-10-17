#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "../components/matrix_arithmetic.hpp"
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

template <class T>
class Matrix {
	static_assert(!std::is_same_v<T, bool>);
	int h = 0, w = 0;
	std::vector<T> a;
	std::pair<T, std::vector<int>> reduce(int columns, bool reduced, long double eps) {
		assert(eps >= 0);
		return matrix_detail::field(a, [&]<class Ops>(auto &work) {
			auto result = matrix_detail::eliminate<Ops>(work, h, w, columns, reduced, eps);
			return std::pair{T(result.determinant), std::move(result.pivots)};
		});
	}
	static std::vector<std::vector<T>> nullspace(const Matrix &r, const std::vector<int> &pivots, int variables) {
		std::vector<bool> pivot(variables);
		for (int p : pivots) pivot[p] = true;
		std::vector<std::vector<T>> basis;
		basis.reserve(variables - pivots.size());
		for (int col = 0; col < variables; ++col) if (!pivot[col]) {
			std::vector<T> v(variables, T(0));
			v[col] = T(1);
			for (int i = 0; i < int(pivots.size()); ++i) v[pivots[i]] = -r(i, col);
			basis.push_back(std::move(v));
		}
		return basis;
	}
	Matrix augmented_identity() const {
		static_assert(!matrix_detail::integer<T>);
		assert(h == w && w <= std::numeric_limits<int>::max() / 2);
		Matrix work(h, w * 2);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w + i) = T(1);
		}
		return work;
	}
	std::pair<T, std::optional<Matrix>> invert(long double eps) const {
		auto work = augmented_identity();
		auto [det, pivots] = work.reduce(w, true, eps);
		if (int(pivots.size()) < h) return {T(0), std::nullopt};
		return {det, work.submatrix(0, h, w, w * 2)};
	}

  public:
	using value_type = T;
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
	explicit Matrix(const std::vector<std::vector<T>> &values) : Matrix(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
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
	int rows() const { return h; }
	int cols() const { return w; }
	bool empty() const { return !h || !w; }
	T *data() { return a.data(); }
	const T *data() const { return a.data(); }
	std::span<T> operator[](int i) { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	std::span<const T> operator[](int i) const { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	T &operator()(int i, int j) { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	const T &operator()(int i, int j) const { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	bool operator==(const Matrix &) const = default;
	static Matrix identity(int n) {
		Matrix result(n);
		for (int i = 0; i < n; ++i) result(i, i) = T(1);
		return result;
	}
	void fill(T value) { std::fill(a.begin(), a.end(), value); }
	Matrix transpose() const {
		Matrix result(w, h);
		for (int i = 0; i < h; i += 32) for (int j = 0; j < w; j += 32)
			for (int x = i; x < std::min(h, i + 32); ++x) for (int y = j; y < std::min(w, j + 32); ++y) result(y, x) = (*this)(x, y);
		return result;
	}
	Matrix submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h && 0 <= col_l && col_l <= col_r && col_r <= w);
		Matrix result(row_r - row_l, col_r - col_l);
		for (int i = row_l; i < row_r; ++i) std::copy((*this)[i].begin() + col_l, (*this)[i].begin() + col_r, result[i - row_l].begin());
		return result;
	}
	Matrix operator-() const { Matrix result = *this; for (auto &x : result.a) x = -x; return result; }
	Matrix &operator+=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] += b.a[i]; return *this; }
	Matrix &operator-=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] -= b.a[i]; return *this; }
	Matrix &operator*=(const Matrix &b) { return *this = *this * b; }
	Matrix &operator*=(T x) { for (auto &v : a) v *= x; return *this; }
	Matrix &operator/=(T x) { static_assert(!matrix_detail::integer<T>); assert(x != T(0)); return *this *= T(1) / x; }
	friend Matrix operator+(Matrix a, const Matrix &b) { return a += b; }
	friend Matrix operator-(Matrix a, const Matrix &b) { return a -= b; }
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		assert(a.w == b.h);
		Matrix result;
		result.h = a.h; result.w = b.w;
		result.a = matrix_detail::multiply(a.a, b.a, a.h, a.w, b.w);
		return result;
	}
	friend Matrix operator*(Matrix a, T x) { return a *= x; }
	friend Matrix operator*(T x, Matrix a) { return a *= x; }
	friend Matrix operator/(Matrix a, T x) { return a /= x; }
	std::vector<T> operator*(const std::vector<T> &v) const {
		assert(int(v.size()) == w);
		return matrix_detail::multiply(a, v, h, w, 1);
	}
	Matrix pow(std::uint64_t exponent) const {
		assert(h == w);
		Matrix result = identity(h), x = *this;
		while (exponent) {
			if (exponent & 1) result *= x;
			exponent >>= 1;
			if (exponent) x *= x;
		}
		return result;
	}
	T trace() const { assert(h == w); T result = 0; for (int i = 0; i < h; ++i) result += (*this)(i, i); return result; }
	T det(long double eps = default_epsilon) const {
		assert(h == w);
		if constexpr (matrix_detail::integer<T>) {
			return matrix_detail::integer_determinant(a, h);
		} else {
			Matrix work = *this;
			auto [value, pivots] = work.reduce(w, false, eps);
			return int(pivots.size()) == h ? value : T(0);
		}
	}
	int rank(long double eps = default_epsilon) const { Matrix work = *this; return int(work.reduce(w, false, eps).second.size()); }
	std::pair<Matrix, std::vector<int>> row_echelon(long double eps = default_epsilon) const {
		Matrix work = *this;
		auto pivots = work.reduce(w, false, eps).second;
		return {std::move(work), std::move(pivots)};
	}
	std::pair<Matrix, std::vector<int>> rref(long double eps = default_epsilon) const {
		Matrix work = *this;
		auto pivots = work.reduce(w, true, eps).second;
		return {std::move(work), std::move(pivots)};
	}
	std::optional<Matrix> inverse(long double eps = default_epsilon) const { return invert(eps).second; }
	std::vector<std::vector<T>> kernel(long double eps = default_epsilon) const {
		auto [work, pivots] = rref(eps);
		return nullspace(work, pivots, w);
	}
	std::optional<LinearSolution<T>> solve(const std::vector<T> &b, long double eps = default_epsilon) const {
		assert(int(b.size()) == h && w < std::numeric_limits<int>::max());
		Matrix work(h, w + 1);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w) = b[i];
		}
		auto pivots = work.reduce(w, true, eps).second;
		for (int i = int(pivots.size()); i < h; ++i) if (!matrix_detail::Field<T>::zero(work(i, w), eps)) return std::nullopt;
		LinearSolution<T> result{std::vector<T>(w, T(0)), nullspace(work, pivots, w)};
		for (int i = 0; i < int(pivots.size()); ++i) result.particular[pivots[i]] = work(i, w);
		return result;
	}
	std::vector<T> characteristic_polynomial(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		auto work = a;
		return matrix_detail::field(work, [&]<class Ops>(auto &b) {
			using V = typename Ops::V;
			for (int k = 1; k < h; ++k) {
				int pivot = -1;
				for (int i = k; i < h; ++i) if (!Ops::zero(b[std::size_t(i) * w + k - 1], eps)
					&& (pivot < 0 || Ops::better(b[std::size_t(i) * w + k - 1], b[std::size_t(pivot) * w + k - 1]))) pivot = i;
				if (pivot < 0) continue;
				if (pivot != k) {
					for (int j = 0; j < w; ++j) std::swap(b[std::size_t(k) * w + j], b[std::size_t(pivot) * w + j]);
					for (int i = 0; i < h; ++i) std::swap(b[std::size_t(i) * w + k], b[std::size_t(i) * w + pivot]);
				}
				V inverse = Ops::inv(b[std::size_t(k) * w + k - 1]);
				for (int i = k + 1; i < h; ++i) {
					V factor = Ops::mul(b[std::size_t(i) * w + k - 1], inverse);
					if (Ops::zero(factor, eps)) continue;
					Ops::axpy(b.data() + std::size_t(i) * w + k, b.data() + std::size_t(k) * w + k, w - k, Ops::neg(factor));
					b[std::size_t(i) * w + k - 1] = V(0);
					for (int j = 0; j < h; ++j) b[std::size_t(j) * w + k] = Ops::add(b[std::size_t(j) * w + k], Ops::mul(factor, b[std::size_t(j) * w + i]));
				}
			}
			std::vector<std::vector<V>> p(h + 1);
			p[0] = {V(1)};
			for (int n = 1; n <= h; ++n) {
				p[n].assign(n + 1, V(0));
				std::copy(p[n - 1].begin(), p[n - 1].end(), p[n].begin() + 1);
				Ops::axpy(p[n].data(), p[n - 1].data(), n, Ops::neg(b[std::size_t(n - 1) * w + n - 1]));
				V product = 1;
				for (int i = n - 2; i >= 0; --i) {
					product = Ops::mul(product, b[std::size_t(i + 1) * w + i]);
					V factor = Ops::mul(product, b[std::size_t(i) * w + n - 1]);
					Ops::axpy(p[n].data(), p[i].data(), i + 1, Ops::neg(factor));
				}
			}
			std::vector<T> result(h + 1);
			for (int i = 0; i <= h; ++i) result[i] = T(p[h][i]);
			return result;
		});
	}
	Matrix adjugate(long double eps = default_epsilon) const {
		assert(h == w);
		auto work = augmented_identity();
		auto [determinant, pivots] = work.reduce(w, true, eps);
		if (int(pivots.size()) == h) return work.submatrix(0, h, w, w * 2) * determinant;
		Matrix result(h);
		if (int(pivots.size()) + 1 < h) return result;
		auto right = nullspace(work, pivots, w)[0];
		int row = 0, col = 0;
		for (int i = 1; i < h; ++i) if (work(h - 1, w + row) == T(0) || matrix_detail::Field<T>::better(work(h - 1, w + i), work(h - 1, w + row))) row = i;
		if (work(h - 1, w + row) == T(0)) return result;
		std::vector<T> left(h);
		T inverse = T(1) / work(h - 1, w + row);
		for (int i = 0; i < h; ++i) left[i] = work(h - 1, w + i) * inverse;
		while (std::find(pivots.begin(), pivots.end(), col) != pivots.end()) ++col;
		Matrix minor(h - 1);
		for (int i = 0, x = 0; i < h; ++i) if (i != row) {
			for (int j = 0, y = 0; j < w; ++j) if (j != col) minor(x, y++) = (*this)(i, j);
			++x;
		}
		T scale = minor.det(eps);
		if ((row + col) & 1) scale = -scale;
		for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) result(i, j) = scale * right[i] * left[j];
		return result;
	}
};

#endif
