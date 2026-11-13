#ifndef STRING_WILDCARD_PATTERN_MATCHING_HPP
#define STRING_WILDCARD_PATTERN_MATCHING_HPP

#include "../components/wildcard_pattern_matching.hpp"
#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

struct WildcardOptions {
	std::optional<unsigned char> text_wildcard = '*';
	std::optional<unsigned char> pattern_wildcard = '*';
};

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, WildcardOptions options) {
	if (text.size() > std::size_t(INT_MAX) || pattern.size() > (std::size_t(1) << 22))
		throw std::length_error("wildcard_pattern_matching: string too long");
	const std::size_t n = text.size(), m = pattern.size();
	if (m > n) return {};
	std::vector<bool> matches(n - m + 1, true);
	if (!m) return matches;
	auto wildcard = [](unsigned char c, std::optional<unsigned char> w) { return w && c == *w; };
	if (n == m) {
		for (std::size_t i = 0; i < m; ++i)
			if (text[i] != pattern[i] && !wildcard(text[i], options.text_wildcard) && !wildcard(pattern[i], options.pattern_wildcard)) {
				matches[0] = false;
				break;
			}
		return matches;
	}
	if (m <= 64) {
		std::array<std::uint64_t, 256> masks{};
		std::uint64_t any = 0, state = 0, last = std::uint64_t(1) << (m - 1);
		for (std::size_t i = 0; i < m; ++i) {
			unsigned char c = pattern[i];
			if (wildcard(c, options.pattern_wildcard)) any |= std::uint64_t(1) << i;
			else masks[c] |= std::uint64_t(1) << i;
		}
		for (auto &mask : masks) mask |= any;
		for (std::size_t i = 0; i < n; ++i) {
			unsigned char c = text[i];
			state = ((state << 1) | 1) & (wildcard(c, options.text_wildcard) ? ~std::uint64_t(0) : masks[c]);
			if (i + 1 >= m) matches[i + 1 - m] = (state & last) != 0;
		}
		return matches;
	}
	std::array<bool, 256> used{};
	std::array<std::size_t, 256> frequency{};
	bool text_wild = false, pattern_wild = false;
	std::size_t literals = 0, text_literals = 0;
	for (unsigned char c : text) {
		if (wildcard(c, options.text_wildcard)) text_wild = true;
		else { used[c] = true; ++frequency[c]; ++text_literals; }
	}
	for (unsigned char c : pattern) {
		if (wildcard(c, options.pattern_wildcard)) pattern_wild = true;
		else { used[c] = true; ++literals; }
	}
	if (!text_wild && !pattern_wild) {
		std::vector<int> prefix(m);
		for (std::size_t i = 1, j = 0; i < m; ++i) {
			while (j && pattern[i] != pattern[j]) j = prefix[j - 1];
			if (pattern[i] == pattern[j]) ++j;
			prefix[i] = static_cast<int>(j);
		}
		std::fill(matches.begin(), matches.end(), false);
		for (std::size_t i = 0, j = 0; i < n; ++i) {
			while (j && text[i] != pattern[j]) j = prefix[j - 1];
			if (text[i] == pattern[j]) ++j;
			if (j == m) { matches[i + 1 - m] = true; j = prefix[j - 1]; }
		}
		return matches;
	}
	std::array<std::uint16_t, 256> codes{};
	std::uint16_t alphabet = 0;
	for (std::size_t c = 0; c < used.size(); ++c)
		if (used[c]) codes[c] = ++alphabet;
	if (alphabet <= 1 || !literals || !text_literals) return matches;
	std::vector<std::uint16_t> a(n), b(m);
	for (std::size_t i = 0; i < n; ++i) {
		unsigned char c = text[i];
		a[i] = wildcard(c, options.text_wildcard) ? 0 : codes[c];
	}
	std::size_t first = m;
	for (std::size_t i = 0; i < m; ++i) {
		unsigned char c = pattern[i];
		if (!wildcard(c, options.pattern_wildcard)) {
			b[i] = codes[c];
			if (first == m || frequency[c] < frequency[static_cast<unsigned char>(pattern[first])]) first = i;
		}
	}
	std::vector<int> candidates;
	candidates.reserve(matches.size());
	for (std::size_t i = 0; i < matches.size(); ++i) candidates.push_back(static_cast<int>(i));
	std::uint64_t budget = 8 * (std::uint64_t(n) + m);
	auto filter = [&](std::size_t position) {
		if (candidates.size() > budget) return false;
		budget -= candidates.size();
		std::size_t kept = 0;
		for (int start : candidates) {
			auto value = a[std::size_t(start) + position];
			if (!value || value == b[position]) candidates[kept++] = start;
			else matches[start] = false;
		}
		candidates.resize(kept);
		return true;
	};
	filter(first);
	for (std::size_t i = 0; i < m && !candidates.empty(); ++i)
		if (b[i] && i != first && !filter(i)) {
			std::uint64_t bound = std::uint64_t(literals) * (alphabet - 1) * (alphabet - 1);
			wildcard_matching_detail::filter_ntt(a, b, matches, bound, text_wild, pattern_wild);
			break;
		}
	return matches;
}

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_pattern_matching(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, WildcardOptions options) {
	auto matches = wildcard_pattern_matching(text, pattern, options);
	std::vector<int> positions;
	for (std::size_t i = 0; i < matches.size(); ++i)
		if (matches[i]) positions.push_back(static_cast<int>(i));
	return positions;
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_find_all(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}

#endif
