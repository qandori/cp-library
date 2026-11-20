#ifndef CONVOLUTION_NTT_HPP
#define CONVOLUTION_NTT_HPP

#include "convolution_mod.hpp"
#include <concepts>

namespace convolution_mod_detail {
template<Word Mod> constexpr Word ntt_root() {
	if constexpr (Mod > 2 && Mod < (Word(1) << 31) && (Mod & 1)) {
		for (Word p = 3; p <= Mod / p; p += 2) if (Mod % p == 0) return 0;
		for (Word g = 2; g < Mod; ++g) {
			if (ntt_mod_detail::power<Mod>(g, (Mod - 1) / 2) == Mod - 1) return g;
		}
	}
	return 0;
}
}

template<std::uint32_t Mod, std::integral T> requires (sizeof(T) <= 8 && !std::is_same_v<T, bool>)
std::vector<std::uint32_t> convolution_ntt(const std::vector<T> &a, const std::vector<T> &b) {
	constexpr auto root = convolution_mod_detail::ntt_root<Mod>();
	static_assert(root != 0, "convolution_ntt requires an odd prime below 2^31");
	if constexpr (root != 0) {
		if (a.empty() || b.empty()) return {};
		assert(a.size() + b.size() - 1 <= (std::size_t(1) << std::countr_zero(Mod - 1)));
		return convolution_mod_detail::multiply<Mod, root>(a, b);
	} else return {};
}

template<class T> requires requires(T x) {
	std::integral_constant<std::uint32_t, T::mod()>{};
	{ T::raw(0) } -> std::same_as<T>;
	x.val();
}
std::vector<T> convolution_ntt(const std::vector<T> &a, const std::vector<T> &b) {
	auto words = [](const auto &v) {
		std::vector<std::uint32_t> result;
		result.reserve(v.size());
		for (auto x : v) result.push_back(x.val());
		return result;
	};
	if (a.empty() || b.empty()) return {};
	auto x = words(a), y = &a == &b ? std::vector<std::uint32_t>{} : words(b);
	auto values = convolution_ntt<T::mod()>(x, &a == &b ? x : y);
	std::vector<T> result;
	result.reserve(values.size());
	for (auto v : values) result.push_back(T::raw(v));
	return result;
}

#endif
