#ifndef MODULAR_ARITHMETIC_HPP
#define MODULAR_ARITHMETIC_HPP

#include "linear_congruence.hpp"
#include "../components/barrett32.hpp"
#include <stdexcept>

inline std::uint32_t pow_mod(std::int64_t x, std::int64_t n, std::uint32_t m) {
	if (!m || n < 0) throw std::invalid_argument("pow_mod arguments");
	std::int64_t r = x % std::int64_t(m);
	if (r < 0) r += m;
	return Barrett32(m).pow(std::uint32_t(r), std::uint64_t(n));
}

constexpr std::int64_t inv_mod(std::int64_t x, std::int64_t m) {
	auto result = linear_congruence(x, 1, m);
	if (!result) throw std::domain_error("inverse does not exist");
	return result->residue;
}

#endif
