#ifndef FPS_998244353_HPP
#define FPS_998244353_HPP

#include "../components/fps_operations.hpp"
#include "../components/fps_horner.hpp"
#include "../components/fps_workspace.hpp"
#include "../components/fps_sparse.hpp"

#include "../components/fps_blocked.hpp"
#include "../components/fps_exp_blocked.hpp"
#include "../components/fps_inverse_blocked.hpp"
#include "../components/fps_sqrt_blocked.hpp"
#include "../components/mint_convolution.hpp"
#include "../components/fps_division_blocked.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_API
#define CP_FPS_API 1
template <class T = mint>
struct [[nodiscard, gnu::warn_unused]] FormalPowerSeries : std::vector<T>, private fps_detail::Operations<T> {
	static_assert(T::mod() == 998244353, "FormalPowerSeries supports only mod 998244353");
	using W = FormalPowerSeries<>;
	using base = std::vector<T>;
	using F = FormalPowerSeries;
	using Sparse = fps_detail::SparseSeries;
	using base::base;
	using base::begin;
	using base::data;
	using base::size;

	FormalPowerSeries() = default;
	FormalPowerSeries(base v) : base(std::move(v)) {}
	F &operator=(base v) {
		base::operator=(std::move(v));
		return *this;
	}
	F &operator=(std::initializer_list<T> v) {
		base::operator=(v);
		return *this;
	}

	static F multiply(const F &a, const F &b) {
		if constexpr (std::is_same_v<T, mint>) return F(mint_convolution_detail::multiply(a, b));
		else {
			W x = import_work(a);
			return export_work(&a == &b ? W::multiply(x, x) : W::multiply(x, import_work(b)));
		}
	}

	F operator-() const {
		F r = *this;
		for (T &x : r) x = -x;
		return r;
	}
	F &operator+=(const F &g) {
		if (size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
		return *this;
	}
	F &operator+=(T x) {
		if (this->empty()) this->resize(1);
		(*this)[0] += x;
		return *this;
	}
	F &operator-=(T x) { return *this += -x; }
	F &operator*=(const F &g) { return *this = multiply(*this, g); }
	F &operator*=(T x) {
		if constexpr (std::is_same_v<T, mint>) {
			scale_words(*this, x);
			return *this;
		}
		for (T &a : *this) a *= x;
		return *this;
	}
	F &operator/=(const F &g) { return *this = div_series(g); }
	F &operator%=(const F &g) { return *this = divmod(g).second; }
	F &operator/=(T x) { return *this *= x.inv(); }

	F &operator<<=(int d) {
		assert(d >= 0);
		auto edge = std::shift_right(begin(), this->end(), d);
		std::fill(begin(), edge, T(0));
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		auto edge = std::shift_left(begin(), this->end(), d);
		std::fill(edge, this->end(), T(0));
		return *this;
	}

	F operator+(const F &g) const {
		F r(*this);
		r += g;
		return r;
	}
	F operator-(const F &g) const {
		F r(*this);
		r -= g;
		return r;
	}
	F operator*(const F &g) const { return multiply(*this, g); }
	F operator/(const F &g) const { return div_series(g); }
	F operator%(const F &g) const { return divmod(g).second; }
	F operator+(T x) const {
		F r(*this);
		r += x;
		return r;
	}
	F operator-(T x) const {
		F r(*this);
		r -= x;
		return r;
	}
	F operator*(T x) const {
		F r(*this);
		r *= x;
		return r;
	}
	F operator/(T x) const {
		F r(*this);
		r /= x;
		return r;
	}
	friend F operator+(T x, F f) { return f += x; }
	friend F operator-(T x, const F &f) { return -f + x; }
	friend F operator*(T x, F f) { return f *= x; }
	F operator<<(int d) const {
		F r(*this);
		r <<= d;
		return r;
	}
	F operator>>(int d) const {
		F r(*this);
		r >>= d;
		return r;
	}

	F pre(int n) const {
		assert(n >= 0);
		return F(begin(), begin() + std::min<int>(n, size()));
	}
	void shrink() { O::trim(*this); }
	void trim() { shrink(); }

	F div_series(const F &g, int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).div_series(import_work(g), deg));
		if (deg < 0) deg = size();
		assert(deg >= 0);
		if (!deg) return {};
		assert(!g.empty() && g[0] != T(0));
		if (this->empty()) return F(deg);
#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 129 && deg <= (1 << 23) && g.size() >= 64 && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_div_series(data(), size(), g.data(), g.size(), r, deg);
				return r;
			}
		}
#endif
		F r = multiply(pre(deg), g.inv(deg)).pre(deg);
		r.resize(deg);
		return r;
	}

	F inv(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).inv(deg));
		else {
			if (deg < 0) deg = static_cast<int>(size());
			assert(deg >= 0);
			if (deg == 0) return {};
			assert(!this->empty() && (*this)[0] != T(0));
			auto terms = Sparse::collect(data(), size(), deg, 32);
			int stride = 0;
			for (auto term : terms) stride = std::gcd(stride, term.first);
			if (terms.size() <= 32 && (deg <= 256 || terms.size() <= 12ULL * stride)) {
				const T inverse = (*this)[0].inv();
				for (auto &[j, x] : terms) x *= inverse;
				return F(Sparse::get<Sparse::Kind::inverse>(std::move(terms), deg, inverse, T(0), nullptr));
			}

#if CP_GCC_LINUX64
			if (deg >= 512 && deg <= (1 << 23) && NTT::vectorized()) {
				F result;
				const int block = 1 << (std::bit_width(static_cast<unsigned>(deg)) - 1), tail = deg - block;
				if (tail > 0 && tail <= 96) {
					fps_detail::prepare_fps_output(result, deg);
					fps_detail::blocked_poly::native_inverse(data(), size(), result, block);
					result.resize(deg);
					for (int i = block; i < deg; ++i)
						result[i] = -result[0] * dot(data() + 1, result.data() + i - 1, std::min<int>(i, size() - 1));
					return result;
				}
				fps_detail::blocked_poly::native_inverse(data(), size(), result, deg);
				return result;
			}
#endif

			F g{(*this)[0].inv()};
			g.resize(std::min(deg, 32));
			for (int i = 1; i < static_cast<int>(g.size()); ++i)
				g[i] =
					-g[0] * dot(data() + 1, g.data() + i - 1, std::min<int>(i, size() - 1));
#if CP_X86
			fps_detail::FpsWords storage;
			Word *a = nullptr, *b = nullptr;
			if (deg > 96 && NTT::vectorized()) {
				int capacity = 64;
				while (capacity < deg && capacity < (1 << 23)) capacity <<= 1;
				storage = fps_detail::allocate_fps_words(2 * capacity);
				a = storage.get();
				b = a + capacity;
				g.reserve(capacity);
				transform(std::max(32, capacity / 16));
			}
#endif
			while (static_cast<int>(g.size()) < deg) {
				const int m = static_cast<int>(g.size()), n = 2 * m;
				if (m >= 32 && (deg - m <= 32 || (m >= 512 && deg - m <= 96))) {
					g.resize(deg);
					for (int i = m; i < deg; ++i) {
						const int count = std::min<int>(i, size() - 1);
						g[i] = -g[0] * dot(data() + 1, g.data() + i - 1, count);
					}
					break;
				}
#if CP_X86
				if (m >= 64 && n <= (1 << 23) && NTT::vectorized()) {
					int lo = std::min<int>(n, size());
					std::copy_n(words(*this), lo, a);
					std::fill(a + lo, a + n, Word(0));
					std::copy_n(words(g), m, b);
					transform(std::max(1, n / 16)).inverse_newton8(a, b, n);
					g.resize(n);
					std::copy_n(a + m, m, words(g) + m);

				} else
#endif
				{
					base f = this->pre(n), r = g;
					f.resize(n);
					r.resize(n);
					ntt(f);
					ntt_half(r);
					pointwise(f, f, r, n);
					ntt_inverse(f);
					std::fill(f.begin(), f.begin() + m, T(0));
					ntt(f);
					pointwise(f, f, r, n);
					ntt_inverse(f);
					const T inv = T::raw(T::mod() - (T::mod() - 1) / n);
					const T scale = -inv * inv;
					g.resize(n);
					transform(1).scale(words(f) + m, m, scale.val());
					std::copy_n(f.begin() + m, m, g.begin() + m);
				}
			}
			g.resize(deg);
			return g;
		}
	}

	F sqrt(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).sqrt(deg));
		else {
			if (deg < 0) deg = static_cast<int>(size());
			assert(deg >= 0);
			int last = static_cast<int>(std::min<std::size_t>(deg, size())), first = 0;
			while (first < last && (*this)[first] == T(0)) ++first;
			if (first == last) return F(deg);
			if (first & 1) return {};
			const T constant = mod_sqrt((*this)[first]);
			if (constant == T(0)) return {};
			while ((*this)[last - 1] == T(0)) --last;
			if (first + 1 == last) {
				F result(deg);
				result[first / 2] = constant;
				return result;
			}
			const int shift = first / 2, target = deg - shift;
			if (target >= 256 && unsigned(target) < T::mod()) {
				auto terms = Sparse::collect(data() + first, last - first, target);
				if (terms.size() <= 12) {
					const T inverse = (*this)[first].inv();
					for (auto &[j, x] : terms) x *= inverse;
					F result(Sparse::get<Sparse::Kind::power>(std::move(terms), target, constant, T(2).inv(), inv_table));
					result.insert(result.begin(), shift, T(0));
					return result;
				}
			}
#if CP_GCC_LINUX64
			if (target >= 256 && target <= (1 << 23) && NTT::vectorized()) {
				F result;
				fps_detail::prepare_fps_output(result, deg);
				result.resize(shift);
				const int block = std::bit_floor(unsigned(target)), tail = target - block;
				const int work = block >= 512 && tail <= 8 ? block : target;
				fps_detail::blocked_poly::native_sqrt(data() + first, last - first, result, work, constant);
				if (work < target) {
					result.resize(deg);
					const T factor = (constant * T(2)).inv();
					for (int i = work; i < target; ++i)
						result[shift + i] = ((first + i < last ? (*this)[first + i] : T(0)) - dot(result.data() + shift + 1, result.data() + shift + i - 1, i - 1)) * factor;
				}
				return result;
			}
#endif
			F f(begin() + first, begin() + last);
			f.resize(target);
			const T half = T(2).inv(), scale = half / constant;
			F g(std::min(target, 32));
			g[0] = constant;
			for (int i = 1; i < static_cast<int>(g.size()); ++i) {
				T sum = 0;
				for (int j = 1; j < i; ++j) sum += g[j] * g[i - j];
				g[i] = (f[i] - sum) * scale;
			}
			F h;
			base cache;
			if (static_cast<int>(g.size()) < target) {
				h = g.pre(16).inv(16);
				cache = h;
				cache.resize(32);
				ntt(cache);
			}
			while (static_cast<int>(g.size()) < target) {
				const int m = static_cast<int>(g.size()), n = 2 * m;
				if (target - m <= 32) {
					g.resize(target);
					for (int i = m; i < target; ++i)
						g[i] = (f[i] - dot(g.data() + 1, g.data() + i - 1, i - 1)) * scale;
					break;
				}
				assert(n <= (1 << 23));
				const T in = T::raw(T::mod() - (T::mod() - 1) / n);
				base a = g, error(n);
				a.resize(n);
				ntt(a);
				inverse_extend(h, a, cache);
				cache = h;
				cache.resize(n);
				ntt(cache);
				pointwise(error, a, a, n);
				ntt_inverse(error);
				std::fill(error.begin(), error.begin() + m, T(0));
				for (int i = m; i < n; ++i) error[i] = (i < target ? f[i] : T(0)) - error[i] * in;
				ntt(error);
				pointwise(error, error, cache, n);
				F::scale_words(error, half);
				ntt_inverse(error);
				g.resize(std::min(n, target));
				for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
			}
			g.resize(target);
			g.insert(g.begin(), shift, T(0));
			return g;
		}
	}

	[[nodiscard]] T eval(T x) const {
		if constexpr (std::is_same_v<T, mint>) return fps_detail::horner(data(), size(), x);
		T ans = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) ans = ans * x + *it;
		return ans;
	}

	void onemul(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this *= T(1) + c;
			return;
		}
		for (int i = (static_cast<int>(size())) - 1; i >= d; --i) (*this)[i] += (*this)[i - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this /= T(1) + c;
			return;
		}
		for (int i = d; i < static_cast<int>(size()); ++i) (*this)[i] -= (*this)[i - d] * c;
	}

	F diff() const {
		const int n = static_cast<int>(size());
		F r(n);
		if constexpr (std::is_same_v<T, mint>) {
			if (n > 1) NTT::weighted(words(r), words(*this) + 1, n - 1, 1);
		} else
			for (int i = 1; i < n; ++i) r[i - 1] = (*this)[i] * i;
		return r;
	}
	F integral() const {
		const int n = static_cast<int>(size());
		assert(unsigned(n) < unsigned(T::mod()));
		F r(n);
		const auto &inv = inverses(n);
		if constexpr (std::is_same_v<T, mint>) {
			if (n > 1) NTT::product(words(r) + 1, words(*this), words(inv) + 1, n - 1);
		} else
			for (int i = 1; i < n; ++i) r[i] = (*this)[i - 1] * inv[i];
		return r;
	}

	F log(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).log(deg));
		else {
			if (deg < 0) deg = static_cast<int>(size());
			assert(deg >= 0);
			if (deg == 0) return {};
			assert(!this->empty() && (*this)[0] == T(1));
			if (deg >= 256 && unsigned(deg) < T::mod()) {
				auto terms = Sparse::collect(data(), size(), deg);
				if (terms.size() <= 12) return F(Sparse::get<Sparse::Kind::logarithm>(std::move(terms), deg, T(0), T(0), inv_table));
			}
			return log_derivative(deg, true);
		}
	}

	F exp(int deg = -1) const {
		if constexpr (std::is_same_v<T, mint>) return exp_impl<false>(deg);
		else return export_work(import_work(*this).exp(deg));
	}

	F pow(long long k) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).pow(k));
		else {
			assert(k >= 0);
			const int n = static_cast<int>(size());
			if (n == 0) return {};
			if (k == 0) {
				F r(n);
				r[0] = 1;
				return r;
			}
			if (k == 1) return *this;
			int first = 0;
			while (first < n && (*this)[first] == T(0)) ++first;
			if (first == n || first > (n - 1) / k) return F(n);
			const int shift = first * k;
			const int len = n - shift;
			const T lead = (*this)[first];
			if (unsigned(len) < T::mod()) {
				auto terms = Sparse::collect(data() + first, size() - first, len);
				if (terms.size() <= 12) {
					const T inverse = lead.inv();
					for (auto &[j, x] : terms) x *= inverse;
					F result(Sparse::get<Sparse::Kind::power>(std::move(terms), len, lead.pow(k), T(k), inv_table));
					result.insert(result.begin(), shift, T(0));
					return result;
				}
			}
			if (k <= 4) {
				F square = multiply(*this, *this).pre(n);
				return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
			}

#if CP_GCC_LINUX64
			if (len >= 256 && len <= (1 << 24) && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_power(data() + first, size() - first, r, len, T(k).val(), lead.pow(k), shift);
				return r;
			}
#endif
			F f(begin() + first, this->end());
			f.resize(len);
			f = f.log_derivative(len);
			f *= T(k);
			f = f.template exp_impl<true>(len);
			f *= lead.pow(k);
			F r(n);
			for (int i = 0; i < len; ++i) r[shift + i] = f[i];
			return r;
		}
	}

	[[nodiscard]] std::pair<F, F> divmod(const F &divisor) const {
		if constexpr (!std::is_same_v<T, mint>) {
			auto [q, r] = import_work(*this).divmod(import_work(divisor));
			return {export_work(q), export_work(r)};
		}
		int n = static_cast<int>(size()), m = static_cast<int>(divisor.size());
		while (n && (*this)[n - 1] == T(0)) --n;
		while (m && divisor[m - 1] == T(0)) --m;
		assert(m);
		if (n < m) return {{}, pre(n)};
		const int qn = n - m + 1;

#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (m > 2 && (std::size_t)m * qn > 4096 && n <= (1 << 23) && NTT::vectorized()) {
				F q, r;
				fps_detail::blocked_poly::native_divide(data(), n, divisor.data(), m, q, r);
				q.trim(), r.trim();
				return {std::move(q), std::move(r)};
			}
		}
#endif
		F a = pre(n);
		if (m <= 2 || (std::size_t)m * qn <= 4096) {
			F q(qn);
			const T inv = divisor[m - 1].inv();
			for (int i = qn - 1; i >= 0; --i) {
				q[i] = a[i + m - 1] * inv;
				for (int j = 0; j < m; ++j) a[i + j] -= q[i] * divisor[j];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F ra = a, rb(divisor.begin(), divisor.begin() + m);
		std::reverse(ra.begin(), ra.end());
		std::reverse(rb.begin(), rb.end());
		F q = ra.pre(qn).div_series(rb, qn);
		std::reverse(q.begin(), q.end());
		std::reverse(rb.begin(), rb.end());
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		for (int i = 0; i < std::min<int>(a.size(), product.size()); ++i) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

	private:
	using O = fps_detail::Operations<T>;
	using O::import_work, O::export_work, O::transform, O::words, O::transform_inplace, O::ntt, O::ntt_half, O::ntt_inverse, O::scale_words, O::geometric_scale, O::batch_inverse, O::pointwise, O::inverse_extend, O::dot, O::intt, O::mod_sqrt, O::inverse_ntt_roots, O::ntt_roots, O::inverses;

	using NTT = fps_detail::NTT;
	using Word = typename O::Word;

	F log_derivative(int deg, bool integrate = false) const {
#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 256 && deg <= (1 << 24) && size() >= 64 && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_log(data(), size(), r, deg, integrate);
				return r;
			}
		}
#endif

		F derivative(std::min<int>(deg, size()));
		if (derivative.size() > 1) NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		if (integrate) {
			const auto &iv = inverses(deg - 1);
			NTT::product(words(r), words(r), words(iv), deg);
		}
		return r;
	}

	template <bool differentiated>
	F exp_impl(int deg) const {
		if (deg < 0) deg = static_cast<int>(size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		if constexpr (std::is_same_v<T, mint>) {
			auto terms = Sparse::collect(data(), size(), deg, 64);
			int stride = 0;
			for (auto term : terms) stride = std::gcd(stride, term.first);
			if (terms.size() <= 64 && (deg <= 256 || terms.size() <= 24ULL * stride)) {
				if constexpr (differentiated) for (auto &[j, x] : terms) x /= T(j);
				return F(Sparse::get<Sparse::Kind::exponential>(std::move(terms), deg, T(1), T(0), inv_table));
			}
		}

#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 256 && deg <= (1 << 24) && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_exp(data(), size(), r, deg, differentiated);
				return r;
			}
		}
#endif
		const auto &iv = inverses(deg);
		const int initial = std::min(deg, 32);
		F g(initial);
		g[0] = 1;
		for (int i = 1; i < initial; ++i) {
			T sum = 0;
			for (int j = 1; j <= i && j < static_cast<int>(size()); ++j)
				sum += (differentiated ? (*this)[j] : T(j) * (*this)[j]) * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		F h = g.pre(16).inv(16);
		base cache = h;
		cache.resize(32);
		ntt(cache);
		while (static_cast<int>(g.size()) < deg) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (deg - m <= 32) {
				base derivative(std::min<int>(deg, size()));
				for (int i = 1; i < static_cast<int>(derivative.size()); ++i) derivative[i] = differentiated ? (*this)[i] : T(i) * (*this)[i];
				derivative.resize(std::max<int>(1, derivative.size()));
				g.resize(deg);
				for (int i = m; i < deg; ++i) {
					const int count = std::min<int>(i, derivative.size() - 1);
					g[i] = dot(derivative.data() + 1, g.data() + i - 1, count) * iv[i];
				}
				break;
			}
			assert(n <= (1 << 23));
			const T im = T::raw(T::mod() - (T::mod() - 1) / m);
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, b(m), error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			std::fill(b.begin(), b.end(), T(0));
			for (int i = 1; i < m && i < static_cast<int>(size()); ++i) b[i - 1] = differentiated ? (*this)[i] : T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			for (int i = 0; i < m - 1; ++i) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			for (int i = (std::min(n, deg)) - 1; i >= m; --i)
				error[i] = (i < static_cast<int>(size()) ? (*this)[i] * (differentiated ? iv[i] : T(1)) : T(0)) + error[i - 1] * in * iv[i];
			std::fill(error.begin(), error.begin() + m, T(0));
			std::fill(error.begin() + std::min(n, deg), error.end(), T(0));
			ntt(error);
			pointwise(error, error, a, n);
			ntt_inverse(error);
			g.resize(std::min(n, deg));
			for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
		}
		return g;
	}

	static const mint *inv_table(int n) { return fps_detail::Operations<mint>::inverses(n).data(); }

};

using fps = FormalPowerSeries<>;

#endif

#if CP_GCC_X86
#pragma GCC pop_options
#endif

#endif
