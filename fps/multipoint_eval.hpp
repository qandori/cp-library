#ifndef CP_FPS_MULTIPOINT_EVAL_HPP
#define CP_FPS_MULTIPOINT_EVAL_HPP

#include "fps.hpp"
#include "../components/fps_product_tree.hpp"
#include "../components/fps_direct_eval.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
std::vector<T> multipoint_eval(const FormalPowerSeries<T> &f, const std::vector<T> &xs) {
	using F = FormalPowerSeries<T>;
	using W = FormalPowerSeries<fps_detail::Mint>;
	using base = std::vector<T>;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) return O::export_work(W(multipoint_eval<fps_detail::Mint>(O::import_work(f), O::import_work(xs))));
	if (xs.empty()) return {};
	if (f.size() <= 32 || xs.size() <= 32) {
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (((xs.size() >= 8 && f.size() >= 8 && (xs.size() >= 32 || f.size() >= 32)) ||
				 (xs.size() >= 4096 && f.size() >= 4)) && fps_detail::NTT::vectorized()) {
				base r(xs.size());
				fps_detail::evaluate_direct(O::words(r), O::words(f), f.size(), O::words(xs), xs.size());
				return r;
			}
		}
#endif
		base r;
		r.reserve(xs.size());
		for (T x : xs) r.push_back(f.eval(x));
		return r;
	}
	constexpr int block = 8;
	int length = block;
	while (length < (int)std::max(f.size(), xs.size())) length <<= 1;
	assert(length <= (1 << 23));
	int size = 1;
	std::vector<base> frequency;
	auto tree = fps_detail::ProductTree<T>::build(xs, size, block, true, &frequency);
	F root = f;
	root.resize(length);
	std::reverse(root.begin(), root.end());
	root = root.div_series(tree[1], length);
	if (size * block < length) root.erase(root.begin(), root.end() - size * block);
	return fps_detail::ProductTree<T>::evaluate(xs, tree, frequency, size, block, std::move(root), true);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
