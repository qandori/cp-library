#ifndef CP_FPS_POLYNOMIAL_GCD_HPP
#define CP_FPS_POLYNOMIAL_GCD_HPP

#include "fps.hpp"
#include "../components/polynomial_half_gcd.hpp"
#include <optional>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
struct PolynomialExtendedGCD {
	FormalPowerSeries<T> gcd, x, y;
};

template<class T>
FormalPowerSeries<T> polynomial_gcd(FormalPowerSeries<T> a, FormalPowerSeries<T> b) {
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>)
		return O::export_work(polynomial_gcd(O::import_work(a), O::import_work(b)));
	else {
		O::trim(a), O::trim(b);
		if (a.size() < b.size()) a.swap(b);
		while (!b.empty()) {
			if (a.size() > b.size() && a.size() > 128) {
				auto matrix = H::half(a, b);
				H::apply(matrix, a, b);
				if (b.empty()) break;
			}
			auto r = a.divmod(b).second;
			a = std::move(b), b = std::move(r);
		}
		if (!a.empty()) a /= a.back();
		return a;
	}
}

template<class T>
PolynomialExtendedGCD<T> polynomial_extended_gcd(FormalPowerSeries<T> a, FormalPowerSeries<T> b) {
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		auto result = polynomial_extended_gcd(O::import_work(a), O::import_work(b));
		return {O::export_work(result.gcd), O::export_work(result.x), O::export_work(result.y)};
	} else {
		O::trim(a), O::trim(b);
		if (a.empty() && b.empty()) return {};
		typename H::Matrix matrix;
		while (!b.empty()) {
			if (a.size() > b.size() && a.size() > 128) {
				auto part = H::half(a, b);
				H::apply(part, a, b), matrix = H::multiply(part, matrix);
				if (b.empty()) break;
			}
			auto [q, r] = a.divmod(b);
			H::step(matrix, q), a = std::move(b), b = std::move(r);
		}
		T inverse = a.back().inv();
		a *= inverse, matrix.a[0] *= inverse, matrix.a[1] *= inverse;
		return {std::move(a), std::move(matrix.a[0]), std::move(matrix.a[1])};
	}
}

template<class T>
std::optional<FormalPowerSeries<T>> polynomial_inv_mod(FormalPowerSeries<T> a, FormalPowerSeries<T> modulus) {
	fps_detail::Operations<T>::trim(modulus);
	assert(!modulus.empty());
	if (modulus.size() == 1) return FormalPowerSeries<T>{};
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		auto result = polynomial_inv_mod(O::import_work(a), O::import_work(modulus));
		if (!result) return std::nullopt;
		return O::export_work(*result);
	} else {
		if (a.size() >= modulus.size()) a = a.divmod(modulus).second;
		O::trim(a);
		FormalPowerSeries<T> b = modulus;
		typename H::Matrix matrix;
		while (!b.empty()) {
			if (a.size() > b.size() && a.size() > 128) {
				auto part = H::half(a, b);
				H::apply(part, a, b), matrix = H::multiply_column(part, matrix);
				if (b.empty()) break;
			}
			auto [q, r] = a.divmod(b);
			H::step_column(matrix, q), a = std::move(b), b = std::move(r);
		}
		if (a.size() != 1) return std::nullopt;
		matrix.a[0] /= a.back();
		return matrix.a[0].divmod(modulus).second;
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
