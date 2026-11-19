#ifndef LCP_ARRAY_HPP
#define LCP_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

namespace lcp_array_detail {
template <class Sequence>
std::vector<int> build(const Sequence &s, const std::vector<int> &sa) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()) && sa.size() == s.size());
	int n = int(s.size());
	if (!n) return {};
	std::vector<int> rank(n, -1), result(n - 1);
	for (int i = 0; i < n; ++i) {
		assert(0 <= sa[i] && sa[i] < n && rank[sa[i]] == -1);
		rank[sa[i]] = i;
	}
	for (int i = 0, h = 0; i < n; ++i) {
		if (h) --h;
		if (rank[i] == 0) { h = 0; continue; }
		int j = sa[rank[i] - 1];
		int limit = n - std::max(i, j);
		while (h < limit && s[i + h] == s[j + h]) ++h;
		result[rank[i] - 1] = h;
	}
	return result;
}
}

inline std::vector<int> lcp_array(std::string_view s, const std::vector<int> &sa) { return lcp_array_detail::build(s, sa); }
template <class T, class Allocator>
std::vector<int> lcp_array(const std::vector<T, Allocator> &s, const std::vector<int> &sa) { return lcp_array_detail::build(s, sa); }
template <class T, std::size_t Extent>
std::vector<int> lcp_array(std::span<T, Extent> s, const std::vector<int> &sa) { return lcp_array_detail::build(s, sa); }

namespace cp_acl {
using ::lcp_array;
}

#endif
