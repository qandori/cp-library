#ifndef FPS_998244353_HPP
#define FPS_998244353_HPP

#include "../convolution/ntt.hpp"
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
struct FormalPowerSeries : std::vector<T> {
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
	F operator+(T x) const { return F(*this) += x; }
	F operator-(T x) const { return F(*this) -= x; }
	F operator*(T x) const { return F(*this) *= x; }
	F operator/(T x) const { return F(*this) /= x; }
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
			F r = multiply(*this, g.inv(deg)).pre(deg);
			r.resize(deg);
			return r;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		const int block = size / 32, length = 2 * block, count = (deg + block - 1) / block;
		F inverse = g.inv(block), r = multiply(pre(block), inverse).pre(block);
		r.resize(deg);
		inverse.resize(length);
		ntt_half(inverse);
		const Word norm = T::mod() - (T::mod() - 1) / length;
		transform(1).scale(words(inverse), length, norm);
		fps_detail::NTT::CompositionWords storage(new (std::align_val_t(64)) unsigned int[(2 * count - 1) * length]);
		Word *denominator = storage.get(), *quotient = denominator + count * length;
		const int first = std::min<int>(block, g.size());
		std::copy_n(words(g), first, denominator);
		std::fill(denominator + first, denominator + block, 0u);
		ntt_half(denominator, length);
		base error(length);
		for (int i = 1; i < count; ++i) {
			const int offset = i * block, n = std::min(block, deg - offset);
			Word *h = denominator + i * length, *q = quotient + (i - 1) * length;
			const int available = std::max(0, std::min<int>(block, static_cast<int>(g.size()) - offset));
			if (available) std::copy_n(words(g) + offset, available, h);
			std::fill(h + available, h + block, 0u);
			ntt_half(h, length);
			std::copy_n(words(r) + offset - block, block, q);
			ntt_half(q, length);
			fps_detail::NTT::scale_normalize(q, length, norm);

			for (int j = 0; j < i; j += 4) {
				const Word *a[4], *b[4], *c[4];
				int terms = std::min(4, i - j);
				for (int k = 0; k < terms; ++k) {
					a[k] = quotient + (j + k) * length;
					b[k] = denominator + (i - j - k) * length;
					c[k] = denominator + (i - j - k - 1) * length;
				}
				auto update = [&](auto number) {
					constexpr int count = decltype(number)::value;
					if (j == 0) {
						fps_detail::NTT::template accumulate_wide<count, true, false>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_wide<count, true, false>(words(error) + block, a, b, c, block, true);
					} else {
						fps_detail::NTT::template accumulate_wide<count, false, false>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_wide<count, false, false>(words(error) + block, a, b, c, block, true);
					}
				};
				if (terms == 4) update(std::integral_constant<int, 4>{});
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
		if (terms.size() <= 32) {
			g.resize(deg);
			for (int i = 1; i < deg; ++i) {
				for (auto [j, x] : terms) {
					if (j > i) break;
					g[i] -= x * g[i - j];
				}
				g[i] *= g[0];
			}
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
		fps_detail::NTT::CompositionWords storage;
		Word *a = nullptr, *b = nullptr;
		if (deg > 96 && fps_detail::NTT::vectorized()) {
			int capacity = 64;
			while (capacity < deg && capacity < (1 << 23)) capacity <<= 1;
			storage.reset(new (std::align_val_t(64)) unsigned int[2 * capacity]);
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
				int lo = std::min<int>(m, this->size());
				std::copy_n(words(*this), lo, a);
				std::fill(a + lo, a + m, Word(0));
				std::copy_n(words(g), m, b);
				int hi = std::max(0, std::min<int>(m, this->size() - m));
				if (hi) fps_detail::NTT::inverse_fold8(a, words(*this) + m, hi);
				transform(std::max(1, m / 16)).template inverse_cubic8<false>(a, b, m);
				g.resize(n);
				std::copy_n(a, m, words(g) + m);
				lo = std::min<int>(n, this->size());
				std::copy_n(words(*this), lo, a);
				std::fill(a + lo, a + n, Word(0));
				std::copy_n(words(g), m, b);
				transform(std::max(1, n / 16)).template inverse_cubic8<true>(a, b, n);
				fps_detail::NTT::inverse_recover8(words(g), a, m);

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
			const int block = size / 32, length = 2 * block, count = (target + block - 1) / block;
			g = f.pre(block).sqrt(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word half_norm = T::mod() - (T::mod() - 1) / (2 * length);
			const int levels = __builtin_ctz((unsigned int)length);
			T root_norm = T::raw(T::mod() - (T::mod() - 1) / (1 << (levels / 2)));
			if (levels & 1) root_norm *= T::raw(441024591);
			transform(1).scale(words(inverse), length, half_norm);
			fps_detail::NTT::CompositionWords storage(new (std::align_val_t(64)) unsigned int[(count - 1) * length]);
			Word *cache = storage.get();
			base error(length);
			base zero(length);
			g.resize(target);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, target - offset);
				Word *q = cache + (i - 1) * length;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, root_norm.val());

				for (int j = 0; j < (i + 1) / 2; j += 6) {
					const Word *a[6], *b[6], *c[6];
					int terms = std::min(6, (i + 1) / 2 - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * length;
						b[k] = (j + k ? cache + (i - j - k) * length : words(zero));
						c[k] = cache + (i - j - k - 1) * length;
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
				fps_detail::NTT::square_fold(words(error), cache + (i / 2) * length, block, i & 1);
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

	static T bostan_mori(F p, F q, long long k) {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return T::raw(W::bostan_mori(import_work(p), import_work(q), k).val());
		assert(k >= 0 && !q.empty() && q[0] != T(0));
		p.trim();
		q.trim();
		if (p.empty()) return 0;
		auto coefficient = [](const base &p, const base &q, int k) {
			F inverse(q.begin(), q.begin() + std::min<int>(k + 1, q.size()));
			inverse = inverse.inv(k + 1);
			return dot(p.data(), inverse.data() + k, std::min<int>(k + 1, p.size()));
		};
		if (k < 32) return coefficient(p, q, k);
		T answer = 0;
		if (p.size() >= q.size()) {
			auto [quotient, remainder] = p.divmod(q);
			if (k < (long long)quotient.size()) answer = quotient[k];
			p = std::move(remainder);
		}
		if (p.empty()) return answer;
		if (q.size() == 2) return answer + p[0] / q[0] * (-q[1] / q[0]).pow(k);
		const int d = static_cast<int>(q.size()) - 1;
		if (k < d) return answer + coefficient(p, q, k);
		int n = 1;
		while (n < d) n <<= 1;
		assert(n <= (1 << 22));
		const T half = T(2).inv(), inv = T(n).inv();
		T correction = 1;
		const T step = T(3).pow((T::mod() - 1) / (2 * n));
		base powers(n), twists(n), a = p, b = q;
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			auto &plan = transform(n);
			for (int i = 0; i < n; ++i) words(twists)[i] = plan.inverse_root_quotients[i] * (0u - T::mod());
		} else {
			const auto &roots = inverse_ntt_roots(2 * n);
			for (int i = 0; i < n; ++i) twists[i] = roots[2 * i];
		}
		T lead = q.back();
		std::fill(powers.begin(), powers.end(), T(1));
		geometric_scale(powers, inv, step);
		a.resize(2 * n);
		b.resize(2 * n);
		ntt(a);
		ntt(b);
		auto doubling = [&](base &v, T leading) {
			if constexpr (std::is_same_v<T, fps_detail::Mint>) {
				Word *odd = words(v) + n;
				std::copy_n(words(v), n, odd);
				auto &plan = transform(std::max(1, n / 2));
				plan.template fft<true>(odd, n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
				fps_detail::NTT::product(odd, odd, words(powers), n);
				plan.template fft<false>(odd, n);
			} else {
				base odd = v;
				ntt_inverse(odd);
				odd[0] -= T(2 * n) * leading;
				pointwise(odd, odd, powers, n);
				ntt(odd);
				v.insert(v.end(), odd.begin(), odd.end());
			}
		};
		while (k) {
			if constexpr (std::is_same_v<T, fps_detail::Mint>) {
				if (k & 1) fps_detail::NTT::reduce_pairs<true>(words(a), words(b), words(twists), n);
				else fps_detail::NTT::reduce_pairs<false>(words(a), words(b), words(twists), n);
			} else {
				for (int i = 0; i < n; ++i) {
					T u = a[2 * i] * b[2 * i + 1], v = a[2 * i + 1] * b[2 * i];
					a[i] = k & 1 ? (u - v) * twists[i] : u + v;
					b[i] = b[2 * i] * b[2 * i + 1];
				}
			}
			correction *= half;
			if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
				a.resize(n);
				b.resize(n);
			}
			lead = (d & 1 ? -lead : lead) * lead;
			k >>= 1;
			if (k < d) break;
			doubling(a, T(0));
			doubling(b, d == n ? lead : T(0));
		}
		a.resize(n);
		b.resize(n);
		if (k) {
			intt(a);
			intt(b);
			if (d == n) b[0] -= lead;
			return answer + correction * coefficient(a, b, k);
		}
		T numerator = 0, denominator = 0;
		for (int i = 0; i < n; ++i) numerator += a[i], denominator += b[i];
		if (d == n) denominator -= T(n) * lead;
		return answer + correction * numerator / denominator;
	}

	static T linear_recurrence(const base &initial, const base &coefficients, long long k) {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return T::raw(W::linear_recurrence(import_work(initial), import_work(coefficients), k).val());
		assert(k >= 0 && initial.size() == coefficients.size());
		const int d = static_cast<int>(initial.size());
		if (k < d) return initial[k];
		if (d == 0) return 0;
		F q(d + 1);
		q[0] = 1;
		for (int i = 0; i < d; ++i) q[i + 1] = -coefficients[i];
		q.trim();
		if (q.size() == 1) return 0;
		F p = multiply(F(initial), q).pre(d);
		return bostan_mori(std::move(p), std::move(q), k);
	}

	F compose(const F &inner, int deg = -1) const {
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0 && (inner.empty() || inner[0] == T(0)));
		if (deg == 0) return {};
		F f = pre(deg), g = inner.pre(deg);
		f.trim();
		g.trim();
		if (f.size() <= 1 || g.size() <= 1) {
			F result(deg);
			if (!f.empty()) result[0] = f[0];
			return result;
		}
		int first = 1;
		while (g[first] == T(0)) ++first;
		if (first + 1 == static_cast<int>(g.size())) {
			F result(deg);
			T power = 1;
			for (int i = 0; i < static_cast<int>(f.size()) && (long long)i * first < deg; ++i)
				result[i * first] = f[i] * power, power *= g[first];
			return result;
		}
		if (deg <= 32 || f.size() <= 8) {
			F result;
			for (int i = (static_cast<int>(f.size())) - 1; i >= 0; --i) {
				result = multiply(result, g).pre(deg);
				result += f[i];
			}
			result.resize(deg);
			return result;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		assert(size <= (1 << 22));
		CompositionNTT transform(size);
		std::vector<unsigned int> p(2 * size), q(size);
		for (int i = 0; i < static_cast<int>(f.size()); ++i) {
			if constexpr (std::is_same_v<T, fps_detail::Mint>) p[i] = f[i].v;
			else p[i] = transform.in(f[i].val());
		}
		for (int i = 0; i < static_cast<int>(g.size()); ++i) {
			if constexpr (std::is_same_v<T, fps_detail::Mint>) q[i] = g[i].v;
			else q[i] = transform.in(g[i].val());
		}
		unsigned int a = transform.one, b = transform.one;
		transform.compose(p.data(), q.data(), 1, size, a, b);
		F result(deg);
		for (int i = 0; i < deg; ++i) {
			if constexpr (std::is_same_v<T, fps_detail::Mint>) result[i] = T::word(p[i]);
			else result[i] = T::raw(transform.out(p[i]));
		}
		return result;
	}

	F compositional_inverse(int deg = -1) const {
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0 && (this->empty() || (*this)[0] == T(0)));
		if (deg == 0) return {};
		if (deg == 1) return F(1);
		assert(this->size() >= 2 && (*this)[1] != T(0));
		if constexpr (!std::is_same_v<T, fps_detail::Mint>)
			return export_work(import_work(*this).compositional_inverse(deg));
		else {
			T w = (*this)[1].inv();
			int length = std::min<int>(deg, this->size());
			int first = 2;
			while (first < length && (*this)[first] == T(0)) ++first;
			if (first == length) {
				F result(deg);
				result[1] = w;
				return result;
			}
			int size = 1;
			while (size < deg) size <<= 1;
			assert(size <= (1 << 21));

			const int n = deg - 1;
			auto &inv = inverses(size);
			F h(n);
			{
				fps_detail::NTT t(size);
				fps_detail::NTT::CompositionWords storage(static_cast<unsigned int *>(::operator new[](8 * size * sizeof(unsigned int), std::align_val_t(64))));
				auto *p = storage.get(), *q = p + 4 * size;
				std::copy_n(words(*this), length, q);
				std::fill_n(q + length, 2 * size - length, 0u);
				t.geometric(q, length, T(-1).v, w.v);
				auto factor = t.reversion_projection(p, q, size);
				t.product(p + size - n, p + size - n, words(inv) + size - n, n);
				for (int i = 0; i < n; ++i) h[i] = T::word(p[size - 1 - i]);
				scale_words(h, T(size - 1) * T::word(factor));
			}
			h = h.log(n);
			scale_words(h, -inv[size - 1]);
			h = h.exp(n);
			scale_words(h, w);
			F result(deg);
			std::copy(h.begin(), h.end(), result.begin() + 1);
			return result;
		}
	}

	T eval(T x) const {
		T ans = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) ans = ans * x + *it;
		return ans;
	}

	std::vector<T> eval(const std::vector<T> &xs) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(W(import_work(*this).eval(import_work(xs))));
		if (xs.empty()) return {};
		if (this->size() <= 32 || xs.size() <= 32) {
			base r;
			r.reserve(xs.size());
			for (T x : xs) r.push_back(eval(x));
			return r;
		}
		constexpr int block = 8;
		int length = block;
		while (length < (int)std::max(this->size(), xs.size())) length <<= 1;
		assert(length <= (1 << 23));
		int size = length / block;
		std::vector<base> frequency;
		auto tree = product_tree(xs, size, block, true, &frequency);
		F root = *this;
		root.resize(length);
		std::reverse(root.begin(), root.end());
		root = root.div_series(tree[1], length);
		return evaluate_tree(xs, tree, frequency, size, block, std::move(root), true);
	}
	std::vector<T> multipoint_eval(const std::vector<T> &xs) const { return eval(xs); }

	static F interpolate(const std::vector<T> &xs, const std::vector<T> &ys) {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(W::interpolate(import_work(xs), import_work(ys)));
		assert(xs.size() == ys.size());
		const int n = static_cast<int>(xs.size());
		if (n == 0) return {};
		constexpr int block = 8;
		int size = 1;
		std::vector<base> frequency;
		auto tree = product_tree(xs, size, block, true, &frequency);
		const int length = size * block;
		F root(length);
		for (int i = 0; i < n; ++i) root[length - n + i] = T(n - i) * tree[1][i];
		auto flip = [&] {
			for (int i = 1; i < 2 * size; ++i) {
				std::reverse(tree[i].begin(), tree[i].end());
				base &a = frequency[i];
				if (a.empty()) continue;
				const int length = static_cast<int>(a.size()), d = static_cast<int>(tree[i].size()) - 1;
				for (int j = 2; j < length; j <<= 1) std::reverse(a.begin() + j, a.begin() + 2 * j);
				if (d == length / 2) {
					for (int j = length / 2; j < length; ++j) a[j] = -a[j];
				} else {
					const auto &roots = ntt_roots(length);
					base phase(length, T(1));
					for (int half = 1; half < length; half <<= 1) {
						const T step = roots[half].pow(d);
						for (int j = 0; j < half; ++j) phase[half + j] = phase[j] * step;
					}
					for (int j = 0; j < length; ++j) a[j] *= phase[j];
				}
			}
		};
		root = root.div_series(tree[1], length);
		base den = evaluate_tree(xs, tree, frequency, size, block, std::move(root));
		flip();
		std::vector<F> value(2 * size);
		base weights(n);
		batch_inverse(weights, den);
		pointwise(weights, weights, ys, n);
		for (int i = 0; i * block < n; ++i) {
			const int d = std::min(block, n - i * block);
			F &f = value[size + i];
			f.resize(d);
			const F &q = tree[size + i];
#if defined(__x86_64__) || defined(__i386__)
			if constexpr (std::is_same_v<T, fps_detail::Mint>) {
				if (d == 8 && fps_detail::NTT::vectorized()) {
					fps_detail::NTT::interpolate_leaf8(words(f), words(q), words(xs) + i * block, words(weights) + i * block);
					continue;
				}
			}
#endif
			for (int j = i * block; j < i * block + d; ++j) {
				T coefficient = 1;
				f[d - 1] += weights[j];
				for (int k = d - 2; k >= 0; --k) {
					coefficient = q[k + 1] + xs[j] * coefficient;
					f[k] += weights[j] * coefficient;
				}
			}
		}
		for (int i = size - 1; i >= 1; --i) {
			if (value[2 * i + 1].empty()) {
				value[i] = std::move(value[2 * i]);
				continue;
			}
			const int length = static_cast<int>(frequency[2 * i].size());
			if (length && static_cast<int>(frequency[2 * i + 1].size()) == length) {
				base a = std::move(value[2 * i]), b = std::move(value[2 * i + 1]);
				a.resize(length);
				b.resize(length);
				ntt(a);
				ntt(b);
				if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::combine(words(a), words(b), words(frequency[2 * i]), words(frequency[2 * i + 1]), length);
				else
					for (int j = 0; j < length; ++j) a[j] = a[j] * frequency[2 * i + 1][j] + b[j] * frequency[2 * i][j];
				intt(a);
				a.resize(tree[i].size() - 1);
				value[i] = std::move(a);
			} else {
				value[i] = value[2 * i] * tree[2 * i + 1] + value[2 * i + 1] * tree[2 * i];
			}
		}
		return std::move(value[1]);
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
		F derivative(std::min<int>(deg, this->size()));
		if (derivative.size() > 1) fps_detail::NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		const auto &inv = inverses(deg - 1);
		fps_detail::NTT::product(words(r), words(r), words(inv), deg);
		return r;
	}

	F exp(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).exp(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		const auto &iv = inverses(deg);
		auto terms = sparse_terms(deg, 64);
		if (terms.size() <= 64) {
			for (auto &[j, x] : terms) x *= T(j);
			F g(deg);
			g[0] = 1;
			for (int i = 1; i < deg; ++i) {
				for (auto [j, x] : terms) {
					if (j > i) break;
					g[i] += x * g[i - j];
				}
				g[i] *= iv[i];
			}
			return g;
		}
		const int initial = std::min(deg, 32);
		F g(initial);
		g[0] = 1;
		for (int i = 1; i < initial; ++i) {
			T sum = 0;
			for (int j = 1; j <= i && j < static_cast<int>(this->size()); ++j)
				sum += T(j) * (*this)[j] * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		if (deg > 256) {
			int size = 1;
			while (size < deg) size <<= 1;
			const int block = size / 32, length = 2 * block, count = (deg + block - 1) / block;
			g = pre(block).exp(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word norm = T::mod() - (T::mod() - 1) / length;
			transform(1).scale(words(inverse), length, norm);
			fps_detail::NTT::CompositionWords storage(new (std::align_val_t(64)) unsigned int[(2 * count - 1) * length]);
			Word *derivative = storage.get(), *cache = derivative + count * length;
			std::fill(derivative, derivative + block, 0u);
			base error(length);
			const int initial_count = std::min<int>(block, this->size());
			if (initial_count > 1) fps_detail::NTT::weighted(derivative + 1, words(*this) + 1, initial_count - 1, 1);
			ntt_half(derivative, length);
			g.resize(deg);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, deg - offset);
				Word *d = derivative + i * length;
				const int derivative_count = std::max(0, std::min<int>(n, static_cast<int>(this->size()) - offset));
				if (derivative_count) fps_detail::NTT::weighted(d, words(*this) + offset, derivative_count, offset);
				std::fill(d + derivative_count, d + block, 0u);
				ntt_half(d, length);
				Word *q = cache + (i - 1) * length;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, norm);
				std::fill(error.begin(), error.end(), T(0));
				for (int j = 0; j < i; j += 3) {
					const Word *a[3], *b[3], *c[3];
					int terms = std::min(3, i - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * length;
						b[k] = derivative + (i - j - k) * length;
						c[k] = derivative + (i - j - k - 1) * length;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						fps_detail::NTT::template accumulate_batch<count>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_batch<count>(words(error) + block, a, b, c, block, true);
					};
					if (terms == 3) update(std::integral_constant<int, 3>{});
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
				for (int i = 1; i < static_cast<int>(derivative.size()); ++i) derivative[i] = T(i) * (*this)[i];
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
			for (int i = 1; i < m && i < static_cast<int>(this->size()); ++i) b[i - 1] = T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			for (int i = 0; i < m - 1; ++i) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			for (int i = (std::min(n, deg)) - 1; i >= m; --i)
				error[i] = (i < static_cast<int>(this->size()) ? (*this)[i] : T(0)) + error[i - 1] * in * iv[i];
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
		f /= lead;
		f = f.log(len);
		f *= T(k);
		f = f.exp(len);
		f *= lead.pow(k);
		F r(n);
		for (int i = 0; i < len; ++i) r[shift + i] = f[i];
		return r;
	}

	F shift(T c) {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			W f = import_work(*this);
			f.shift(fps_detail::Mint::raw(c.val()));
			return *this = export_work(f);
		}
		const int n = static_cast<int>(this->size());
		if (n <= 1 || c == T(0)) return *this;
		assert(unsigned(n) < unsigned(T::mod()));
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			F a(n), b(n), weights;
			Word fact = fps_detail::NTT::one;
			for (int i = 0; i < n; ++i) {
				words(a)[i] = fact;
				fact = (unsigned long long)fact * (i + 1) % T::mod();
			}
			const Word last = T::word(words(a)[n - 1]).inv().v;
			Word invfact = last;
			for (int i = n - 1; i >= 0; --i) {
				words(b)[i] = invfact;
				invfact = (unsigned long long)invfact * i % T::mod();
			}
			fps_detail::NTT::product(words(a), words(a), words(*this), n);
			std::reverse(a.begin(), a.end());
			weights = b;
			geometric_scale(b, T(1), c);
			F prod = multiply(a, b);
			std::reverse(prod.begin(), prod.begin() + n);
			fps_detail::NTT::product(words(*this), words(prod), words(weights), n);
			return *this;
		}
		const auto &inv = inverses(n);
		F a(n), b(n);
		T fact = 1, invfact = 1, cpow = 1;
		for (int i = 0; i < n; ++i) {
			a[n - 1 - i] = (*this)[i] * fact;
			b[i] = cpow * invfact;
			fact *= i + 1;
			invfact *= inv[i + 1];
			cpow *= c;
		}
		F prod = multiply(a, b);
		invfact = 1;
		for (int i = 0; i < n; ++i) {
			(*this)[i] = prod[n - 1 - i] * invfact;
			invfact *= inv[i + 1];
		}
		return *this;
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
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		for (int i = 0; i < std::min<int>(a.size(), product.size()); ++i) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

  private:
	static W import_work(const base &a) {
		W r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = fps_detail::Mint::raw(a[i].val());
		return r;
	}
	static F export_work(const W &a) {
		F r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = T::raw(a[i].val());
		return r;
	}
	static fps_detail::NTT &transform(int n) { return fps_detail::ntt_plan(n); }
	using Word = fps_detail::NTT::U;
	static Word *words(base &a) {
		static_assert(sizeof(T) == sizeof(Word) && std::is_trivially_copyable_v<T>);
		return reinterpret_cast<Word *>(a.data());
	}
	static const Word *words(const base &a) { return reinterpret_cast<const Word *>(a.data()); }
	template <bool inverse>
	static void transform_inplace(base &a) {
		int n = a.size();
		assert(n && !(n & (n - 1)) && n <= (1 << 23));
		auto &t = transform(std::max(1, n / 2));
		t.template fft<inverse>(words(a), n);
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) t.normalize(words(a), n);
	}
	static void ntt(base &a) { transform_inplace<false>(a); }
	static void ntt_half(Word *a, int n) {
		std::copy_n(a, n / 2, a + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(a, n, n / 2);
	}
	static void ntt_half(base &a) {
		int n = a.size();
		std::copy_n(a.begin(), n / 2, a.begin() + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(words(a), n, n / 2);
	}
	static void ntt_inverse(base &a) { transform_inplace<true>(a); }
	static void scale_words(base &a, T factor) {
		if (a.empty()) return;
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			auto &t = transform(1);
			t.scale(words(a), a.size(), factor.val());
		} else
			for (T &x : a) x *= factor;
	}
	static void geometric_scale(base &a, T first, T step) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::geometric(words(a), a.size(), first.v, step.v);
		else
			for (T &x : a) {
				x *= first;
				first *= step;
			}
	}
	static void batch_inverse(base &dst, const base &src) {
		int n = src.size();
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 128 && fps_detail::NTT::vectorized()) {
				fps_detail::NTT::batch_inverse8(words(dst), words(src), n);
				return;
			}
		}
#endif
		base prefix(n);
		T p = 1;
		for (int i = 0; i < n; ++i) prefix[i] = p, p *= src[i];
		assert(p != T(0));
		p = p.inv();
		for (int i = n - 1; i >= 0; --i) {
			dst[i] = prefix[i] * p;
			p *= src[i];
		}
	}
	static void pointwise(base &r, const base &a, const base &b, int n) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::product(words(r), words(a), words(b), n);
		else
			for (int i = 0; i < n; ++i) r[i] = a[i] * b[i];
	}
	std::vector<std::pair<int, T>> sparse_terms(int deg, int limit) const {
		std::vector<std::pair<int, T>> terms;
		for (int i = 1; i < std::min<int>(deg, this->size()) && static_cast<int>(terms.size()) <= limit; ++i)
			if ((*this)[i] != T(0)) terms.emplace_back(i, (*this)[i]);
		return terms;
	}
	static base evaluate_tree(const base &points, const std::vector<F> &tree,
							  std::vector<base> &frequency, int size, int block, F root, bool consume = false) {
		base result(points.size());
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (block == 8 && fps_detail::NTT::vectorized()) {
				const int total = size * block;
				auto &plan = transform(std::max(1, total / 2));
				fps_detail::NTT::CompositionWords scratch;
				if (!consume) scratch.reset(new (std::align_val_t(64)) unsigned int[2 * total]);
				for (int level = 1; level <= size; level <<= 1) {
					const int length = total / level, half = length / 2;
					for (int node = level; node < 2 * level; ++node) {
						const int offset = (node - level) * length;
						if (offset >= static_cast<int>(points.size())) break;
						Word *cur = words(root) + offset;
						if (length == 8) {
							fps_detail::NTT::evaluate_leaf8(words(result) + offset, cur, words(tree[node]), tree[node].size(), words(points) + offset, std::min<int>(8, points.size() - offset));
						} else if (offset + half >= static_cast<int>(points.size())) {
							std::copy_n(cur + half, half, cur);
						} else if (!frequency[2 * node].empty()) {
							Word *a = consume ? words(frequency[2 * node]) : scratch.get();
							Word *b = consume ? words(frequency[2 * node + 1]) : scratch.get() + total;
							if (!consume) {
								std::copy_n(words(frequency[2 * node]), length, a);
								std::copy_n(words(frequency[2 * node + 1]), length, b);
							}
							plan.template fft<false>(cur, length);
							fps_detail::NTT::product(a, a, cur, length);
							fps_detail::NTT::product(b, b, cur, length);
							plan.template fft<true>(a, length);
							plan.template fft<true>(b, length);
							const Word norm = T::mod() - (T::mod() - 1) / length;
							fps_detail::NTT::scale_copy(cur, b + half, half, norm);
							fps_detail::NTT::scale_copy(cur + half, a + half, half, norm);
						} else {
							alignas(32) Word left[8], right[8];
							assert(length == 16);
							fps_detail::NTT::evaluate_middle8(left, right, cur, words(tree[2 * node + 1]), tree[2 * node + 1].size(), words(tree[2 * node]), tree[2 * node].size());
							std::copy_n(left, 8, cur);
							std::copy_n(right, 8, cur + 8);
						}
					}
				}
				return result;
			}
		}
#endif
		auto dfs = [&](auto &&self, int node, int l, int r, F cur) -> void {
			if (l * block >= static_cast<int>(points.size())) return;
			const int length = (r - l) * block, half = length / 2;
			if (r - l == 1 || length <= 8) {
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (std::is_same_v<T, fps_detail::Mint>) {
					if (length == 8 && fps_detail::NTT::vectorized()) {
						const int count = std::min<int>(8, points.size() - l * block);
						fps_detail::NTT::evaluate_leaf8(words(result) + l * block, words(cur), words(tree[node]), tree[node].size(), words(points) + l * block, count);
						return;
					}
				}
#endif
				base polynomial(length);
				for (int i = 0; i < length; ++i)
					for (int j = 0; j + i < length && j < static_cast<int>(tree[node].size()); ++j)
						polynomial[length - 1 - i - j] += cur[i] * tree[node][j];
				for (int i = l * block; i < std::min<int>(r * block, points.size()); ++i) {
					T value = 0;
					for (int j = length - 1; j >= 0; --j) value = value * points[i] + polynomial[j];
					result[i] = value;
				}
				return;
			}
			const int mid = (l + r) / 2;
			if (mid * block >= static_cast<int>(points.size())) {
				self(self, 2 * node, l, mid, F(cur.begin() + half, cur.end()));
				return;
			}
			F left, right;
			if (!frequency[2 * node].empty()) {
				ntt(static_cast<base &>(cur));
				F a = consume ? std::move(frequency[2 * node]) : frequency[2 * node];
				F b = consume ? std::move(frequency[2 * node + 1]) : frequency[2 * node + 1];
				pointwise(a, a, cur, length);
				pointwise(b, b, cur, length);
				ntt_inverse(a);
				ntt_inverse(b);
				const Word norm = T::mod() - (T::mod() - 1) / length;
				fps_detail::NTT::scale_copy(words(b), words(b) + half, half, norm);
				fps_detail::NTT::scale_copy(words(a), words(a) + half, half, norm);
				b.resize(half);
				a.resize(half);
				left = std::move(b);
				right = std::move(a);
			} else {
				left.resize(half);
				right.resize(half);
				bool done = false;
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (std::is_same_v<T, fps_detail::Mint>) {
					if (length == 16 && fps_detail::NTT::vectorized()) {
						fps_detail::NTT::evaluate_middle8(words(left), words(right), words(cur), words(tree[2 * node + 1]), tree[2 * node + 1].size(), words(tree[2 * node]), tree[2 * node].size());
						done = true;
					}
				}
#endif
				if (!done) {
					F a = multiply(cur, tree[2 * node + 1]), b = multiply(cur, tree[2 * node]);
					std::copy_n(a.begin() + half, half, left.begin());
					std::copy_n(b.begin() + half, half, right.begin());
				}
			}
			self(self, 2 * node, l, mid, std::move(left));
			self(self, 2 * node + 1, mid, r, std::move(right));
		};
		dfs(dfs, 1, 0, size, std::move(root));
		return result;
	}
	static void inverse_extend(F &h, const base &a, const base &cache) {
		const int m = static_cast<int>(cache.size());
		const T im = T::raw(T::mod() - (T::mod() - 1) / m);
		base b(m);
		pointwise(b, a, cache, m);
		ntt_inverse(b);
		std::fill(b.begin(), b.begin() + m / 2, T(0));
		ntt(b);
		pointwise(b, b, cache, m);
		ntt_inverse(b);
		h.resize(m);
		for (int i = m / 2; i < m; ++i) h[i] = -b[i] * im * im;
	}
	static T dot(const T *a, const T *b, int n) {

#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 64 && fps_detail::NTT::vectorized())
				return T::word(fps_detail::NTT::reverse_dot8(reinterpret_cast<const Word *>(a), reinterpret_cast<const Word *>(b), n));
		}
#endif
		unsigned int result = 0;
		for (int i = 0; i < n; i += 16) {
			unsigned long long sum = 0;
			for (int j = i; j < std::min(n, i + 16); ++j)
				sum += (unsigned long long)a[j].val() * b[-j].val();
			result += sum % T::mod();
			if (result >= T::mod()) result -= T::mod();
		}
		return T::raw(result);
	}

	using CompositionNTT = fps_detail::NTT;
	static void intt(base &a) {
		ntt_inverse(a);
		const T inv = T::raw(T::mod() - (T::mod() - 1) / static_cast<int>(a.size()));
		scale_words(a, inv);
	}
	static T mod_sqrt(T a) {
		if (a == T(0) || a.pow((T::mod() - 1) / 2) != T(1)) return 0;
		T r = a.pow(60), t = a.pow(119), c = T(3).pow(119);
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T u = t; u != T(1); u *= u) ++i;
			T b = c.pow(1 << (m - i - 1));
			r *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return r;
	}
	static const base &inverse_ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m)).inv();
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const base &ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m));
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static std::vector<F> product_tree(const std::vector<T> &xs, int &size, int block,
									   bool reciprocal,
									   std::vector<std::vector<T>> *frequency = nullptr) {
		const int groups = (static_cast<int>(xs.size()) + block - 1) / block;
		while (size < groups) size <<= 1;
		std::vector<F> tree(2 * size);
		if (frequency) frequency->resize(2 * size);
		for (int i = 0; i < size; ++i) {
			if (i >= groups) {
				tree[size + i] = {1};
				continue;
			}
			F p;
			p.reserve(block + 1);
			p.push_back(T(1));
			for (int j = i * block; j < std::min<int>((i + 1) * block, xs.size()); ++j) {
				const int d = static_cast<int>(p.size()) - 1;
				if (reciprocal) {
					p.push_back(0);
					for (int k = d + 1; k >= 1; --k) p[k] -= xs[j] * p[k - 1];
				} else {
					p.push_back(p.back());
					for (int k = d; k >= 1; --k) p[k] = p[k - 1] - xs[j] * p[k];
					p[0] *= -xs[j];
				}
			}
			tree[size + i] = std::move(p);
		}
		for (int i = size - 1; i >= 1; --i) {
			if (tree[2 * i + 1].size() == 1 && tree[2 * i + 1][0] == T(1)) {
				tree[i] = tree[2 * i];
				continue;
			}
			int depth = 31 - __builtin_clz((unsigned int)i);
			int length = (size >> depth) * block;
			if (frequency && length >= 32) {
				auto &a = (*frequency)[2 * i];
				auto &b = (*frequency)[2 * i + 1];
				auto transform = [&](base &a, const F &p) {
					if (a.empty()) {
						a.resize(length);
						std::copy(p.begin(), p.end(), a.begin());
						ntt(a);
					} else {
						const int half = length / 2;
						base odd(half);
						std::copy_n(p.begin(), std::min<int>(half, p.size()), odd.begin());
						if (static_cast<int>(p.size()) > half) odd[0] -= p[half];
						geometric_scale(odd, T(1), ntt_roots(length)[length / 2]);
						ntt(odd);
						a.insert(a.end(), odd.begin(), odd.end());
					}
				};
				transform(a, tree[2 * i]);
				transform(b, tree[2 * i + 1]);
				std::vector<T> c(length);
				pointwise(c, a, b, length);
				if (i > 1) (*frequency)[i] = c;
				ntt_inverse(c);
				const int n = (int)(tree[2 * i].size() + tree[2 * i + 1].size() - 1);
				tree[i].resize(n);
				const T inv = T::raw(T::mod() - (T::mod() - 1) / length);
				if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::scale_copy(words(tree[i]), words(c), std::min(n, length), inv.val());
				else
					for (int j = 0; j < std::min(n, length); ++j) tree[i][j] = c[j] * inv;
				if (n == length + 1) {
					const T lead = tree[2 * i].back() * tree[2 * i + 1].back();
					tree[i][0] -= lead;
					tree[i][length] = lead;
				}
			} else {
				tree[i] = multiply(tree[2 * i], tree[2 * i + 1]);
			}
		}
		return tree;
	}
	void trim() {
		while (!this->empty() && this->back() == T(0)) this->pop_back();
	}
	static const std::vector<T> &inverses(int n) {
		assert(unsigned(n) < unsigned(T::mod()));
		static std::vector<T> inv{0, 1};
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			int first = inv.size(), count = (n + 1 - first) / 8 * 8;
			if (count >= 256 && fps_detail::NTT::vectorized()) {
				inv.reserve(n + 1);
				inv.resize(first + count);
				fps_detail::NTT::inverse_sequence8(reinterpret_cast<Word *>(inv.data()) + first, first, count);
			}
		}
#endif
		while (static_cast<int>(inv.size()) <= n) {
			const int i = static_cast<int>(inv.size());
			inv.push_back(-inv[T::mod() % i] * (T::mod() / i));
		}
		return inv;
	}
};

using fps = FormalPowerSeries<fps_mint>;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
