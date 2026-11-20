#ifndef MODINT_HPP
#define MODINT_HPP

#include "modular_arithmetic.hpp"
#include <cassert>
#include <istream>
#include <ostream>
#include <type_traits>

namespace modint_detail {
template <std::uint32_t Mod> struct StaticModulus {
	static_assert(Mod > 0);
	static constexpr std::uint32_t mod() { return Mod; }
	static constexpr std::uint32_t mul(std::uint32_t a, std::uint32_t b) { return std::uint64_t(a) * b % Mod; }
};
template <int Id> struct DynamicModulus {
	static inline Barrett32 reducer{998244353};
	static std::uint32_t mod() { return reducer.mod; }
	static std::uint32_t mul(std::uint32_t a, std::uint32_t b) { return reducer.mul(a, b); }
};
template <class Policy> class [[gnu::warn_unused]] Modint {
	std::uint32_t value_ = 0;
public:
	constexpr Modint() = default;
	template <class I, std::enable_if_t<std::is_integral_v<I> || std::is_same_v<I, __int128_t> || std::is_same_v<I, __uint128_t>, int> = 0>
	constexpr Modint(I x) {
		if constexpr (std::is_signed_v<I> || std::is_same_v<I, __int128_t>) {
			auto r = __int128_t(x) % mod();
			value_ = std::uint32_t(r < 0 ? r + mod() : r);
		} else value_ = std::uint32_t(__uint128_t(x) % mod());
	}
	[[nodiscard]] static constexpr std::uint32_t mod() { return Policy::mod(); }
	static void set_mod(std::uint32_t m) requires requires { Policy::reducer; } { Policy::reducer = Barrett32(m); }
	[[nodiscard]] static constexpr Modint raw(std::uint32_t x) { assert(x < mod()); Modint result; result.value_ = x; return result; }
	[[nodiscard]] constexpr std::uint32_t val() const { return value_; }
	constexpr Modint &operator+=(Modint x) {
		std::uint64_t sum = std::uint64_t(value_) + x.value_;
		value_ = std::uint32_t(sum >= mod() ? sum - mod() : sum);
		return *this;
	}
	constexpr Modint &operator-=(Modint x) { value_ = value_ >= x.value_ ? value_ - x.value_ : std::uint32_t(std::uint64_t(value_) + mod() - x.value_); return *this; }
	constexpr Modint &operator*=(Modint x) { value_ = Policy::mul(value_, x.value_); return *this; }
	Modint &operator/=(Modint x) { return *this *= x.inv(); }
	[[nodiscard]] constexpr Modint operator+() const { return *this; }
	[[nodiscard]] constexpr Modint operator-() const { return raw(value_ ? mod() - value_ : 0); }
	constexpr Modint &operator++() { return *this += 1; }
	constexpr Modint &operator--() { return *this -= 1; }
	constexpr Modint operator++(int) { auto old = *this; ++*this; return old; }
	constexpr Modint operator--(int) { auto old = *this; --*this; return old; }
	[[nodiscard]] friend constexpr Modint operator+(Modint a, Modint b) { return a += b; }
	[[nodiscard]] friend constexpr Modint operator-(Modint a, Modint b) { return a -= b; }
	[[nodiscard]] friend constexpr Modint operator*(Modint a, Modint b) { return a *= b; }
	[[nodiscard]] friend Modint operator/(Modint a, Modint b) { return a /= b; }
	[[nodiscard]] friend constexpr bool operator==(Modint, Modint) = default;
	[[nodiscard]] constexpr Modint pow(std::int64_t n) const {
		if (n < 0) throw std::invalid_argument("modint exponent");
		Modint result(1), x = *this;
		for (; n; n >>= 1, x *= x) if (n & 1) result *= x;
		return result;
	}
	[[nodiscard]] Modint inv() const { return raw(std::uint32_t(inv_mod(value_, mod()))); }
	friend std::istream &operator>>(std::istream &in, Modint &x) { std::int64_t value; if (in >> value) x = value; return in; }
	friend std::ostream &operator<<(std::ostream &out, Modint x) { return out << x.val(); }
};
}

template <std::uint32_t Mod> using StaticModint = modint_detail::Modint<modint_detail::StaticModulus<Mod>>;
template <int Id = 0> using DynamicModint = modint_detail::Modint<modint_detail::DynamicModulus<Id>>;

#endif
