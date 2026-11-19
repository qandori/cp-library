#ifndef CRT_HPP
#define CRT_HPP

#include "linear_congruence.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

struct CRTSolution {
	std::int64_t residue, modulus;
	bool operator==(const CRTSolution &) const = default;
};

constexpr std::optional<CRTSolution> crt(std::int64_t r0, std::int64_t m0, std::int64_t r1, std::int64_t m1) {
	if (m0 <= 0 || m1 <= 0) throw std::invalid_argument("crt modulus");
	r0 %= m0;
	r1 %= m1;
	if (r0 < 0) r0 += m0;
	if (r1 < 0) r1 += m1;
	if (m0 < m1) {
		std::swap(r0, r1);
		std::swap(m0, m1);
	}
	if (m0 % m1 == 0) {
		if (r0 % m1 != r1) return std::nullopt;
		return CRTSolution{r0, m0};
	}
	auto solution = linear_congruence(m0, r1 - r0, m1);
	if (!solution) return std::nullopt;
	__int128_t modulus = __int128_t(m0) * solution->period;
	if (modulus > std::numeric_limits<std::int64_t>::max()) throw std::overflow_error("crt modulus");
	return CRTSolution{std::int64_t(r0 + __int128_t(m0) * solution->residue), std::int64_t(modulus)};
}

namespace crt_detail {
template<class I>
std::optional<CRTSolution> solve(const std::vector<I> &residues, const std::vector<I> &moduli) {
	if (residues.size() != moduli.size()) throw std::invalid_argument("crt size");
	for (auto m : moduli) if (m <= 0) throw std::invalid_argument("crt modulus");
	CRTSolution result{0, 1};
	for (std::size_t i = 0; i < residues.size(); ++i) {
		auto next = crt(result.residue, result.modulus, residues[i], moduli[i]);
		if (!next) return std::nullopt;
		result = *next;
	}
	return result;
}
}

inline std::optional<CRTSolution> crt(const std::vector<std::int64_t> &residues, const std::vector<std::int64_t> &moduli) {
	return crt_detail::solve(residues, moduli);
}

namespace cp_acl {
inline std::pair<long long, long long> crt(const std::vector<long long> &r, const std::vector<long long> &m) {
	if (auto result = crt_detail::solve(r, m)) return {result->residue, result->modulus};
	return {0, 0};
}
}

#endif
