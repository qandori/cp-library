#ifndef BINOMIAL_MOD_HPP
#define BINOMIAL_MOD_HPP

#include "../components/barrett32.hpp"
#include "linear_congruence.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <vector>

class BinomialMod {
	using U = std::uint32_t;
	using W = std::uint64_t;
	struct PrimePower {
		U prime, exponent, coefficient;
		Barrett32 modulus;
		std::vector<U> factorial, inverse, powers;
		PrimePower(U p, U e, U q, U limit, U weight)
			: prime(p), exponent(e), coefficient(weight), modulus(q),
			  factorial(std::size_t(std::min(limit, q - 1)) + 1), inverse(factorial.size()), powers(e, 1) {
			factorial[0] = 1;
			for (std::size_t i = 1; i < factorial.size(); ++i)
				factorial[i] = i % p ? modulus.mul(factorial[i - 1], U(i)) : factorial[i - 1];
			inverse.back() = U(linear_congruence(factorial.back(), 1, q)->residue);
			for (std::size_t i = factorial.size() - 1; i; --i)
				inverse[i - 1] = i % p ? modulus.mul(inverse[i], U(i)) : inverse[i];
			for (U i = 1; i < e; ++i) powers[i] = U(W(powers[i - 1]) * p);
		}
		template<bool Binary> U choose_power(W n, W k) const {
			struct Level { W n, k, r; };
			std::array<Level, 64> levels;
			W r = n - k;
			U valuation = 0;
			bool negative = false;
			if constexpr (Binary) {
				valuation = U(std::popcount(k) + std::popcount(r) - std::popcount(n));
				if (valuation >= exponent) return 0;
				negative = exponent == 2 && ((valuation - ((n >> 1) - (k >> 1) - (r >> 1))) & 1);
			}
			U length = 0;
			while (n) {
				levels[length++] = {n, k, r};
				if constexpr (Binary) n >>= 1, k >>= 1, r >>= 1;
				else {
					n /= prime, k /= prime, r /= prime;
					U carry = U(n - k - r);
					valuation += carry;
					if (valuation >= exponent) return 0;
					if (length >= exponent) negative ^= carry != 0;
				}
			}
			U a = 1, b = 1, c = 1;
			for (U i = 0; i < length; ++i) {
				auto value = levels[i];
				if constexpr (Binary) value.n &= modulus.mod - 1, value.k &= modulus.mod - 1, value.r &= modulus.mod - 1;
				else if (i + exponent < length) {
					auto q = levels[i + exponent];
					value.n -= q.n * modulus.mod;
					value.k -= q.k * modulus.mod;
					value.r -= q.r * modulus.mod;
				}
				a = modulus.mul(a, factorial[std::size_t(value.n)]);
				b = modulus.mul(b, inverse[std::size_t(value.k)]);
				c = modulus.mul(c, inverse[std::size_t(value.r)]);
			}
			U result = modulus.mul(modulus.mul(a, b), c);
			if (negative) result = modulus.mod - result;
			return modulus.mul(result, powers[valuation]);
		}
		template<bool Single = false> U choose(W n, W k) const {
			if constexpr (Single) if (n < prime) return modulus.mul(factorial[n], modulus.mul(inverse[k], inverse[n - k]));
			if (exponent == 1) {
				if (prime == 2) return (n & k) == k;
				U result = 1;
				while (n) {
					U a = U(n % prime), b = U(k % prime);
					if (a < b) return 0;
					result = modulus.mul(result, modulus.mul(factorial[a], modulus.mul(inverse[b], inverse[a - b])));
					n /= prime, k /= prime;
				}
				return result;
			}
			return prime == 2 ? choose_power<true>(n, k) : choose_power<false>(n, k);
		}
	};
	Barrett32 modulus_;
	std::vector<PrimePower> factors_;
	std::vector<U> pascal_;
	std::int64_t max_n_ = std::numeric_limits<std::int64_t>::max();
	int pascal_limit_ = -1;
	static constexpr U small_limit_ = 64, small_full_limit_ = 32;
	static std::size_t row(U n) { return std::size_t((n + 1) / 2) * ((n + 2) / 2); }
	template<bool Wide> void build_pascal(U limit) {
		pascal_limit_ = int(limit);
		pascal_.resize(row(limit + 1));
		const U m = mod();
		auto add = [m](U a, U b) {
			U sum = a + b;
			if constexpr (Wide) return sum >= m || sum < a ? sum - m : sum;
			else return std::min(sum, sum - m);
		};
		pascal_[0] = 1;
		for (U n = 1; n <= limit; ++n) {
			const U *prev = pascal_.data() + row(n - 1);
			U *curr = pascal_.data() + row(n);
			curr[0] = 1;
			for (U k = 1; k <= (n - 1) / 2; ++k) curr[k] = add(prev[k - 1], prev[k]);
			if (!(n & 1)) curr[n / 2] = add(prev[n / 2 - 1], prev[n / 2 - 1]);
		}
	}
	static bool full_tables_fit(U m, U max_q) {
		for (U p = 2; m > max_q && p <= max_q && W(p) * p <= m; p += p == 2 ? 1 : 2) {
			if (m % p) continue;
			U before = m;
			do { m /= p; } while (m % p == 0);
			if (before / m > max_q) return false;
		}
		return m <= max_q;
	}
	static U checked_(U m) {
		if (!m) throw std::invalid_argument("BinomialMod modulus");
		return m;
	}

public:
	explicit BinomialMod(U modulus, U table_limit = 1000000) : modulus_(checked_(modulus)) {
		if (modulus > 2 && table_limit <= small_limit_) {
			const bool full = full_tables_fit(modulus, table_limit + 1);
			if (!full || table_limit <= small_full_limit_) {
				if (modulus <= (U(1) << 31)) build_pascal<false>(table_limit);
				else build_pascal<true>(table_limit);
			}
			if (!full) { max_n_ = table_limit; return; }
		}
		auto add = [&](U p, U e, U q) {
			U weight = U(W(modulus / q) * W(linear_congruence(modulus / q, 1, q)->residue));
			factors_.emplace_back(p, e, q, table_limit, weight);
			if (q - 1 > table_limit) max_n_ = table_limit;
		};
		U remaining = modulus;
		for (U p = 2; p <= remaining / p; p += p == 2 ? 1 : 2) {
			if (remaining % p) continue;
			U q = 1, e = 0;
			do { remaining /= p; q *= p; ++e; } while (remaining % p == 0);
			add(p, e, q);
		}
		if (remaining > 1) add(remaining, 1, remaining);
	}
	U mod() const { return modulus_.mod; }
	std::int64_t max_n() const { return max_n_; }
	U comb(std::int64_t n, std::int64_t k) const {
		if (n < 0 || k < 0 || n < k || mod() == 1) return 0;
		if (n > max_n_) throw std::out_of_range("BinomialMod n");
		if (!k || n == k) return 1;
		if (n <= pascal_limit_) return pascal_[row(U(n)) + std::min(k, n - k)];
		if (factors_.size() == 1) return factors_[0].choose<true>(W(n), W(k));
		U answer = 0;
		for (const auto &f : factors_) {
			W sum = W(answer) + modulus_.mul(f.coefficient, f.choose(W(n), W(k)));
			answer = U(sum < mod() ? sum : sum - mod());
		}
		return answer;
	}
	U operator()(std::int64_t n, std::int64_t k) const { return comb(n, k); }
};

#endif
