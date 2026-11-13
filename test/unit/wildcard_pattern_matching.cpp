#include "string/wildcard_pattern_matching.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

std::vector<bool> brute(std::string_view text, std::string_view pattern, WildcardOptions options = {}) {
	if (pattern.size() > text.size()) return {};
	std::vector<bool> result(text.size() - pattern.size() + 1, true);
	for (std::size_t i = 0; i < result.size(); ++i) {
		for (std::size_t j = 0; j < pattern.size(); ++j) {
			auto a = static_cast<unsigned char>(text[i + j]);
			auto b = static_cast<unsigned char>(pattern[j]);
			if (a != b && options.text_wildcard != a && options.pattern_wildcard != b) {
				result[i] = false;
				break;
			}
		}
	}
	return result;
}

std::vector<int> positions(const std::vector<bool> &flags) {
	std::vector<int> result;
	for (int i = 0; i < int(flags.size()); ++i) if (flags[i]) result.push_back(i);
	return result;
}

void check(std::string_view text, std::string_view pattern, WildcardOptions options = {}, bool overloads = false) {
	auto expected = brute(text, pattern, options);
	assert(wildcard_pattern_matching(text, pattern, options) == expected);
	assert(wildcard_find_all(text, pattern, options) == positions(expected));
	if (overloads && options.text_wildcard && options.text_wildcard == options.pattern_wildcard) {
		char wildcard = char(*options.text_wildcard);
		assert(wildcard_pattern_matching(text, pattern, wildcard) == expected);
		assert(wildcard_find_all(text, pattern, wildcard) == positions(expected));
		if (wildcard == '*') {
			assert(wildcard_pattern_matching(text, pattern) == expected);
			assert(wildcard_find_all(text, pattern) == positions(expected));
		}
	}
}

std::vector<std::string> words(int maximum) {
	std::vector<std::string> result{std::string()};
	std::size_t begin = 0;
	for (int length = 0; length < maximum; ++length) {
		std::size_t end = result.size();
		for (std::size_t i = begin; i < end; ++i)
			for (char c : {'a', 'b', '*'}) result.push_back(result[i] + c);
		begin = end;
	}
	return result;
}

void exhaustive() {
	auto strings = words(5);
	for (const auto &text : strings)
		for (const auto &pattern : strings) check(text, pattern);
	strings = words(3);
	for (auto tw : {std::optional<unsigned char>('*'), std::optional<unsigned char>()}) {
		for (auto pw : {std::optional<unsigned char>('*'), std::optional<unsigned char>()}) {
			WildcardOptions options{tw, pw};
			for (const auto &text : strings)
				for (const auto &pattern : strings) check(text, pattern, options);
		}
	}
}

void bytes_and_views() {
	check({}, {}, {}, true);
	check({}, "*", {}, true);
	check("abc", {}, {}, true);
	check("abc", "****", {}, true);
	check("ab", "*", {}, true);
	check("ab", "**", {}, true);
	check("abc", "a*c", {}, true);
	check("a*c", "abc", {}, true);
	check("abc", "a**c", {}, true);
	check("a*?b", "a?*b", WildcardOptions{std::nullopt, std::nullopt});
	check("a*?b", "a?*b", WildcardOptions{'*', '?'});
	check("a*?b", "a?*b", WildcardOptions{'?', '*'});
	std::string bytes;
	for (int i = 0; i < 256; ++i) bytes += char(i);
	for (int wildcard = 0; wildcard < 256; ++wildcard) {
		WildcardOptions options{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)};
		check(bytes + bytes, bytes, options, true);
		check(bytes + bytes, std::string(1, char(wildcard)), options, true);
		check(std::string(256, char(wildcard)), bytes, options);
		check(bytes, std::string(256, char(wildcard)), options);
	}
	for (int first : {0, 1, 42, 127, 128, 255}) {
		for (int second : {0, 1, 63, 127, 128, 255}) {
			std::string text = bytes + bytes;
			std::string pattern = bytes.substr(17, 175);
			for (std::size_t i = 0; i < text.size(); i += 13) text[i] = char(first);
			for (std::size_t i = 0; i < pattern.size(); i += 7) pattern[i] = char(second);
			check(text, pattern, WildcardOptions{static_cast<unsigned char>(first), static_cast<unsigned char>(second)});
		}
	}
	for (int offset = 0; offset < 8; ++offset) {
		std::string text = bytes + bytes, pattern = bytes;
		for (int length : {0, 1, 2, 31, 32, 33, 127, 128, 129, 240}) {
			check(std::string_view(text).substr(offset, 300), std::string_view(pattern).substr(7 - offset, length));
			check(std::string_view(text).substr(offset, length), std::string_view(pattern).substr(7 - offset, length), WildcardOptions{std::nullopt, std::nullopt});
		}
	}
}

void randomized() {
	std::mt19937_64 rng(902581634);
	for (int trial = 0; trial < 1800; ++trial) {
		int n = int(rng() % 250), m = int(rng() % 270);
		int alphabet = std::array<int, 6>{1, 2, 3, 7, 26, 256}[trial % 6];
		std::string text(n, '\0'), pattern(m, '\0');
		for (char &c : text) c = char(rng() % alphabet);
		for (char &c : pattern) c = char(rng() % alphabet);
		auto tw = static_cast<unsigned char>(rng()), pw = static_cast<unsigned char>(rng());
		WildcardOptions options{trial % 5 == 0 ? std::nullopt : std::optional<unsigned char>(tw),
			trial % 7 == 0 ? std::nullopt : std::optional<unsigned char>(pw)};
		int density = std::array<int, 5>{0, 1, 25, 90, 100}[trial % 5];
		if (options.text_wildcard) for (char &c : text) if (int(rng() % 100) < density) c = char(tw);
		if (options.pattern_wildcard) for (char &c : pattern) if (int(rng() % 100) < density) c = char(pw);
		check(text, pattern, options);
		if (trial % 13 == 0) check(text, pattern, {}, true);
	}
}

void boundaries() {
	std::mt19937 rng(957362);
	for (int m : {1, 2, 3, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023}) {
		for (int n : {m - 1, m, m + 1, 2 * m - 1, 2 * m, 8 * m + 17}) {
			std::string text(n, 'a'), pattern(m, 'a');
			for (char &c : text) c = char('a' + rng() % 3);
			int at = n >= m ? int(rng() % (n - m + 1)) : 0;
			if (n >= m) pattern = text.substr(at, m);
			for (std::size_t i = 0; i < text.size(); ++i) if (rng() % 3 == 0) text[i] = '*';
			for (char &c : pattern) if (rng() % 3 == 0) c = '*';
			check(text, pattern);
			if (n >= m) assert(wildcard_pattern_matching(text, pattern)[at]);
			check(pattern, text);
		}
	}
	for (int m : {255, 256, 257, 1000, 1023, 1024, 1025, 4095, 4096, 4097}) {
		int n = 9 * m + 19;
		std::string text(n, 'a'), pattern(m, 'a');
		for (std::size_t i = 0; i < text.size(); i += 17) text[i] = '*';
		for (std::size_t i = 0; i < pattern.size(); i += 13) pattern[i] = '*';
		std::vector<bool> expected(n - m + 1, true);
		assert(wildcard_pattern_matching(text, pattern) == expected);
		pattern[m / 2] = 'b';
		check(text, pattern);
		for (int at : {0, m - 1, m, 2 * m - 1, n - m}) text[at + m / 2] = 'b';
		check(text, pattern);
	}
}

void long_inputs() {
	int n = 120000, m = 35000;
	std::string text(n, 'a'), pattern(m, '*');
	assert(wildcard_pattern_matching(text, pattern) == std::vector<bool>(n - m + 1, true));
	text.assign(n, '*'); pattern.assign(m, 'z');
	assert(wildcard_pattern_matching(text, pattern) == std::vector<bool>(n - m + 1, true));
	text.assign(n, 'a'); pattern.assign(m, 'a');
	assert(wildcard_pattern_matching(text, pattern) == std::vector<bool>(n - m + 1, true));
	pattern.back() = 'b';
	assert(wildcard_find_all(text, pattern).empty());
	pattern.assign(m, '*'); pattern[0] = 'b'; pattern[m / 2] = 'c'; pattern.back() = 'd';
	std::vector<int> expected{17, m + 39, 2 * m + 71};
	for (int at : expected) { text[at] = 'b'; text[at + m / 2] = 'c'; text[at + m - 1] = 'd'; }
	assert(wildcard_find_all(text, pattern) == expected);
	check("a*b", "a?b", WildcardOptions{std::nullopt, '?'});
	check("a?b", "a*b", WildcardOptions{'?', std::nullopt});
}

void check_ntt(const std::vector<std::uint16_t> &text, const std::vector<std::uint16_t> &pattern) {
	std::vector<bool> expected(text.size() - pattern.size() + 1, true);
	for (std::size_t i = 0; i < expected.size(); ++i)
		for (std::size_t j = 0; j < pattern.size(); ++j)
			if (text[i + j] && pattern[j] && text[i + j] != pattern[j]) { expected[i] = false; break; }
	auto matches = std::vector<bool>(expected.size(), true);
	bool tw = std::find(text.begin(), text.end(), 0) != text.end();
	bool pw = std::find(pattern.begin(), pattern.end(), 0) != pattern.end();
	wildcard_matching_detail::filter_ntt(text, pattern, matches, 65025ULL * pattern.size(), tw, pw);
	assert(matches == expected);
	matches.assign(expected.size(), true);
	for (std::size_t i = 0; i < matches.size(); i += 3) matches[i] = expected[i] = false;
	wildcard_matching_detail::filter_ntt(text, pattern, matches, 65025ULL * pattern.size(), tw, pw);
	assert(matches == expected);
}

void ntt_backend() {
	std::mt19937 rng(790261);
	for (int m : {1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 513, 1025}) {
		for (int n : {m, 2 * m + 3, 9 * m + 13}) {
			for (int mode = 0; mode < 4; ++mode) {
				std::vector<std::uint16_t> text(n), pattern(m);
				for (int i = 0; i < n; ++i) text[i] = 1 + i % 5;
				for (int i = 0; i < m; ++i) pattern[i] = 1 + i % 5;
				if (mode & 1) for (auto &value : text) if (rng() % 3 == 0) value = 0;
				if (mode & 2) for (auto &value : pattern) if (rng() % 3 == 0) value = 0;
				check_ntt(text, pattern);
			}
		}
	}
	std::vector<std::uint16_t> text{0}, pattern{0};
	for (int i = 0; i < 15472; ++i) { text.push_back(255); pattern.push_back(1); }
	for (int difference : {229, 18, 6}) { text.push_back(1 + difference); pattern.push_back(1); }
	text.push_back(0); pattern.push_back(0);
	std::uint64_t score = 0;
	for (std::size_t i = 0; i < text.size(); ++i) {
		int difference = int(text[i]) - pattern[i];
		score += difference * difference;
	}
	assert(score == 998244353);
	std::vector<bool> matches(1, true);
	wildcard_matching_detail::filter_prime<998244353>(text, pattern, matches, true, true);
	assert(matches[0]);
	check_ntt(text, pattern);
	check_ntt(pattern, text);
	check_ntt(pattern, pattern);
	text.assign(131075, 0); pattern.assign(4097, 0);
	matches.assign(text.size() - pattern.size() + 1, true);
	wildcard_matching_detail::filter_ntt(text, pattern, matches, 65025ULL * pattern.size(), true, true);
	assert(std::all_of(matches.begin(), matches.end(), [](bool value) { return value; }));
}

void limits() {
	std::string pattern((1 << 22) + 1, '*');
	bool threw = false;
	try { wildcard_pattern_matching({}, pattern); } catch (const std::length_error &) { threw = true; }
	assert(threw);
	threw = false;
	try { wildcard_find_all({}, pattern, WildcardOptions{}); } catch (const std::length_error &) { threw = true; }
	assert(threw);
}

int main() {
	exhaustive();
	bytes_and_views();
	randomized();
	boundaries();
	long_inputs();
	ntt_backend();
	limits();
	std::cout << "Wildcard pattern matching passed\n";
}
