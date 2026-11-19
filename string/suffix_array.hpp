#ifndef STRING_SUFFIX_ARRAY_HPP
#define STRING_SUFFIX_ARRAY_HPP

#include "../components/suffix_array.hpp"
#include <span>
#include <type_traits>

inline std::vector<int> suffix_array(std::string_view s) { return suffix_array_detail::bytes(s); }

namespace suffix_array_detail {
template <class Sequence>
std::vector<int> ordered(const Sequence &s) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> order(s.size()), ranks(s.size());
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int a, int b) { return s[a] < s[b]; });
	int rank = 0;
	for (std::size_t i = 0; i < order.size(); ++i) {
		if (i && s[order[i - 1]] < s[order[i]]) ++rank;
		ranks[order[i]] = rank;
	}
	if (!order.empty() && rank == int(order.size()) - 1) return order;
	return suffix_array_detail::induced_sort(ranks, rank);
}
}

template <class T, class Allocator>
std::vector<int> suffix_array(const std::vector<T, Allocator> &s) { return suffix_array_detail::ordered(s); }

template <class T, std::size_t Extent>
std::vector<int> suffix_array(std::span<T, Extent> s) { return suffix_array_detail::ordered(s); }

inline std::vector<int> suffix_array(const std::vector<int> &s, int upper) {
	assert(upper >= 0 && s.size() <= std::size_t(std::numeric_limits<int>::max()));
	for ([[maybe_unused]] int x : s) assert(0 <= x && x <= upper);
	if (upper > std::numeric_limits<int>::max() - 2) return suffix_array(s);
	return suffix_array_detail::induced_sort(s, upper);
}

template <class T, std::size_t Extent> requires std::is_same_v<std::remove_const_t<T>, int>
std::vector<int> suffix_array(std::span<T, Extent> s, int upper) {
	assert(upper >= 0 && s.size() <= std::size_t(std::numeric_limits<int>::max()));
	for ([[maybe_unused]] int x : s) assert(0 <= x && x <= upper);
	if (upper > std::numeric_limits<int>::max() - 2) return suffix_array(s);
	return suffix_array_detail::induced_sort(s, upper);
}

template <class Allocator>
std::vector<int> suffix_array(const std::vector<int, Allocator> &s, int upper) {
	return suffix_array(std::span<const int>(s.data(), s.size()), upper);
}

namespace cp_acl {
using ::suffix_array;
}

#endif
