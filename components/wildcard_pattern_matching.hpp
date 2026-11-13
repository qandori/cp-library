#ifndef COMPONENTS_WILDCARD_PATTERN_MATCHING_HPP
#define COMPONENTS_WILDCARD_PATTERN_MATCHING_HPP

#include "ntt_mod_core.hpp"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace wildcard_matching_detail {

template <std::uint32_t mod>
void filter_prime(const std::vector<std::uint16_t> &text, const std::vector<std::uint16_t> &pattern,
		std::vector<bool> &matches, bool text_has_wildcard, bool pattern_has_wildcard) {
	std::size_t m = pattern.size(), length = std::bit_ceil(std::min(text.size(), 2 * m - 1));
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::Plan<mod, 3> plan(length, vectorized);
	std::vector<std::uint32_t> values(length), mask(pattern_has_wildcard ? length : 0), squares(text_has_wildcard ? length : 0);
	std::uint64_t pattern_square_sum = 0;
	for (std::size_t i = 0; i < m; ++i) {
		auto value = pattern[m - 1 - i];
		values[i] = value;
		if (pattern_has_wildcard) mask[i] = value != 0;
		if (text_has_wildcard) squares[i] = std::uint32_t(value) * value;
		pattern_square_sum += std::uint32_t(value) * value;
	}
	ntt_mod_detail::forward(values, plan, vectorized);
	if (pattern_has_wildcard) ntt_mod_detail::forward(mask, plan, vectorized);
	if (text_has_wildcard) ntt_mod_detail::forward(squares, plan, vectorized);
	std::uint32_t scale = (std::uint64_t(1) << 32) % mod * (mod - (mod - 1) / length) % mod;
	std::uint32_t constant = text_has_wildcard ? 0 : pattern_square_sum % mod;
	std::vector<std::uint32_t> work(length), result(length);
	for (std::size_t start = 0; start < matches.size(); start += length - m + 1) {
		std::size_t size = std::min(length, text.size() - start), count = size - m + 1;
		auto first = matches.begin() + start, last = first + count;
		if (std::find(first, last, true) == last) continue;
		for (std::size_t i = 0; i < size; ++i) result[i] = text[start + i];
		std::fill(result.begin() + size, result.end(), 0);
		ntt_mod_detail::forward(result, plan, vectorized);
		ntt_mod_detail::pointwise<mod>(result, values, vectorized);
		for (auto &value : result) {
			std::uint32_t twice = value + value;
			if (twice >= mod) twice -= mod;
			value = twice ? mod - twice : 0;
		}
		for (int channel = 0; channel < 2; ++channel) {
			if (channel == 0 ? !pattern_has_wildcard : !text_has_wildcard) continue;
			for (std::size_t i = 0; i < size; ++i) {
				auto value = text[start + i];
				work[i] = channel == 0 ? std::uint32_t(value) * value : value != 0;
			}
			std::fill(work.begin() + size, work.end(), 0);
			ntt_mod_detail::forward(work, plan, vectorized);
			ntt_mod_detail::pointwise<mod>(work, channel == 0 ? mask : squares, vectorized);
			for (std::size_t i = 0; i < length; ++i) {
				std::uint32_t sum = result[i] + work[i];
				result[i] = sum < mod ? sum : sum - mod;
			}
		}
		ntt_mod_detail::backward(result, plan, vectorized);
		std::reverse(result.begin() + 1, result.end());
		ntt_mod_detail::scale<mod>(result, scale, vectorized);
		std::uint64_t window = 0;
		if (!pattern_has_wildcard)
			for (std::size_t i = 0; i < m; ++i) window += std::uint32_t(text[start + i]) * text[start + i];
		for (std::size_t i = 0; i < count; ++i) {
			if (matches[start + i]) {
				std::uint32_t value = result[m - 1 + i] + constant;
				if (value >= mod) value -= mod;
				if (!pattern_has_wildcard) value = (value + window) % mod;
				if (value != 0) matches[start + i] = false;
			}
			if (!pattern_has_wildcard && i + 1 < count) {
				window -= std::uint32_t(text[start + i]) * text[start + i];
				window += std::uint32_t(text[start + i + m]) * text[start + i + m];
			}
		}
	}
}

inline void filter_ntt(const std::vector<std::uint16_t> &text, const std::vector<std::uint16_t> &pattern,
		std::vector<bool> &matches, std::uint64_t bound, bool text_has_wildcard, bool pattern_has_wildcard) {
	if (pattern.size() > (std::size_t(1) << 22)) throw std::length_error("wildcard pattern is too long");
	if (std::find(matches.begin(), matches.end(), true) == matches.end()) return;
	filter_prime<998244353>(text, pattern, matches, text_has_wildcard, pattern_has_wildcard);
	if (bound >= 998244353 && std::find(matches.begin(), matches.end(), true) != matches.end())
		filter_prime<469762049>(text, pattern, matches, text_has_wildcard, pattern_has_wildcard);
}

}

#endif
