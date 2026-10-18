#ifndef FPS_998244353_HPP
#define FPS_998244353_HPP

#include "../components/fps_operations.hpp"
#include "../components/fps_factorials.hpp"
#include "../components/fps_workspace.hpp"
#include "../components/fps_log.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class T = fps_mint>
struct FormalPowerSeries : std::vector<T>, private fps_detail::Operations<T> {
	static_assert(T::mod() == 998244353, "FormalPowerSeries supports only mod 998244353");
	using W = FormalPowerSeries<fps_detail::Mint>;
	using base = std::vector<T>;
	using F = FormalPowerSeries;
	using base::base;

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
		if (a.empty() || b.empty()) return {};
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			W x = import_work(a);
			return export_work(&a == &b ? W::multiply(x, x) : W::multiply(x, import_work(b)));
		}
		if (std::min(a.size(), b.size()) <= 32) {
			F r(a.size() + b.size() - 1);
			const F &small = a.size() < b.size() ? a : b;
			const F &large = a.size() < b.size() ? b : a;
			if constexpr (std::is_same_v<T, fps_detail::Mint>) {
				for (int j = 0; j < static_cast<int>(small.size()); ++j)
					fps_detail::NTT::add_scaled(words(r) + j, words(large), large.size(), small[j].val());
			} else
				for (int j = 0; j < static_cast<int>(small.size()); ++j)
					for (int i = 0; i < static_cast<int>(large.size()); ++i) r[i + j] += large[i] * small[j];
			return r;
		}
		int n = 1, size = a.size() + b.size() - 1;
		while (n < size) n <<= 1;
		assert(n <= (1 << 23));
		F x = a, y;
		x.resize(n);
		if (&a != &b) {
			y = b;
			y.resize(n);
		}
		if (n >= 64 && &a != &b) {
			auto &t = transform(n / 16);
			t.convolution(words(x), words(y), n);
			x.resize(size);
			t.scale(words(x), size, T::mod() - (T::mod() - 1) / (n / 8));
			return x;
		}
		ntt(x);
		if (&a == &b) pointwise(x, x, x, n);
		else {
			ntt(y);
			pointwise(x, x, y, n);
		}
		intt(x);
		x.resize(size);
		return x;
	}

	F operator-() const {
		F r = *this;
		for (T &x : r) x = -x;
		return r;
	}
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
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
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
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
		const int n = static_cast<int>(this->size());
		if (d >= n) return *this = F(n);
		this->insert(this->begin(), d, T(0));
		this->resize(n);
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		const int n = static_cast<int>(this->size());
		if (d >= n) return *this = F(n);
		this->erase(this->begin(), this->begin() + d);
		this->resize(n);
		return *this;
	}

	F operator+(const F &g) const { return F(*this) += g; }
	F operator-(const F &g) const { return F(*this) -= g; }
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
	F operator<<(int d) const { return F(*this) <<= d; }
	F operator>>(int d) const { return F(*this) >>= d; }

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min<int>(n, this->size()));
	}

	F div_series(const F &g, int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).div_series(import_work(g), deg));
		if (deg < 0) deg = this->size();
		assert(deg >= 0);
		if (!deg) return {};
		assert(!g.empty() && g[0] != T(0));
		if (this->empty()) return F(deg);
		if (deg <= 128 || g.size() < 64) {
			F r = multiply(pre(deg), g.inv(deg)).pre(deg);
			r.resize(deg);
			return r;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		if (deg < size - size / 4) size >>= 1;
		const int block = size / 32, length = 2 * block, pitch = length + 16, count = (deg + block - 1) / block;
		F inverse = g.inv(block), r = multiply(pre(block), inverse).pre(block);
		r.resize(deg);
		inverse.resize(length);
		ntt_half(inverse);
		const Word norm = T::mod() - (T::mod() - 1) / length;
		transform(1).scale(words(inverse), length, norm);
		auto storage = fps_detail::allocate_fps_words((2 * count - 1) * pitch);
		Word *denominator = storage.get(), *quotient = denominator + count * pitch;
		for (int i = 0; i < count; ++i) {
			Word *h = denominator + i * pitch;
			const int available = std::max(0, std::min<int>(block, static_cast<int>(g.size()) - i * block));
			if (available) std::copy_n(words(g) + i * block, available, h);
			std::fill(h + available, h + block, 0u);
			ntt_half(h, length);
		}
		O::join_windows_strided(denominator, length, count, pitch);
		base error(length);
		for (int i = 1; i < count; ++i) {
			const int offset = i * block, n = std::min(block, deg - offset);
			Word *q = quotient + (i - 1) * pitch;
			std::copy_n(words(r) + offset - block, block, q);
			ntt_half(q, length);
			fps_detail::NTT::scale_normalize(q, length, norm);

			for (int j = 0; j < i; j += 7) {
				const Word *a[7], *b[7];
				int terms = std::min(7, i - j);
				for (int k = 0; k < terms; ++k) {
					a[k] = quotient + (j + k) * pitch;
					b[k] = denominator + (i - j - k) * pitch;
				}
				auto update = [&](auto number) {
					constexpr int count = decltype(number)::value;
					if (j == 0) O::template accumulate_products<count, true>(words(error), a, b, length);
					else O::template accumulate_products<count, false>(words(error), a, b, length);
				};
				if (terms == 7) update(std::integral_constant<int, 7>{});
				else if (terms == 6) update(std::integral_constant<int, 6>{});
				else if (terms == 5) update(std::integral_constant<int, 5>{});
				else if (terms == 4) update(std::integral_constant<int, 4>{});
				else if (terms == 3) update(std::integral_constant<int, 3>{});
				else if (terms == 2) update(std::integral_constant<int, 2>{});
				else update(std::integral_constant<int, 1>{});
			}
			ntt_inverse(error);
			for (int j = 0; j < block; ++j) error[j] = (offset + j < static_cast<int>(this->size()) ? (*this)[offset + j] : T(0)) - error[j];
			ntt_half(error);
			pointwise(error, error, inverse, length);
			ntt_inverse(error);
			std::copy_n(error.begin(), n, r.begin() + offset);
		}
		return r;
	}

	F inv(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).inv(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] != T(0));
		F g{(*this)[0].inv()};
		auto terms = sparse_terms(deg, 32);
		int stride = terms.size() <= 32 ? prepare_sparse(terms) : 1;
		if (terms.size() <= 32 && (deg <= 256 || terms.size() <= 12ULL * stride)) {
			g.resize(deg);
			if (terms.size() == 1 && stride == 1) {
				std::fill(g.begin(), g.end(), g[0]);
				geometric_scale(g, T(1), -terms[0].second * g[0]);
				return g;
			}
			for (long long i = stride; stride && i < deg; i += stride) g[i] = -sparse_dot(terms, g, static_cast<int>(i)) * g[0];
			return g;
		}

		if (deg > 256 && this->size() >= 64) {
#if defined(__x86_64__) || defined(__i386__)
			if (deg < 1024 || !fps_detail::NTT::vectorized()) return F{1}.div_series(*this, deg);
#else
			return F{1}.div_series(*this, deg);
#endif
		}
		g.resize(std::min(deg, 32));
		for (int i = 1; i < static_cast<int>(g.size()); ++i)
			g[i] =
				-g[0] * dot(this->data() + 1, g.data() + i - 1, std::min<int>(i, this->size() - 1));
#if defined(__x86_64__) || defined(__i386__)
		fps_detail::FpsWords storage;
		Word *a = nullptr, *b = nullptr;
		if (deg > 96 && fps_detail::NTT::vectorized()) {
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
					const int count = std::min<int>(i, this->size() - 1);
					g[i] = -g[0] * dot(this->data() + 1, g.data() + i - 1, count);
				}
				break;
			}
#if defined(__x86_64__) || defined(__i386__)
			if (m >= 64 && n <= (1 << 23) && fps_detail::NTT::vectorized()) {
				int lo = std::min<int>(n, this->size());
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

	F sqrt(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).sqrt(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		F f = pre(deg);
		f.trim();
		int first = 0;
		while (first < static_cast<int>(f.size()) && f[first] == T(0)) ++first;
		if (first == static_cast<int>(f.size())) return F(deg);
		if (first & 1) return {};
		const T constant = mod_sqrt(f[first]);
		if (constant == T(0)) return {};
		if (first + 1 == static_cast<int>(f.size())) {
			F result(deg);
			result[first / 2] = constant;
			return result;
		}
		const int shift = first / 2, target = deg - shift;
		f.erase(f.begin(), f.begin() + first);
		f.resize(target);
		const T half = T(2).inv(), scale = half / constant;
		F g(std::min(target, 32));
		g[0] = constant;
		for (int i = 1; i < static_cast<int>(g.size()); ++i) {
			T sum = 0;
			for (int j = 1; j < i; ++j) sum += g[j] * g[i - j];
			g[i] = (f[i] - sum) * scale;
		}
		if (target > 256) {
			int size = 1;
			while (size < target) size <<= 1;
			if (target < size - size / 4) size >>= 1;
			const int block = size / 32, length = 2 * block, pitch = length + 16, count = (target + block - 1) / block;
			g = f.pre(block).sqrt(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word half_norm = T::mod() - (T::mod() - 1) / (2 * length);
			const int levels = __builtin_ctz((unsigned int)length);
			T root_norm = T::raw(T::mod() - (T::mod() - 1) / (1 << (levels / 2)));
			if (levels & 1) root_norm *= T::raw(441024591);
			transform(1).scale(words(inverse), length, half_norm);
			auto storage = fps_detail::allocate_fps_words((count - 1) * pitch);
			Word *cache = storage.get();
			base error(length);
			base zero(length);
			g.resize(target);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, target - offset);
				Word *q = cache + (i - 1) * pitch;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, root_norm.val());

				for (int j = 0; j < (i + 1) / 2; j += 6) {
					const Word *a[6], *b[6], *c[6];
					int terms = std::min(6, (i + 1) / 2 - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * pitch;
						b[k] = (j + k ? cache + (i - j - k) * pitch : words(zero));
						c[k] = cache + (i - j - k - 1) * pitch;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						if (j == 0) {
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error) + block, a, b, c, block, true);
						} else {
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error) + block, a, b, c, block, true);
						}
					};
					if (terms == 6) update(std::integral_constant<int, 6>{});
					else if (terms == 5) update(std::integral_constant<int, 5>{});
					else if (terms == 4) update(std::integral_constant<int, 4>{});
					else if (terms == 3) update(std::integral_constant<int, 3>{});
					else if (terms == 2) update(std::integral_constant<int, 2>{});
					else update(std::integral_constant<int, 1>{});
				}
				fps_detail::NTT::square_fold(words(error), cache + (i / 2) * pitch, block, i & 1);
				ntt_inverse(error);
				for (int j = 0; j < block; ++j) error[j] = (offset + j < target ? f[offset + j] : T(0)) - error[j];
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				std::copy_n(error.begin(), n, g.begin() + offset);
			}
			g.insert(g.begin(), shift, T(0));
			return g;
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

	T eval(T x) const {
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
		for (int i = (static_cast<int>(this->size())) - 1; i >= d; --i) (*this)[i] += (*this)[i - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this /= T(1) + c;
			return;
		}
		for (int i = d; i < static_cast<int>(this->size()); ++i) (*this)[i] -= (*this)[i - d] * c;
	}

	F diff() const {
		const int n = static_cast<int>(this->size());
		F r(n);
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::weighted(words(r), words(*this) + 1, n - 1, 1);
		} else
			for (int i = 1; i < n; ++i) r[i - 1] = (*this)[i] * i;
		return r;
	}
	F integral() const {
		const int n = static_cast<int>(this->size());
		assert(unsigned(n) < unsigned(T::mod()));
		F r(n);
		const auto &inv = inverses(n);
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::product(words(r) + 1, words(*this), words(inv) + 1, n - 1);
		} else
			for (int i = 1; i < n; ++i) r[i] = (*this)[i - 1] * inv[i];
		return r;
	}

	F log(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).log(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] == T(1));
		return log_derivative(deg, true);
	}

	F exp(int deg = -1) const {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) return exp_impl<false>(deg);
		else return export_work(import_work(*this).exp(deg));
	}

	F pow(long long k) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).pow(k));
		assert(k >= 0);
		const int n = static_cast<int>(this->size());
		if (n == 0) return {};
		if (k == 0) {
			F r(n);
			r[0] = 1;
			return r;
		}
		if (k == 1) return *this;
		if (k <= 4) {
			F square = multiply(*this, *this).pre(n);
			return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
		}
		int first = 0;
		while (first < n && (*this)[first] == T(0)) ++first;
		if (first == n || first > (n - 1) / k) return F(n);
		const int shift = first * k;
		const int len = n - shift;
		const T lead = (*this)[first];
		F f(this->begin() + first, this->end());
		f.resize(len);
		f = f.log_derivative(len);
		f *= T(k);
		f = f.template exp_impl<true>(len);
		f *= lead.pow(k);
		F r(n);
		for (int i = 0; i < len; ++i) r[shift + i] = f[i];
		return r;
	}

	std::pair<F, F> divmod(const F &divisor) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			auto [q, r] = import_work(*this).divmod(import_work(divisor));
			return {export_work(q), export_work(r)};
		}
		F a = *this;
		a.trim();
		int m = static_cast<int>(divisor.size());
		while (m && divisor[m - 1] == T(0)) --m;
		assert(m);
		if (static_cast<int>(a.size()) < m) return {{}, std::move(a)};
		const int qn = static_cast<int>(a.size()) - m + 1;
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
		int cyclic = 1;
		while (cyclic < std::max(m, qn)) cyclic <<= 1;
		if (std::min(m - 1, qn) > 32 && cyclic < m + std::min(m - 1, qn) - 2) {
			assert(cyclic <= (1 << 23));
			F product = rb, quotient = q;
			product.resize(cyclic);
			quotient.resize(cyclic);
			auto &t = transform(cyclic / 16);
			t.convolution(words(product), words(quotient), cyclic);
			t.scale(words(product), m - 1, T::mod() - (T::mod() - 1) / (cyclic / 8));
			for (int i = 0; i < m - 1; ++i) {
				a[i] -= product[i];
				if (i + cyclic < static_cast<int>(a.size())) a[i] += a[i + cyclic];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		for (int i = 0; i < std::min<int>(a.size(), product.size()); ++i) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

	private:
	using O = fps_detail::Operations<T>;
	using O::import_work;
	using O::export_work;
	using O::transform;
	using O::words;
	using O::transform_inplace;
	using O::ntt;
	using O::ntt_half;
	using O::ntt_inverse;
	using O::scale_words;
	using O::geometric_scale;
	using O::batch_inverse;
	using O::pointwise;
	using O::inverse_extend;
	using O::dot;
	using O::prepare_sparse;
	using O::sparse_dot;
	using O::intt;
	using O::mod_sqrt;
	using O::inverse_ntt_roots;
	using O::ntt_roots;
	using O::inverses;

	using Word = typename O::Word;

	F log_derivative(int deg, bool integrate = false) const {
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 256 && deg <= (1 << 24) && this->size() >= 64 && fps_detail::NTT::vectorized()) {
				int k = 32;
				while (k * 4 < deg - 1) k *= 2;
				const int n = 2 * k, count = (deg - 1 + k - 1) / k, pitch = n + 16;
				F h = inv(k), r(deg);
				fps_detail::LogTransform t{transform(n / 16)};
				auto memory = fps_detail::allocate_fps_words((count + 2) * pitch);
				Word *ht = memory.get(), *window = ht + pitch, *q = window + (count - 1) * pitch, *work = q + pitch;
				std::copy_n(words(h), k, ht);
				const Word norm = T::mod() - (T::mod() - 1) / (n / 8);
				t.forward(ht, n, norm, true);
				for (int j = 1; j < count; ++j) {
					Word *w = window + (j - 1) * pitch;
					const int first = (j - 1) * k, c = std::max(0, std::min<int>(n, static_cast<int>(this->size()) - first));
					if (c) std::copy_n(words(*this) + first, c, w);
					std::fill(w + c, w + n, 0u);
					t.forward(w, n);
				}
				for (int j = 0; j < count; ++j) {
					const int first = j * k, c = std::min(k, deg - 1 - first);
					if (j) {
						const Word *aa[] = {window + (j - 1) * pitch, window}, *bb[] = {q, work};
						if (count == 4 && j == 2)
							t.toeplitz(work, window + 2 * pitch, window, window + pitch, q, work, n);
						else if (count == 4 && j == 3) {
							const Word *a[] = {window}, *b[] = {work};
							t.products(work, a, b, 1, n, window + 2 * pitch);
						} else {
							t.products(work, aa, bb, j, n);
						}
						t.inverse(work, n);
					}
					const int available = std::max(0, std::min<int>(c, static_cast<int>(this->size()) - first - 1));
					if (available) fps_detail::NTT::weighted(work, words(*this) + first + 1, available, first + 1);
					std::fill(work + available, work + k, 0u);
					if (j)
						for (int i = 0; i < k; ++i) work[i] = fps_detail::NTT::sub(work[i], work[k + i]);
					t.forward(work, n, 1, true);
					const Word *aa[] = {work}, *bb[] = {ht};
					t.products(work, aa, bb, 1, n);
					t.inverse(work, n);
					if (integrate)
						fps_detail::LogTransform::integrate(words(r) + first + 1, work, work + k, first + 1, c);
					else
						std::copy_n(work, c, words(r) + first + 1);
					if (j + 1 < count) {
						Word *cache = j == 0 ? q : work;
						if (j == 0) std::copy_n(work, k, cache);
						t.forward(cache, n, norm, true);
					}
				}
				return r;
			}
		}
#endif

		F derivative(std::min<int>(deg, this->size()));
		if (derivative.size() > 1) fps_detail::NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		if (integrate) {
			const auto &iv = inverses(deg - 1);
			fps_detail::NTT::product(words(r), words(r), words(iv), deg);
		}
		return r;
	}

	template <bool differentiated>
	F exp_impl(int deg) const {
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		auto terms = sparse_terms(deg, 64);
		int stride = terms.size() <= 64 ? prepare_sparse(terms) : 1;
		if (terms.size() <= 64 && (deg <= 256 || terms.size() <= 24ULL * stride)) {
			F g(deg);
			g[0] = 1;
			if (terms.size() == 1 && stride == 1 && deg >= 256) {
				base fact(deg);
				fps_detail::factorials(words(fact), words(g), deg);
				geometric_scale(g, T(1), terms[0].second);
				return g;
			}
			if (!stride) return g;
			const auto &iv = inverses((deg - 1) / stride);
			T factor = differentiated ? T(stride).inv() : T(1);
			for (auto &[j, x] : terms) x *= differentiated ? factor : T(j / stride);
			prepare_sparse(terms);
			for (long long i = stride; i < deg; i += stride) g[i] = sparse_dot(terms, g, static_cast<int>(i)) * iv[i / stride];
			return g;
		}
		const auto &iv = inverses(deg);
		const int initial = std::min(deg, 32);
		F g(initial);
		g[0] = 1;
		for (int i = 1; i < initial; ++i) {
			T sum = 0;
			for (int j = 1; j <= i && j < static_cast<int>(this->size()); ++j)
				sum += (differentiated ? (*this)[j] : T(j) * (*this)[j]) * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		if (deg > 256) {
			int size = 1;
			while (size < deg) size <<= 1;
			if (deg < size - size / 4) size >>= 1;
			const int block = size / 32, length = 2 * block, pitch = length + 16, count = (deg + block - 1) / block;
			g = pre(block).template exp_impl<differentiated>(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word norm = T::mod() - (T::mod() - 1) / length;
			transform(1).scale(words(inverse), length, norm);
			auto storage = fps_detail::allocate_fps_words((2 * count - 1) * pitch);
			Word *derivative = storage.get(), *cache = derivative + count * pitch;
			for (int i = 0; i < count; ++i) {
				Word *d = derivative + i * pitch;
				const int offset = i * block;
				const int available = std::max(0, std::min<int>(block, std::min<int>(deg, this->size()) - offset));
				if (available) {
					if constexpr (differentiated) std::copy_n(words(*this) + offset, available, d);
					else fps_detail::NTT::weighted(d, words(*this) + offset, available, offset);
				}
				std::fill(d + available, d + block, 0u);
				ntt_half(d, length);
			}
			O::join_windows_strided(derivative, length, count, pitch);
			base error(length);
			g.resize(deg);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, deg - offset);
				Word *q = cache + (i - 1) * pitch;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, norm);
				for (int j = 0; j < i; j += 7) {
					const Word *a[7], *b[7];
					int terms = std::min(7, i - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * pitch;
						b[k] = derivative + (i - j - k) * pitch;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						if (j == 0) O::template accumulate_products<count, true>(words(error), a, b, length);
						else O::template accumulate_products<count, false>(words(error), a, b, length);
					};
					if (terms == 7) update(std::integral_constant<int, 7>{});
					else if (terms == 6) update(std::integral_constant<int, 6>{});
					else if (terms == 5) update(std::integral_constant<int, 5>{});
					else if (terms == 4) update(std::integral_constant<int, 4>{});
					else if (terms == 3) update(std::integral_constant<int, 3>{});
					else if (terms == 2) update(std::integral_constant<int, 2>{});
					else update(std::integral_constant<int, 1>{});
				}
				ntt_inverse(error);
				std::fill(error.begin() + block, error.end(), T(0));
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				fps_detail::NTT::product(words(error), words(error), words(iv) + offset, n);
				std::fill(error.begin() + n, error.end(), T(0));
				ntt_half(error);
				fps_detail::NTT::product(words(error), words(error), cache, length);
				ntt_inverse(error);
				std::copy_n(error.begin(), n, g.begin() + offset);
			}
			return g;
		}
		F h = g.pre(16).inv(16);
		base cache = h;
		cache.resize(32);
		ntt(cache);
		while (static_cast<int>(g.size()) < deg) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (deg - m <= 32) {
				base derivative(std::min<int>(deg, this->size()));
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
			for (int i = 1; i < m && i < static_cast<int>(this->size()); ++i) b[i - 1] = differentiated ? (*this)[i] : T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			for (int i = 0; i < m - 1; ++i) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			for (int i = (std::min(n, deg)) - 1; i >= m; --i)
				error[i] = (i < static_cast<int>(this->size()) ? (*this)[i] * (differentiated ? iv[i] : T(1)) : T(0)) + error[i - 1] * in * iv[i];
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

	std::vector<std::pair<int, T>> sparse_terms(int deg, int limit) const {
		std::vector<std::pair<int, T>> terms;
		for (int i = 1; i < std::min<int>(deg, this->size()) && static_cast<int>(terms.size()) <= limit; ++i)
			if ((*this)[i] != T(0)) terms.emplace_back(i, (*this)[i]);
		return terms;
	}

	void trim() { O::trim(*this); }

};

using fps = FormalPowerSeries<fps_mint>;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
