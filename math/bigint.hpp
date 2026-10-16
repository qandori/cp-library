#ifndef BIGINT_HPP
#define BIGINT_HPP

#include "../components/ntt.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <charconv>
#include <compare>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

class BigInt {
	using U = std::uint64_t;
	using W = __uint128_t;
	using D = std::vector<U>;
	static constexpr U base = 1000000000000000000ULL;
	D a;
	bool negative = false;
	template <class I>
	static constexpr bool integral = std::is_integral_v<I> || std::is_same_v<I, __int128_t> || std::is_same_v<I, __uint128_t>;
	static void trim(D &v) {
		while (!v.empty() && !v.back()) v.pop_back();
	}
	void normalize() {
		trim(a);
		if (a.empty()) negative = false;
	}
	static int compare(const D &a, const D &b) {
		if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
		for (std::size_t i = a.size(); i--;) if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
		return 0;
	}
	static void add(D &a, const D &b) {
		std::size_t n = a.size(), m = b.size();
		a.resize(std::max(n, m));
		U carry = 0;
		for (std::size_t i = 0; i < m || carry; ++i) {
			if (i == a.size()) a.push_back(0);
			U value = a[i] + (i < m ? b[i] : 0) + carry;
			carry = value >= base;
			a[i] = value - carry * base;
		}
	}
	static void subtract(D &a, const D &b) {
		U borrow = 0;
		for (std::size_t i = 0; i < b.size() || borrow; ++i) {
			U value = (i < b.size() ? b[i] : 0) + borrow;
			borrow = a[i] < value;
			a[i] = a[i] + borrow * base - value;
		}
		trim(a);
	}
	static void multiply_small(D &a, U b) {
		if (!b) return a.clear();
		W carry = 0;
		for (auto &x : a) {
			W value = W(x) * b + carry;
			carry = base_quotient(value);
			x = value - carry * base;
		}
		if (carry) a.push_back(carry);
	}
	static W high_product(W a, W b) {
		W low = W(U(a)) * U(b);
		W middle = W(U(a)) * U(b >> 64) + (low >> 64);
		W upper = W(U(a >> 64)) * U(b) + U(middle);
		return (a >> 64) * (b >> 64) + (middle >> 64) + (upper >> 64);
	}
	static W base_quotient(W value) {
		constexpr W inverse = std::numeric_limits<W>::max() / base;
		W q = high_product(value, inverse);
		return q + (value - q * base >= base);
	}
	static U divide_small(D &a, U b) {
		assert(b);
		if (b == 1) { trim(a); return 0; }
		W inverse = std::numeric_limits<W>::max() / b, remainder = 0;
		for (std::size_t i = a.size(); i--;) {
			W value = remainder * base + a[i];
			U q = high_product(value, inverse);
			remainder = value - W(q) * b;
			if (remainder >= b) ++q, remainder -= b;
			a[i] = q;
		}
		trim(a);
		return remainder;
	}
	static D schoolbook(const D &a, const D &b) {
		D c(a.size() + b.size());
		W carry = 0;
		bool square = &a == &b;
		for (std::size_t k = 0; k + 1 < c.size(); ++k) {
			std::size_t begin = k >= b.size() ? k + 1 - b.size() : 0;
			std::size_t end = std::min(k + 1, a.size());
			W value = carry;
			if (square) {
				for (std::size_t i = begin; i < std::min(end, (k + 1) / 2); ++i) value += W(a[i]) * a[k - i] * 2;
				if (!(k & 1) && k / 2 < a.size()) value += W(a[k / 2]) * a[k / 2];
			} else for (std::size_t i = begin; i < end; ++i) value += W(a[i]) * b[k - i];
			carry = base_quotient(value);
			c[k] = value - carry * base;
		}
		c.back() = carry;
		trim(c);
		return c;
	}
	static void karatsuba(const U *a, const U *b, W *c, int n, W *work, U *scratch, int na, int nb) {
		std::fill(c, c + 2 * n, W(0));
		if (!na || !nb) return;
		if (n <= 16) {
			if (a == b) {
				for (int i = 0; i < na; ++i) {
					c[2 * i] += W(a[i]) * a[i];
					for (int j = 0; j < i; ++j) c[i + j] += W(a[i]) * a[j] * 2;
				}
			} else for (int i = 0; i < na; ++i)
				for (int j = 0; j < nb; ++j) c[i + j] += W(a[i]) * b[j];
			return;
		}
		int m = n / 2;
		karatsuba(a, b, c, m, work, scratch, std::min(na, m), std::min(nb, m));
		karatsuba(a + m, b + m, c + n, m, work, scratch, std::max(0, na - m), std::max(0, nb - m));
		auto *x = scratch;
		auto *y = a == b ? x : x + m;
		for (int i = 0; i < m; ++i) x[i] = a[i] + a[i + m];
		if (x != y) for (int i = 0; i < m; ++i) y[i] = b[i] + b[i + m];
		karatsuba(x, y, work, m, work + n, scratch + n, std::min(na, m), std::min(nb, m));
		for (int i = 0; i < n; ++i) work[i] -= c[i] + c[i + n];
		for (int i = 0; i < n; ++i) c[i + m] += work[i];
	}
	static D karatsuba(const D &a, const D &b) {
		int n = std::bit_ceil(std::max(a.size(), b.size()));
		auto words = std::make_unique_for_overwrite<U[]>(4 * n);
		auto buffer = std::make_unique_for_overwrite<W[]>(4 * n);
		U *x = words.get(), *y = &a == &b ? x : x + n;
		std::copy(a.begin(), a.end(), x);
		std::fill(x + a.size(), x + n, U(0));
		if (x != y) {
			std::copy(b.begin(), b.end(), y);
			std::fill(y + b.size(), y + n, U(0));
		}
		W *result = buffer.get();
		karatsuba(x, y, result, n, result + 2 * n, x + 2 * n, a.size(), b.size());
		D c(a.size() + b.size());
		W carry = 0;
		for (std::size_t i = 0; i < c.size(); ++i) {
			carry += result[i];
			W q = base_quotient(carry);
			c[i] = carry - q * base;
			carry = q;
		}
		trim(c);
		return c;
	}
	static void add_shifted(D &a, const D &b, std::size_t shift) {
		a.resize(std::max(a.size(), b.size() + shift));
		U carry = 0;
		for (std::size_t i = 0; i < b.size() || carry; ++i) {
			if (i + shift == a.size()) a.push_back(0);
			U value = a[i + shift] + (i < b.size() ? b[i] : 0) + carry;
			carry = value >= base;
			a[i + shift] = value - carry * base;
		}
	}
	template <unsigned radix, int width>
	static bool centered_fits(const D &a, const D &b) {
		U sa = 0, sb = 0, ma = 0, mb = 0, ca = 0, cb = 0;
		auto scan = [](U value, U &carry, U &sum, U &maximum) {
			value += carry;
			for (int j = 0; j < width; ++j) {
				int digit = value % radix;
				value /= radix;
				int adjust = digit > int(radix / 2);
				digit -= adjust * int(radix);
				value += adjust;
				U magnitude = digit < 0 ? -digit : digit;
				sum += magnitude;
				maximum = std::max(maximum, magnitude);
			}
			carry = value;
		};
		for (std::size_t i = 0; i < std::max(a.size(), b.size()); ++i) {
			if (i < a.size()) scan(a[i], ca, sa, ma);
			if (i < b.size()) scan(b[i], cb, sb, mb);
			if (std::min(sa, sb) > (i + 1) * width * (radix / 16)) return false;
			if (std::min(W(sa) * mb, W(sb) * ma) * 2 >= fps_detail::NTT::mod) return false;
		}
		sa += ca; sb += cb;
		ma = std::max(ma, ca); mb = std::max(mb, cb);
		return std::min(W(sa) * mb, W(sb) * ma) * 2 < fps_detail::NTT::mod;
	}
	template <unsigned radix, int width, bool centered = false>
	static D transform_product(const D &a, const D &b) {
		using N = fps_detail::NTT;
		auto chunks = [](const D &v) {
			std::size_t n = (v.size() - 1) * width;
			for (U x = v.back(); x; x /= radix) ++n;
			return n;
		};
		std::size_t length = centered ? (a.size() + b.size()) * width + 1 : chunks(a) + chunks(b) - 1;
		if (length > (1U << 23)) {
			const D &u = a.size() >= b.size() ? a : b, &v = a.size() >= b.size() ? b : a;
			std::size_t half = u.size() / 2;
			D low(u.begin(), u.begin() + half), high(u.begin() + half, u.end());
			trim(low);
			D c = multiply(low, v), upper = multiply(high, v);
			add_shifted(c, upper, half);
			return c;
		}
		int n = std::bit_ceil(length);
		bool square = &a == &b;
		std::vector<std::uint32_t> p(n), q(square ? 0 : n);
		auto fill = [](auto &p, const D &v) {
			std::size_t offset = 0, length = 0;
			U carry = 0;
			for (U limb : v) {
				U value = limb + carry;
				for (int j = 0; j < width; ++j) {
					int digit = value % radix;
					value /= radix;
					if constexpr (centered) {
						int adjust = digit > int(radix / 2);
						digit -= adjust * int(radix);
						value += adjust;
					}
					p[offset++] = digit < 0 ? digit + int(N::mod) : digit;
					if (digit) length = offset;
				}
				carry = value;
			}
			if (carry) p[offset++] = carry, length = offset;
			return length;
		};
		std::size_t na = fill(p, a), nb = square ? na : fill(q, b);
		length = na + nb - 1;
		n = std::bit_ceil(length);
		p.resize(n);
		if (!square) q.resize(n);
		auto &t = fps_detail::ntt_plan(std::max(1, n / 2));
		t.fft<false>(p.data(), n);
		if (!square) t.fft<false>(q.data(), n);
		N::product(p.data(), p.data(), square ? p.data() : q.data(), n);
		t.fft<true>(p.data(), n);
		N::scale_normalize(p.data(), n, std::uint64_t(fps_mint(n).inv().val()) * N::one % N::mod);
		D c;
		c.reserve(a.size() + b.size());
		using C = std::conditional_t<centered, std::int64_t, U>;
		C carry = 0;
		U value = 0, scale = 1;
		for (std::size_t i = 0; i < length || carry; ++i) {
			C digit = i < length ? p[i] : 0;
			if constexpr (centered) digit -= C(digit > N::mod / 2) * N::mod;
			carry += digit;
			C remainder = carry % radix;
			carry /= radix;
			if constexpr (centered) {
				C borrow = remainder < 0;
				remainder += borrow * radix;
				carry -= borrow;
			}
			value += remainder * scale;
			scale *= radix;
			if (scale == base) c.push_back(value), value = 0, scale = 1;
		}
		if (value) c.push_back(value);
		trim(c);
		return c;
	}
	static D multiply(const D &a, const D &b) {
		if (a.empty() || b.empty()) return {};
		if (!a[0] || !b[0]) {
			std::size_t na = 0, nb = 0;
			while (!a[na]) ++na;
			while (!b[nb]) ++nb;
			D x(a.begin() + na, a.end()), y(b.begin() + nb, b.end());
			D c = multiply(x, y);
			c.insert(c.begin(), na + nb, 0);
			return c;
		}
		if (std::min(a.size(), b.size()) == 1) {
			D c = a.size() >= b.size() ? a : b;
			multiply_small(c, a.size() < b.size() ? a[0] : b[0]);
			return c;
		}
		if (std::min(a.size(), b.size()) <= 8 || std::max(a.size(), b.size()) <= 32) return schoolbook(a, b);
		if (std::max(a.size(), b.size()) <= 256) return karatsuba(a, b);
		std::size_t shorter = std::min(a.size(), b.size());
		if (centered_fits<1000, 6>(a, b)) return transform_product<1000, 6, true>(a, b);
		if (shorter * 9 <= 998244352 / 9801) return transform_product<100, 9>(a, b);
		if (centered_fits<100, 9>(a, b)) return transform_product<100, 9, true>(a, b);
		return transform_product<10, 18>(a, b);
	}
	static std::pair<D, D> divide_knuth(D a, D b) {
		std::size_t n = a.size(), m = b.size();
		U factor = base / (b.back() + 1);
		multiply_small(a, factor);
		multiply_small(b, factor);
		a.resize(n + 1);
		D q(n - m + 1);
		W inverse = std::numeric_limits<W>::max() / b.back();
		for (std::size_t j = q.size(); j--;) {
			W value = W(a[j + m]) * base + a[j + m - 1];
			U digit = high_product(value, inverse);
			if (value - W(digit) * b.back() >= b.back()) ++digit;
			digit = std::min(digit, base - 1);
			W remainder = value - W(digit) * b.back();
			while (remainder < base && W(digit) * b[m - 2] > remainder * base + a[j + m - 2]) --digit, remainder += b.back();
			W borrow = 0;
			for (std::size_t i = 0; i < m; ++i) {
				W product = W(digit) * b[i] + borrow;
				W next = base_quotient(product);
				U low = product - next * base;
				borrow = next + (a[j + i] < low);
				a[j + i] = a[j + i] + (a[j + i] < low ? base : 0) - low;
			}
			bool over = a[j + m] < borrow;
			a[j + m] = a[j + m] + (over ? base : 0) - borrow;
			if (over) {
				--digit;
				U carry = 0;
				for (std::size_t i = 0; i < m; ++i) {
					U sum = a[j + i] + b[i] + carry;
					carry = sum >= base;
					a[j + i] = sum - carry * base;
				}
				a[j + m] += carry;
			}
			q[j] = digit;
		}
		a.resize(m);
		divide_small(a, factor);
		trim(q);
		return {std::move(q), std::move(a)};
	}
	static D reciprocal(const D &a) {
		std::size_t n = a.size();
		if (n <= 8) {
			D power(2 * n + 1);
			power.back() = 1;
			return divide_knuth(std::move(power), a).first;
		}
		std::size_t half = (n + 1) / 2;
		D high(a.end() - half, a.end());
		D x = reciprocal(high);
		x.insert(x.begin(), n - half, 0);
		D product = multiply(a, x), power(2 * n + 1);
		power.back() = 1;
		int order = compare(power, product);
		if (order >= 0) subtract(power, product);
		else subtract(product, power), power = std::move(product);
		D correction = multiply(x, power);
		bool rounded = false;
		if (order < 0) {
			for (std::size_t i = 0; i < std::min(2 * n, correction.size()); ++i) rounded |= correction[i] != 0;
		}
		if (correction.size() <= 2 * n) correction.clear();
		else correction.erase(correction.begin(), correction.begin() + 2 * n);
		if (rounded) add(correction, D{1});
		if (order >= 0) add(x, correction);
		else subtract(x, correction);
		product = multiply(a, x);
		power.assign(2 * n + 1, 0);
		power.back() = 1;
		assert(compare(power, product) >= 0);
		subtract(power, product);
		while (compare(power, a) >= 0) subtract(power, a), add(x, D{1});
		return x;
	}
	static std::pair<D, D> divide(const D &a, const D &b) {
		assert(!b.empty());
		if (compare(a, b) < 0) return {{}, a};
		if (b.size() == 1) {
			D q = a;
			U remainder = divide_small(q, b[0]);
			return {std::move(q), remainder ? D{remainder} : D{}};
		}
		if (b.size() <= 16 || a.size() - b.size() <= 16) return divide_knuth(a, b);
		U factor = base / (b.back() + 1);
		D u = a, v = b;
		multiply_small(u, factor);
		multiply_small(v, factor);
		D inverse = reciprocal(v), quotient(u.size()), remainder;
		std::size_t n = v.size(), blocks = (u.size() + n - 1) / n;
		for (std::size_t block = blocks; block--;) {
			std::size_t begin = block * n, end = std::min(begin + n, u.size());
			remainder.insert(remainder.begin(), u.begin() + begin, u.begin() + end);
			trim(remainder);
			if (compare(remainder, v) < 0) continue;
			D q = multiply(remainder, inverse);
			if (q.size() <= 2 * n) q.clear();
			else q.erase(q.begin(), q.begin() + 2 * n);
			D product = multiply(q, v);
			subtract(remainder, product);
			while (compare(remainder, v) >= 0) subtract(remainder, v), add(q, D{1});
			std::copy(q.begin(), q.end(), quotient.begin() + begin);
		}
		trim(quotient);
		divide_small(remainder, factor);
		return {std::move(quotient), std::move(remainder)};
	}

public:
	BigInt() = default;
	BigInt(const BigInt &) = default;
	BigInt &operator=(const BigInt &) = default;
	BigInt(BigInt &&b) noexcept : a(std::move(b.a)), negative(std::exchange(b.negative, false)) {}
	BigInt &operator=(BigInt &&b) noexcept {
		if (this != &b) a = std::move(b.a), negative = std::exchange(b.negative, false);
		return *this;
	}
	template <class I> requires integral<I>
	BigInt(I value) {
		if constexpr (std::numeric_limits<I>::is_signed) negative = value < 0;
		__uint128_t magnitude = value;
		if (negative) magnitude = -magnitude;
		while (magnitude) a.push_back(magnitude % base), magnitude /= base;
	}
	explicit BigInt(std::string_view value) {
		assert(!value.empty());
		std::size_t begin = value[0] == '-' || value[0] == '+';
		negative = value[0] == '-';
		assert(begin < value.size());
		a.reserve((value.size() - begin + 17) / 18);
		for (std::size_t end = value.size(); end > begin;) {
			std::size_t start = end - begin > 18 ? end - 18 : begin;
			U digit = 0;
			for (std::size_t i = start; i < end; ++i) {
				assert(value[i] >= '0' && value[i] <= '9');
				digit = digit * 10 + value[i] - '0';
			}
			a.push_back(digit);
			end = start;
		}
		normalize();
	}
	std::string str() const {
		if (a.empty()) return "0";
		std::string result(a.size() * 18 + negative, '0');
		char *out = result.data();
		if (negative) *out++ = '-';
		auto first = std::to_chars(out, out + 18, a.back());
		out = first.ptr;
		for (std::size_t i = a.size() - 1; i--;) {
			U value = a[i];
			for (int j = 17; j >= 0; --j) out[j] = '0' + value % 10, value /= 10;
			out += 18;
		}
		result.resize(out - result.data());
		return result;
	}
	int sign() const { return a.empty() ? 0 : negative ? -1 : 1; }
	explicit operator bool() const { return !a.empty(); }
	template <class I> requires integral<I>
	explicit operator I() const {
		__uint128_t value = 0;
		for (std::size_t i = a.size(); i--;) value = value * base + a[i];
		return I(negative ? -value : value);
	}
	friend bool operator==(const BigInt &x, const BigInt &y) { return x.negative == y.negative && x.a == y.a; }
	friend std::strong_ordering operator<=>(const BigInt &x, const BigInt &y) {
		int order = x.negative != y.negative ? (x.negative ? -1 : 1) : compare(x.a, y.a) * (x.negative ? -1 : 1);
		return order <=> 0;
	}
	BigInt operator-() const {
		BigInt result = *this;
		if (!a.empty()) result.negative = !negative;
		return result;
	}
	BigInt operator+() const { return *this; }
	BigInt &operator+=(const BigInt &b) {
		if (negative == b.negative) add(a, b.a);
		else if (compare(a, b.a) >= 0) subtract(a, b.a);
		else {
			D result = b.a;
			subtract(result, a);
			a = std::move(result);
			negative = b.negative;
		}
		if (a.empty()) negative = false;
		return *this;
	}
	BigInt &operator-=(const BigInt &b) {
		if (negative != b.negative) add(a, b.a);
		else if (compare(a, b.a) >= 0) subtract(a, b.a);
		else {
			D result = b.a;
			subtract(result, a);
			a = std::move(result);
			negative = !negative;
		}
		if (a.empty()) negative = false;
		return *this;
	}
	BigInt &operator*=(const BigInt &b) {
		bool sign = negative != b.negative;
		a = multiply(a, b.a);
		negative = !a.empty() && sign;
		return *this;
	}
	template <class I> requires integral<I>
	BigInt &operator*=(I value) {
		bool sign = false;
		if constexpr (std::numeric_limits<I>::is_signed) sign = value < 0;
		__uint128_t magnitude = value;
		if (sign) magnitude = -magnitude;
		if (magnitude >= base) return *this *= BigInt(value);
		multiply_small(a, U(magnitude));
		negative = !a.empty() && (negative != sign);
		return *this;
	}
	friend std::pair<BigInt, BigInt> divmod(const BigInt &x, const BigInt &y) {
		auto [q, r] = divide(x.a, y.a);
		BigInt quotient, remainder;
		quotient.a = std::move(q);
		remainder.a = std::move(r);
		quotient.negative = !quotient.a.empty() && x.negative != y.negative;
		remainder.negative = !remainder.a.empty() && x.negative;
		return {std::move(quotient), std::move(remainder)};
	}
	BigInt &operator/=(const BigInt &b) {
		*this = divmod(*this, b).first;
		return *this;
	}
	BigInt &operator%=(const BigInt &b) {
		*this = divmod(*this, b).second;
		return *this;
	}
	friend BigInt operator+(BigInt a, const BigInt &b) { a += b; return a; }
	friend BigInt operator-(BigInt a, const BigInt &b) { a -= b; return a; }
	friend BigInt operator*(const BigInt &a, const BigInt &b) {
		BigInt result;
		result.a = multiply(a.a, b.a);
		result.negative = !result.a.empty() && a.negative != b.negative;
		return result;
	}
	template <class I> requires integral<I>
	friend BigInt operator*(BigInt a, I b) { a *= b; return a; }
	template <class I> requires integral<I>
	friend BigInt operator*(I a, BigInt b) { b *= a; return b; }
	friend BigInt operator/(const BigInt &a, const BigInt &b) { return divmod(a, b).first; }
	friend BigInt operator%(const BigInt &a, const BigInt &b) { return divmod(a, b).second; }
	BigInt &operator++() { return *this += 1; }
	BigInt operator++(int) { BigInt old = *this; ++*this; return old; }
	BigInt &operator--() { return *this -= 1; }
	BigInt operator--(int) { BigInt old = *this; --*this; return old; }
	friend BigInt abs(BigInt value) { value.negative = false; return value; }
	friend BigInt pow(BigInt value, std::uint64_t exponent) {
		BigInt result = 1;
		for (; exponent; exponent >>= 1) {
			if (exponent & 1) result *= value;
			if (exponent > 1) value *= value;
		}
		return result;
	}
	friend std::istream &operator>>(std::istream &in, BigInt &value) {
		std::string text;
		if (in >> text) value = BigInt(text);
		return in;
	}
	friend std::ostream &operator<<(std::ostream &out, const BigInt &value) { return out << value.str(); }
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
