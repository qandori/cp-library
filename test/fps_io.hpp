#ifndef TEST_FPS_IO_HPP
#define TEST_FPS_IO_HPP

#include "components/fast_io.hpp"
#include "fps/fps.hpp"
#include "components/fps_io.hpp"

inline fps read_fps(FastInput &input, int n) {
	fps result;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::read_coefficients(input, result, n);
		return result;
	}
#endif
	result.resize(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_fps(FastOutput &output, const fps &f) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::write_coefficients(output, f.data(), f.size());
		return;
	}
#endif
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}

#endif
