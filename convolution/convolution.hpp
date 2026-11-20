#ifndef CONVOLUTION_HPP
#define CONVOLUTION_HPP

#include "../fps/fps.hpp"
#include "../components/convolution_dispatch.hpp"

#ifndef CP_MODULAR_CONVOLUTION_API
#define CP_MODULAR_CONVOLUTION_API
template<unsigned Mod, class T>
std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if constexpr (std::is_integral_v<T>) {
		auto c = ::convolution_mod(a, b, Mod);
		return {c.begin(), c.end()};
	} else {
		static_assert(Mod == T::mod());
		return ::convolution(a, b);
	}
}

inline auto convolution_ll(const std::vector<long long> &a, const std::vector<long long> &b) {
	return ::convolution_integer(a, b);
}
#endif

#endif
