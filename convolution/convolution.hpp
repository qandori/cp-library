#ifndef CONVOLUTION_HPP
#define CONVOLUTION_HPP

#include "../fps/fps.hpp"
#include "../components/convolution_dispatch.hpp"

#ifndef CP_ACL_CONVOLUTION
#define CP_ACL_CONVOLUTION
namespace cp_acl {
template<unsigned Mod = 0, class T>
std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if constexpr (std::is_integral_v<T>) {
		auto c = ::convolution_mod(a, b, Mod ? Mod : 998244353);
		return {c.begin(), c.end()};
	} else {
		static_assert(!Mod || Mod == T::mod());
		return ::convolution(a, b);
	}
}

inline auto convolution_ll(const std::vector<long long> &a, const std::vector<long long> &b) {
	return ::convolution_integer(a, b);
}
}
#endif

#endif
