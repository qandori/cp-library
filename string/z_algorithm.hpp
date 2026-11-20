#ifndef STRING_Z_ALGORITHM_HPP
#define STRING_Z_ALGORITHM_HPP

#include "../components/sequence_match.hpp"
#include <cassert>
#include <limits>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

template <class T>
std::vector<int> z_algorithm(std::span<const T> sequence) {
	assert(sequence.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> result(sequence.size());
	sequence_match_detail::Matcher<T> match(sequence);
	sequence_match_detail::z_table(int(sequence.size()), result.data(), [&](int a, int b, int limit) { return match.forward(a, b, limit); });
	return result;
}

template <class T, std::size_t Extent> requires (!std::is_const_v<T> || Extent != std::dynamic_extent)
std::vector<int> z_algorithm(std::span<T, Extent> sequence) {
	return z_algorithm(std::span<const std::remove_const_t<T>>(sequence.data(), sequence.size()));
}

inline std::vector<int> z_algorithm(std::string_view sequence) {
	return z_algorithm(std::span<const char>(sequence.data(), sequence.size()));
}

template <class T, class Allocator>
std::vector<int> z_algorithm(const std::vector<T, Allocator> &sequence) {
	return z_algorithm(std::span<const T>(sequence.data(), sequence.size()));
}

#endif
