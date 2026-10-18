#ifndef CP_COMPONENTS_FPS_BLOCKED_HPP
#define CP_COMPONENTS_FPS_BLOCKED_HPP

#include "fps_workspace.hpp"
#include "../math/mint.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>
#include <cstring>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP, kNI = 998244351;

inline void forward(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a],%[h3]), %%ymm0\n\t"
		"vmovdqa 32(%[a],%[h3]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h]), %%ymm2\n\t"
		"vmovdqa 32(%[a],%[h]), %%ymm3\n\t"
		"vpmuludq (%[a],%[h3]), %%ymm11, %%ymm4\n\t"
		"vpmuludq 32(%[a],%[h3]), %%ymm11, %%ymm5\n\t"
		"vpmuludq 4(%[a],%[h3]), %%ymm11, %%ymm6\n\t"
		"vpmuludq 36(%[a],%[h3]), %%ymm11, %%ymm7\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm8\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm9\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpblendd $170, %%ymm6, %%ymm4, %%ymm6\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa (%[a]), %%ymm5\n\t"
		"vpmulld (%[w]), %%ymm0, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vmovdqa 32(%[a]), %%ymm4\n\t"
		"vpmulld (%[w]), %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpminud %%ymm10, %%ymm2, %%ymm10\n\t"
		"vpmuludq (%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm0\n\t"
		"vpminud %%ymm0, %%ymm3, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpmuludq 32(%[a],%[h],2), %%ymm11, %%ymm1\n\t"
		"vpmuludq 4(%[a],%[h],2), %%ymm11, %%ymm3\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpmulld (%[w]), %%ymm8, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpmuludq 36(%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm1\n\t"
		"vpaddd %%ymm6, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm3, %%ymm8, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm7, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm8, %%ymm7\n\t"
		"vpsrlq $32, %%ymm10, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm2, %%ymm9, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm1\n\t"
		"vpmulld 64(%[w]), %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm10\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm10, %%ymm1, %%ymm10\n\t"
		"vpsubd %%ymm9, %%ymm6, %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm1\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm3, %%ymm0, %%ymm4\n\t"
		"vpsubd %%ymm3, %%ymm0, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm1, %%ymm0\n\t"
		"vpsubd %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm5\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm5, %%ymm3, %%ymm5\n\t"
		"vpsubd %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpaddd %%ymm8, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm9, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm9, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm10, %%ymm7, %%ymm7\n\t"
		"vmovdqa %%ymm1, (%[a])\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpaddd %%ymm6, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vmovdqa %%ymm5, (%[a],%[h],2)\n\t"
		"vmovdqa %%ymm9, (%[a],%[h3])\n\t"
		"vmovdqa %%ymm7, 32(%[a])\n\t"
		"vmovdqa %%ymm10, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm3, 32(%[a],%[h],2)\n\t"
		"vmovdqa %%ymm6, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a]), %%ymm0\n\t"
		"vmovdqa 32(%[a]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm2\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd (%[a],%[h]), %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd (%[a],%[h]), %%ymm4, %%ymm4\n\t"
		"vpaddd 32(%[a],%[h]), %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpsubd 32(%[a],%[h]), %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm4, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm10\n\t"
		"vpsubd (%[a],%[h3]), %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm4, %%ymm4\n\t"
		"vpsubd 32(%[a],%[h3]), %%ymm10, %%ymm10\n\t"
		"vpaddd (%[a],%[h3]), %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsrlq $32, %%ymm5, %%ymm0\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpaddd 32(%[a],%[h3]), %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm5, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm0, %%ymm9, %%ymm0\n\t"
		"vpsubd %%ymm8, %%ymm4, %%ymm8\n\t"
		"vpsrlq $32, %%ymm7, %%ymm4\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsrlq $32, %%ymm10, %%ymm5\n\t"
		"vpmuludq %%ymm13, %%ymm10, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpmulld 64(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpminud %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm1\n\t"
		"vpminud %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm3\n\t"
		"vpaddd %%ymm9, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm7, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm7\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm1\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm4, %%ymm8, %%ymm4\n\t"
		"vpaddd %%ymm5, %%ymm0, %%ymm5\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm7, %%ymm0\n\t"
		"vpmulld (%[w]), %%ymm7, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm1, %%ymm9, %%ymm1\n\t"
		"vpsrlq $32, %%ymm3, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm3, %%ymm0\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm1, (%[a],%[h],2)\n\t"
		"vpsrlq $32, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpmulld (%[w]), %%ymm3, %%ymm3\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm7, %%ymm8\n\t"
		"vpmuludq %%ymm11, %%ymm10, %%ymm7\n\t"
		"vpmulld (%[w]), %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm0\n\t"
		"vmovdqa %%ymm8, (%[a],%[h3])\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm8\n\t"
		"vpminud %%ymm0, %%ymm6, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm6\n\t"
		"vpminud %%ymm8, %%ymm4, %%ymm8\n\t"
		"vmovdqa %%ymm0, (%[a])\n\t"
		"vpblendd $170, %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm7\n\t"
		"vpminud %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm3, %%ymm9\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa %%ymm6, 32(%[a])\n\t"
		"vmovdqa %%ymm7, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm9, 32(%[a],%[h],2)\n\t"
		"vpsubd %%ymm1, %%ymm10, %%ymm1\n\t"
		"vmovdqa %%ymm1, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void forward_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss 4(%[y]), %%ymm12\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a],%[h]), %%ymm0\n\t"
		"vmovdqa 32(%[a],%[h]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h3]), %%ymm2\n\t"
		"vmovdqa 32(%[a],%[h3]), %%ymm3\n\t"
		"vmovdqa (%[a]), %%ymm4\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm5\n\t"
		"vmovdqa 32(%[a]), %%ymm6\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm9\n\t"
		"vpminud %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm0\n\t"
		"vpminud %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm10\n\t"
		"vpminud %%ymm0, %%ymm2, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm2\n\t"
		"vpminud %%ymm10, %%ymm3, %%ymm10\n\t"
		"vpsubd %%ymm0, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm3\n\t"
		"vpaddd %%ymm0, %%ymm8, %%ymm0\n\t"
		"vpsubd %%ymm10, %%ymm2, %%ymm2\n\t"
		"vpsrlq $32, %%ymm1, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm1, %%ymm11\n\t"
		"vpminud %%ymm3, %%ymm4, %%ymm3\n\t"
		"vpsrlq $32, %%ymm2, %%ymm4\n\t"
		"vpmuludq %%ymm13, %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm12, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm10, %%ymm9, %%ymm10\n\t"
		"vpmuludq %%ymm13, %%ymm2, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm4\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm12, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm8, %%ymm11, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm11\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpminud %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm5\n\t"
		"vpminud %%ymm5, %%ymm6, %%ymm5\n\t"
		"vpblendd $170, %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm9\n\t"
		"vpaddd %%ymm11, %%ymm3, %%ymm6\n\t"
		"vpminud %%ymm9, %%ymm7, %%ymm9\n\t"
		"vpsubd %%ymm11, %%ymm3, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm3\n\t"
		"vpaddd %%ymm9, %%ymm5, %%ymm7\n\t"
		"vpsubd %%ymm9, %%ymm5, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm5\n\t"
		"vpminud %%ymm3, %%ymm6, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm6\n\t"
		"vpminud %%ymm5, %%ymm11, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm11\n\t"
		"vpsubd %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm1\n\t"
		"vpminud %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm7\n\t"
		"vpminud %%ymm11, %%ymm0, %%ymm11\n\t"
		"vpminud %%ymm1, %%ymm9, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpminud %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpsubd %%ymm4, %%ymm2, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm10\n\t"
		"vpaddd %%ymm11, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm11, %%ymm9, %%ymm11\n\t"
		"vpaddd %%ymm8, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"
		"vmovdqa %%ymm3, (%[a])\n\t"
		"vpsubd %%ymm7, %%ymm2, %%ymm7\n\t"
		"vpaddd %%ymm4, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm4, %%ymm10, %%ymm4\n\t"
		"vmovdqa %%ymm11, (%[a],%[h])\n\t"
		"vmovdqa %%ymm5, (%[a],%[h],2)\n\t"
		"vmovdqa %%ymm8, (%[a],%[h3])\n\t"
		"vmovdqa %%ymm6, 32(%[a])\n\t"
		"vmovdqa %%ymm7, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm1, 32(%[a],%[h],2)\n\t"
		"vmovdqa %%ymm4, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss 4(%[y]), %%ymm12\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm0\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm1\n\t"
		"vmovdqa (%[a]), %%ymm2\n\t"
		"vmovdqa 32(%[a]), %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd (%[a],%[h3]), %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd (%[a],%[h3]), %%ymm4, %%ymm4\n\t"
		"vpaddd 32(%[a],%[h3]), %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpsubd 32(%[a],%[h3]), %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm4, %%ymm7\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm8\n\t"
		"vpmulld %%ymm12, %%ymm4, %%ymm4\n\t"
		"vpsrlq $32, %%ymm5, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm5, %%ymm10\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm7\n\t"
		"vpaddd (%[a],%[h]), %%ymm2, %%ymm11\n\t"
		"vpmuludq %%ymm13, %%ymm9, %%ymm9\n\t"
		"vpmulld %%ymm12, %%ymm5, %%ymm5\n\t"
		"vpsubd (%[a],%[h]), %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpaddd 32(%[a],%[h]), %%ymm3, %%ymm0\n\t"
		"vpsubd 32(%[a],%[h]), %%ymm3, %%ymm3\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vpblendd $170, %%ymm7, %%ymm8, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm8\n\t"
		"vpblendd $170, %%ymm9, %%ymm10, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpminud %%ymm8, %%ymm11, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpminud %%ymm11, %%ymm2, %%ymm11\n\t"
		"vpaddd %%ymm6, %%ymm8, %%ymm2\n\t"
		"vpminud %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm6, %%ymm8, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm4, %%ymm7\n\t"
		"vpminud %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpminud %%ymm0, %%ymm3, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpsubd %%ymm9, %%ymm5, %%ymm9\n\t"
		"vpaddd %%ymm7, %%ymm11, %%ymm5\n\t"
		"vpsubd %%ymm7, %%ymm11, %%ymm7\n\t"
		"vpaddd %%ymm8, %%ymm10, %%ymm11\n\t"
		"vpaddd %%ymm9, %%ymm0, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpsubd %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm10\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm2\n\t"
		"vpminud %%ymm10, %%ymm6, %%ymm10\n\t"
		"vpminud %%ymm4, %%ymm7, %%ymm4\n\t"
		"vmovdqa %%ymm3, (%[a])\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm7\n\t"
		"vpminud %%ymm5, %%ymm11, %%ymm5\n\t"
		"vpminud %%ymm2, %%ymm1, %%ymm2\n\t"
		"vmovdqa %%ymm0, (%[a],%[h])\n\t"
		"vpminud %%ymm3, %%ymm8, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm9, %%ymm7\n\t"
		"vmovdqa %%ymm10, (%[a],%[h],2)\n\t"
		"vmovdqa %%ymm4, (%[a],%[h3])\n\t"
		"vmovdqa %%ymm5, 32(%[a])\n\t"
		"vmovdqa %%ymm2, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm3, 32(%[a],%[h],2)\n\t"
		"vmovdqa %%ymm7, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void scale_radix2(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss 4(%[y]), %%ymm12\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a]), %%ymm0\n\t"
		"vmovdqa 32(%[a]), %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm2\n\t"
		"vpaddd (%[a],%[h]), %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm3\n\t"
		"vpsubd (%[a],%[h]), %%ymm2, %%ymm2\n\t"
		"vpaddd 32(%[a],%[h]), %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm0, %%ymm4\n\t"
		"vpsubd 32(%[a],%[h]), %%ymm3, %%ymm3\n\t"
		"vpmuludq %%ymm13, %%ymm0, %%ymm5\n\t"
		"vpsrlq $32, %%ymm2, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm2, %%ymm7\n\t"
		"vpsrlq $32, %%ymm1, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm1, %%ymm9\n\t"
		"vpsrlq $32, %%ymm3, %%ymm10\n\t"
		"vpmuludq %%ymm13, %%ymm3, %%ymm11\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm4\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm8, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm10, %%ymm10\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm12, %%ymm0, %%ymm0\n\t"
		"vpmulld %%ymm12, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm4, %%ymm5, %%ymm4\n\t"
		"vpblendd $170, %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpmulld %%ymm12, %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm12, %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpblendd $170, %%ymm10, %%ymm11, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm4, %%ymm0, %%ymm4\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpsubd %%ymm10, %%ymm3, %%ymm10\n\t"
		"vpsubd %%ymm15, %%ymm4, %%ymm3\n\t"
		"vpsubd %%ymm15, %%ymm6, %%ymm1\n\t"
		"vpsubd %%ymm15, %%ymm8, %%ymm2\n\t"
		"vpsubd %%ymm15, %%ymm10, %%ymm0\n\t"
		"vpminud %%ymm3, %%ymm4, %%ymm3\n\t"
		"vpminud %%ymm1, %%ymm6, %%ymm1\n\t"
		"vpminud %%ymm2, %%ymm8, %%ymm2\n\t"
		"vpminud %%ymm0, %%ymm10, %%ymm0\n\t"
		"vmovdqa %%ymm3, (%[a])\n\t"
		"vmovdqa %%ymm1, (%[a],%[h])\n\t"
		"vmovdqa %%ymm2, 32(%[a])\n\t"
		"vmovdqa %%ymm0, 32(%[a],%[h])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}
namespace blocked_ntt {

inline constexpr std::uint32_t kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec;
using kernels::kP;

constexpr std::uint32_t multiply_mod(std::uint32_t x, std::uint32_t y) {
	return std::uint32_t(std::uint64_t(x) * y % kP);
}

constexpr std::uint32_t power(std::uint32_t x, std::uint32_t e) {
	std::uint32_t r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr std::uint32_t quotient(std::uint32_t w) { return std::uint32_t((std::uint64_t(w) << 32) / kP); }

inline constexpr std::uint32_t kR = std::uint32_t((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<std::uint32_t, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(std::uint32_t x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, std::uint32_t bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(std::uint32_t value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(std::uint32_t value, std::uint32_t value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, f.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline std::size_t slot(std::size_t k) { return (k >> 3 << 4) | (k & 7); }

inline std::size_t table_words(int lg) { return (std::size_t(1) << lg) / 8 + 16; }

inline void build_table(std::uint32_t* t, std::size_t count, const std::array<std::uint32_t, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const std::size_t h = std::size_t(1) << level;
		if (h >= count) break;
		const std::uint32_t root = roots[level];
		if (h < 8) {
			for (std::size_t j = 0; j < h; ++j) {
				const std::uint32_t v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (std::size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

inline void inverse_radix4(Vec* f, std::size_t h, const std::uint32_t* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(multiply(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = reduce(add(p0, p1), 2 * kP), cd = reduce(add(p2, p3), 2 * kP);
		const Vec amb = reduce(diff(p0, p1), 2 * kP), cmd = multiply(diff(p2, p3), z);
		f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

}
}
namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"mov %[k], %[tx]\n\t"
		"and $-8, %[tx]\n\t"
		"add %[k], %[tx]\n\t"
		"lea (%[table],%[tx],4), %[tx]\n\t"
		"lea (%[k],%[k]), %[ty]\n\t"
		"and $-8, %[ty]\n\t"
		"lea (%[ty],%[k],2), %[ty]\n\t"
		"lea (%[table],%[ty],4), %[ty]\n\t"
		"vbroadcastss 32(%[tx]), %%ymm0\n\t"
		"vbroadcastss (%[tx]), %%ymm1\n\t"
		"vmovdqa 128(%[a]), %%ymm2\n\t"
		"vmovdqa (%[a]), %%ymm3\n\t"
		"vpmuludq 384(%[a]), %%ymm0, %%ymm4\n\t"
		"vpmuludq 388(%[a]), %%ymm0, %%ymm5\n\t"
		"vpmulld 384(%[a]), %%ymm1, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpmuludq 256(%[a]), %%ymm0, %%ymm8\n\t"
		"vpminud %%ymm7, %%ymm2, %%ymm7\n\t"
		"vpmuludq 260(%[a]), %%ymm0, %%ymm2\n\t"
		"vpmulld 256(%[a]), %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm10\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpminud %%ymm10, %%ymm3, %%ymm10\n\t"
		"vpblendd $170, %%ymm5, %%ymm4, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm4\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vbroadcastss 32(%[ty]), %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpblendd $170, %%ymm2, %%ymm8, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vbroadcastss (%[ty]), %%ymm8\n\t"
		"vpsubd %%ymm5, %%ymm6, %%ymm5\n\t"
		"vpaddd %%ymm5, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm5, %%ymm4, %%ymm5\n\t"
		"vpsubd %%ymm2, %%ymm9, %%ymm2\n\t"
		"vbroadcastss 36(%[ty]), %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm4\n\t"
		"vpsrlq $32, %%ymm5, %%ymm6\n\t"
		"vpaddd %%ymm2, %%ymm10, %%ymm11\n\t"
		"vpsubd %%ymm2, %%ymm10, %%ymm2\n\t"
		"vbroadcastss 4(%[ty]), %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm13\n\t"
		"vpminud %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpminud %%ymm13, %%ymm2, %%ymm13\n\t"
		"vpmuludq %%ymm3, %%ymm7, %%ymm2\n\t"
		"vpmuludq %%ymm3, %%ymm4, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm11\n\t"
		"vpmulld %%ymm8, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm4, %%ymm2, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm2\n\t"
		"vpmuludq %%ymm9, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm4\n\t"
		"vmovdqa 32(%[a]), %%ymm7\n\t"
		"vpaddd %%ymm4, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm4, %%ymm11, %%ymm4\n\t"
		"vpmuludq %%ymm9, %%ymm5, %%ymm11\n\t"
		"vpmulld %%ymm10, %%ymm5, %%ymm5\n\t"
		"vmovdqa %%ymm12, (%[a])\n\t"
		"vmovdqa 160(%[a]), %%ymm12\n\t"
		"vmovdqa %%ymm4, 128(%[a])\n\t"
		"vpmuludq 288(%[a]), %%ymm0, %%ymm4\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpblendd $170, %%ymm6, %%ymm11, %%ymm6\n\t"
		"vpmuludq 292(%[a]), %%ymm0, %%ymm11\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpblendd $170, %%ymm11, %%ymm4, %%ymm11\n\t"
		"vpmuludq 416(%[a]), %%ymm0, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm6, %%ymm5, %%ymm6\n\t"
		"vpmuludq 420(%[a]), %%ymm0, %%ymm5\n\t"
		"vpaddd %%ymm6, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpmulld 416(%[a]), %%ymm1, %%ymm2\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vmovdqa %%ymm13, 256(%[a])\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm13\n\t"
		"vpblendd $170, %%ymm5, %%ymm4, %%ymm5\n\t"
		"vpminud %%ymm13, %%ymm12, %%ymm13\n\t"
		"vmovdqa %%ymm6, 384(%[a])\n\t"
		"vpmulld 288(%[a]), %%ymm1, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm4\n\t"
		"vpminud %%ymm12, %%ymm7, %%ymm12\n\t"
		"vpsubd %%ymm11, %%ymm6, %%ymm11\n\t"
		"vpsubd %%ymm5, %%ymm2, %%ymm5\n\t"
		"vpaddd %%ymm11, %%ymm12, %%ymm2\n\t"
		"vpsubd %%ymm11, %%ymm12, %%ymm11\n\t"
		"vpaddd %%ymm5, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm5, %%ymm4, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm12\n\t"
		"vpsrlq $32, %%ymm13, %%ymm6\n\t"
		"vpmuludq %%ymm3, %%ymm13, %%ymm7\n\t"
		"vpmulld %%ymm8, %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm4, %%ymm2, %%ymm4\n\t"
		"vpsrlq $32, %%ymm5, %%ymm2\n\t"
		"vpmuludq %%ymm3, %%ymm6, %%ymm6\n\t"
		"vpminud %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm4, %%ymm11\n\t"
		"vpmuludq %%ymm9, %%ymm2, %%ymm2\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpblendd $170, %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpmuludq %%ymm9, %%ymm5, %%ymm7\n\t"
		"vpmulld %%ymm10, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpblendd $170, %%ymm2, %%ymm7, %%ymm2\n\t"
		"vpsubd %%ymm6, %%ymm13, %%ymm6\n\t"
		"vmovdqa 64(%[a]), %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm6, %%ymm11, %%ymm6\n\t"
		"vmovdqa 192(%[a]), %%ymm11\n\t"
		"vmovdqa %%ymm4, 32(%[a])\n\t"
		"vpmuludq 320(%[a]), %%ymm0, %%ymm4\n\t"
		"vmovdqa %%ymm6, 160(%[a])\n\t"
		"vpmuludq 324(%[a]), %%ymm0, %%ymm6\n\t"
		"vpsubd %%ymm2, %%ymm5, %%ymm2\n\t"
		"vpmuludq 448(%[a]), %%ymm0, %%ymm5\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpaddd %%ymm2, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm2, %%ymm7, %%ymm2\n\t"
		"vpmuludq 452(%[a]), %%ymm0, %%ymm7\n\t"
		"vpblendd $170, %%ymm6, %%ymm4, %%ymm6\n\t"
		"vmovdqa %%ymm12, 288(%[a])\n\t"
		"vpmulld 448(%[a]), %%ymm1, %%ymm12\n\t"
		"vpmulld 320(%[a]), %%ymm1, %%ymm4\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vmovdqa %%ymm2, 416(%[a])\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm2\n\t"
		"vpminud %%ymm5, %%ymm13, %%ymm5\n\t"
		"vpminud %%ymm2, %%ymm11, %%ymm2\n\t"
		"vpsubd %%ymm6, %%ymm4, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm4\n\t"
		"vpaddd %%ymm6, %%ymm5, %%ymm11\n\t"
		"vpsubd %%ymm6, %%ymm5, %%ymm6\n\t"
		"vpsubd %%ymm7, %%ymm12, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm5\n\t"
		"vpaddd %%ymm7, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm7, %%ymm4, %%ymm7\n\t"
		"vpminud %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpminud %%ymm5, %%ymm6, %%ymm5\n\t"
		"vpsrlq $32, %%ymm2, %%ymm6\n\t"
		"vpmuludq %%ymm3, %%ymm2, %%ymm11\n\t"
		"vpsrlq $32, %%ymm7, %%ymm4\n\t"
		"vpmuludq %%ymm9, %%ymm7, %%ymm13\n\t"
		"vpmuludq %%ymm3, %%ymm6, %%ymm6\n\t"
		"vpmuludq %%ymm9, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm8, %%ymm2, %%ymm2\n\t"
		"vpmulld %%ymm10, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpblendd $170, %%ymm6, %%ymm11, %%ymm6\n\t"
		"vpblendd $170, %%ymm4, %%ymm13, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm4\n\t"
		"vmovdqa 224(%[a]), %%ymm7\n\t"
		"vmovdqa 96(%[a]), %%ymm2\n\t"
		"vpaddd %%ymm6, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm6, %%ymm13, %%ymm6\n\t"
		"vpaddd %%ymm4, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm4, %%ymm11, %%ymm4\n\t"
		"vpmuludq 480(%[a]), %%ymm0, %%ymm11\n\t"
		"vpmuludq 352(%[a]), %%ymm0, %%ymm13\n\t"
		"vmovdqa %%ymm12, 64(%[a])\n\t"
		"vpmuludq 484(%[a]), %%ymm0, %%ymm12\n\t"
		"vpmuludq 356(%[a]), %%ymm0, %%ymm0\n\t"
		"vmovdqa %%ymm6, 192(%[a])\n\t"
		"vpmulld 480(%[a]), %%ymm1, %%ymm6\n\t"
		"vpmulld 352(%[a]), %%ymm1, %%ymm1\n\t"
		"vmovdqa %%ymm5, 320(%[a])\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vmovdqa %%ymm4, 448(%[a])\n\t"
		"vpblendd $170, %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpblendd $170, %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm13\n\t"
		"vpminud %%ymm13, %%ymm7, %%ymm13\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpsubd %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm6\n\t"
		"vpminud %%ymm7, %%ymm2, %%ymm7\n\t"
		"vpsubd %%ymm0, %%ymm1, %%ymm0\n\t"
		"vpaddd %%ymm12, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpaddd %%ymm0, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm0, %%ymm7, %%ymm0\n\t"
		"vpsrlq $32, %%ymm13, %%ymm7\n\t"
		"vpmuludq %%ymm3, %%ymm13, %%ymm1\n\t"
		"vpsrlq $32, %%ymm12, %%ymm2\n\t"
		"vpmuludq %%ymm9, %%ymm12, %%ymm11\n\t"
		"vpmuludq %%ymm3, %%ymm7, %%ymm3\n\t"
		"vpmuludq %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm7\n\t"
		"vpmulld %%ymm8, %%ymm13, %%ymm8\n\t"
		"vpmulld %%ymm10, %%ymm12, %%ymm10\n\t"
		"vpminud %%ymm2, %%ymm6, %%ymm2\n\t"
		"vpminud %%ymm7, %%ymm0, %%ymm7\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm6\n\t"
		"vpblendd $170, %%ymm3, %%ymm1, %%ymm3\n\t"
		"vpblendd $170, %%ymm9, %%ymm11, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm3, %%ymm8, %%ymm3\n\t"
		"vpsubd %%ymm9, %%ymm10, %%ymm9\n\t"
		"vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm3, %%ymm0, %%ymm3\n\t"
		"vpaddd %%ymm9, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm9, %%ymm6, %%ymm9\n\t"
		"vmovdqa %%ymm2, 96(%[a])\n\t"
		"vmovdqa %%ymm3, 224(%[a])\n\t"
		"vmovdqa %%ymm7, 352(%[a])\n\t"
		"vmovdqa %%ymm9, 480(%[a])\n\t"
		"add $512, %[a]\n\t"
		"inc %[k]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"mov %[k], %[tx]\n\t"
		"and $-8, %[tx]\n\t"
		"add %[k], %[tx]\n\t"
		"lea (%[table],%[tx],4), %[tx]\n\t"
		"lea (%[k],%[k]), %[ty]\n\t"
		"and $-8, %[ty]\n\t"
		"lea (%[ty],%[k],2), %[ty]\n\t"
		"lea (%[table],%[ty],4), %[ty]\n\t"
		"vmovdqa (%[a]), %%ymm0\n\t"
		"vmovdqa 256(%[a]), %%ymm1\n\t"
		"vbroadcastss 32(%[ty]), %%ymm2\n\t"
		"vbroadcastss (%[ty]), %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd 128(%[a]), %%ymm0, %%ymm0\n\t"
		"vpsubd 128(%[a]), %%ymm4, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpaddd 384(%[a]), %%ymm1, %%ymm1\n\t"
		"vbroadcastss 36(%[ty]), %%ymm7\n\t"
		"vpsrlq $32, %%ymm4, %%ymm8\n\t"
		"vpmuludq %%ymm2, %%ymm4, %%ymm9\n\t"
		"vpsubd 384(%[a]), %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm3, %%ymm4, %%ymm4\n\t"
		"vbroadcastss 4(%[ty]), %%ymm10\n\t"
		"vpmuludq %%ymm2, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm5, %%ymm11\n\t"
		"vpminud %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vpminud %%ymm0, %%ymm1, %%ymm0\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpaddd %%ymm0, %%ymm6, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm6\n\t"
		"vpminud %%ymm9, %%ymm1, %%ymm9\n\t"
		"vbroadcastss 32(%[tx]), %%ymm1\n\t"
		"vpsubd %%ymm0, %%ymm6, %%ymm0\n\t"
		"vmovdqa %%ymm9, (%[a])\n\t"
		"vpsrlq $32, %%ymm0, %%ymm9\n\t"
		"vpmuludq %%ymm7, %%ymm5, %%ymm6\n\t"
		"vpmuludq %%ymm7, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm8, %%ymm4, %%ymm8\n\t"
		"vpmulld %%ymm10, %%ymm5, %%ymm5\n\t"
		"vbroadcastss (%[tx]), %%ymm4\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpblendd $170, %%ymm11, %%ymm6, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpmuludq %%ymm1, %%ymm0, %%ymm12\n\t"
		"vpmuludq %%ymm1, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpsubd %%ymm11, %%ymm6, %%ymm6\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm4, %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpsrlq $32, %%ymm6, %%ymm8\n\t"
		"vpmuludq %%ymm1, %%ymm6, %%ymm5\n\t"
		"vpblendd $170, %%ymm9, %%ymm12, %%ymm9\n\t"
		"vpmulld %%ymm4, %%ymm6, %%ymm6\n\t"
		"vmovdqa 32(%[a]), %%ymm12\n\t"
		"vpmuludq %%ymm1, %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm13\n\t"
		"vpminud %%ymm13, %%ymm11, %%ymm13\n\t"
		"vmovdqa 288(%[a]), %%ymm11\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vmovdqa %%ymm13, 128(%[a])\n\t"
		"vpblendd $170, %%ymm8, %%ymm5, %%ymm8\n\t"
		"vpsubd %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm9, 256(%[a])\n\t"
		"vpsubd %%ymm8, %%ymm6, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm6\n\t"
		"vpaddd 160(%[a]), %%ymm12, %%ymm12\n\t"
		"vmovdqa %%ymm8, 384(%[a])\n\t"
		"vpsubd 160(%[a]), %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm9\n\t"
		"vpaddd 416(%[a]), %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm6, %%ymm0\n\t"
		"vpmuludq %%ymm2, %%ymm6, %%ymm5\n\t"
		"vpsubd 416(%[a]), %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm3, %%ymm6, %%ymm6\n\t"
		"vpmuludq %%ymm2, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm8, %%ymm13\n\t"
		"vpminud %%ymm9, %%ymm12, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm12\n\t"
		"vpmuludq %%ymm7, %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpmuludq %%ymm7, %%ymm8, %%ymm11\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm10, %%ymm8, %%ymm8\n\t"
		"vpblendd $170, %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpaddd %%ymm12, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm12, %%ymm5, %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpblendd $170, %%ymm13, %%ymm11, %%ymm13\n\t"
		"vpminud %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpsrlq $32, %%ymm12, %%ymm9\n\t"
		"vpmuludq %%ymm1, %%ymm12, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpmuludq %%ymm1, %%ymm9, %%ymm9\n\t"
		"vmovdqa %%ymm5, 32(%[a])\n\t"
		"vpsubd %%ymm0, %%ymm6, %%ymm0\n\t"
		"vpmulld %%ymm4, %%ymm12, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm13, %%ymm8, %%ymm13\n\t"
		"vpblendd $170, %%ymm9, %%ymm11, %%ymm9\n\t"
		"vpsubd %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpaddd %%ymm13, %%ymm0, %%ymm13\n\t"
		"vmovdqa 64(%[a]), %%ymm0\n\t"
		"vpsrlq $32, %%ymm6, %%ymm11\n\t"
		"vpmuludq %%ymm1, %%ymm6, %%ymm8\n\t"
		"vpmulld %%ymm4, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm5\n\t"
		"vpmuludq %%ymm1, %%ymm11, %%ymm11\n\t"
		"vpminud %%ymm5, %%ymm13, %%ymm5\n\t"
		"vmovdqa 320(%[a]), %%ymm13\n\t"
		"vpsubd %%ymm9, %%ymm12, %%ymm9\n\t"
		"vmovdqa %%ymm5, 160(%[a])\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm9, 288(%[a])\n\t"
		"vpblendd $170, %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpaddd 192(%[a]), %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm11, %%ymm6, %%ymm11\n\t"
		"vpsubd 192(%[a]), %%ymm8, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm9\n\t"
		"vpaddd 448(%[a]), %%ymm13, %%ymm13\n\t"
		"vmovdqa %%ymm11, 416(%[a])\n\t"
		"vpsrlq $32, %%ymm8, %%ymm11\n\t"
		"vpmuludq %%ymm2, %%ymm8, %%ymm5\n\t"
		"vpsubd 448(%[a]), %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm3, %%ymm8, %%ymm8\n\t"
		"vpmuludq %%ymm2, %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm6, %%ymm12\n\t"
		"vpminud %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm0\n\t"
		"vpmuludq %%ymm7, %%ymm12, %%ymm12\n\t"
		"vpminud %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpmuludq %%ymm7, %%ymm6, %%ymm13\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm10, %%ymm6, %%ymm6\n\t"
		"vpblendd $170, %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpaddd %%ymm0, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpblendd $170, %%ymm12, %%ymm13, %%ymm12\n\t"
		"vpminud %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpsrlq $32, %%ymm0, %%ymm9\n\t"
		"vpmuludq %%ymm1, %%ymm0, %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpmuludq %%ymm1, %%ymm9, %%ymm9\n\t"
		"vmovdqa %%ymm5, 64(%[a])\n\t"
		"vpsubd %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpmulld %%ymm4, %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm8\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpblendd $170, %%ymm9, %%ymm13, %%ymm9\n\t"
		"vpsubd %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpaddd %%ymm12, %%ymm11, %%ymm12\n\t"
		"vmovdqa 96(%[a]), %%ymm11\n\t"
		"vpsrlq $32, %%ymm8, %%ymm13\n\t"
		"vpmuludq %%ymm1, %%ymm8, %%ymm6\n\t"
		"vpmulld %%ymm4, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm5\n\t"
		"vpmuludq %%ymm1, %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm5, %%ymm12, %%ymm5\n\t"
		"vmovdqa 352(%[a]), %%ymm12\n\t"
		"vpsubd %%ymm9, %%ymm0, %%ymm9\n\t"
		"vmovdqa %%ymm5, 192(%[a])\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vmovdqa %%ymm9, 320(%[a])\n\t"
		"vpblendd $170, %%ymm13, %%ymm6, %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm6\n\t"
		"vpaddd 224(%[a]), %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm13, %%ymm8, %%ymm13\n\t"
		"vpsubd 224(%[a]), %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm9\n\t"
		"vpaddd 480(%[a]), %%ymm12, %%ymm12\n\t"
		"vmovdqa %%ymm13, 448(%[a])\n\t"
		"vpsrlq $32, %%ymm6, %%ymm13\n\t"
		"vpmuludq %%ymm2, %%ymm6, %%ymm5\n\t"
		"vpsubd 480(%[a]), %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm3, %%ymm6, %%ymm3\n\t"
		"vpmuludq %%ymm2, %%ymm13, %%ymm2\n\t"
		"vpsrlq $32, %%ymm8, %%ymm13\n\t"
		"vpmuludq %%ymm7, %%ymm8, %%ymm6\n\t"
		"vpminud %%ymm9, %%ymm11, %%ymm9\n\t"
		"vpmuludq %%ymm7, %%ymm13, %%ymm7\n\t"
		"vpmulld %%ymm10, %%ymm8, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm8\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm8, %%ymm12, %%ymm8\n\t"
		"vpblendd $170, %%ymm2, %%ymm5, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpaddd %%ymm8, %%ymm9, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm7, %%ymm6, %%ymm7\n\t"
		"vpsubd %%ymm8, %%ymm5, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm8, %%ymm6\n\t"
		"vpmuludq %%ymm1, %%ymm8, %%ymm12\n\t"
		"vpminud %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpmuludq %%ymm1, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm4, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm5, 96(%[a])\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm7, %%ymm10, %%ymm10\n\t"
		"vpblendd $170, %%ymm6, %%ymm12, %%ymm6\n\t"
		"vpaddd %%ymm7, %%ymm2, %%ymm7\n\t"
		"vpsrlq $32, %%ymm10, %%ymm2\n\t"
		"vpmuludq %%ymm1, %%ymm10, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm3\n\t"
		"vpmuludq %%ymm1, %%ymm2, %%ymm1\n\t"
		"vpmulld %%ymm4, %%ymm10, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm7, %%ymm3\n\t"
		"vmovdqa %%ymm3, 224(%[a])\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm6, %%ymm8, %%ymm6\n\t"
		"vpblendd $170, %%ymm1, %%ymm12, %%ymm1\n\t"
		"vmovdqa %%ymm6, 352(%[a])\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm1, %%ymm4, %%ymm1\n\t"
		"vmovdqa %%ymm1, 480(%[a])\n\t"
		"add $512, %[a]\n\t"
		"inc %[k]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y) {
	Vec* const end = a + 4 * count;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vbroadcastss 32(%[x]), %%ymm0\n\t"
		"vbroadcastss (%[x]), %%ymm1\n\t"
		"vmovdqa 32(%[a]), %%ymm2\n\t"
		"vbroadcastss 32(%[y]), %%ymm3\n\t"
		"vbroadcastss 36(%[y]), %%ymm4\n\t"
		"vmovdqa (%[a]), %%ymm5\n\t"
		"vbroadcastss (%[y]), %%ymm6\n\t"
		"vpmuludq 96(%[a]), %%ymm0, %%ymm7\n\t"
		"vpmuludq 100(%[a]), %%ymm0, %%ymm8\n\t"
		"vpmulld 96(%[a]), %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpmuludq 64(%[a]), %%ymm0, %%ymm11\n\t"
		"vpminud %%ymm10, %%ymm2, %%ymm10\n\t"
		"vpmuludq 68(%[a]), %%ymm0, %%ymm0\n\t"
		"vbroadcastss 4(%[y]), %%ymm2\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm12\n\t"
		"vpmulld 64(%[a]), %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm13\n\t"
		"vpblendd $170, %%ymm8, %%ymm7, %%ymm8\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpminud %%ymm13, %%ymm5, %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpblendd $170, %%ymm0, %%ymm11, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpaddd %%ymm8, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm8, %%ymm12, %%ymm8\n\t"
		"vpsubd %%ymm0, %%ymm1, %%ymm0\n\t"
		"vpsrlq $32, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm3, %%ymm10, %%ymm12\n\t"
		"vpsrlq $32, %%ymm8, %%ymm9\n\t"
		"vpmuludq %%ymm4, %%ymm8, %%ymm11\n\t"
		"vpmuludq %%ymm3, %%ymm1, %%ymm3\n\t"
		"vpmuludq %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpaddd %%ymm0, %%ymm13, %%ymm9\n\t"
		"vpsubd %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpmulld %%ymm6, %%ymm10, %%ymm6\n\t"
		"vpmulld %%ymm2, %%ymm8, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vmovdqa 160(%[a]), %%ymm13\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpminud %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpminud %%ymm10, %%ymm0, %%ymm10\n\t"
		"vbroadcastss 36(%[x]), %%ymm0\n\t"
		"vmovdqa 128(%[a]), %%ymm9\n\t"
		"vpblendd $170, %%ymm3, %%ymm12, %%ymm3\n\t"
		"vpblendd $170, %%ymm4, %%ymm11, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm12\n\t"
		"vbroadcastss 4(%[x]), %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm3, %%ymm6, %%ymm3\n\t"
		"vpsubd %%ymm4, %%ymm2, %%ymm4\n\t"
		"vpaddd %%ymm3, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm3, %%ymm11, %%ymm3\n\t"
		"vpaddd %%ymm4, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm4, %%ymm12, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm8, %%ymm11\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm6\n\t"
		"vpmuludq 224(%[a]), %%ymm0, %%ymm5\n\t"
		"vpmuludq 228(%[a]), %%ymm0, %%ymm7\n\t"
		"vpminud %%ymm12, %%ymm13, %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm13\n\t"
		"vpminud %%ymm13, %%ymm9, %%ymm13\n\t"
		"vpminud %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpminud %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpminud %%ymm6, %%ymm10, %%ymm6\n\t"
		"vbroadcastss 40(%[y]), %%ymm10\n\t"
		"vpmulld 224(%[a]), %%ymm1, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm9\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpmulld 192(%[a]), %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm9, %%ymm4, %%ymm9\n\t"
		"vpmuludq 192(%[a]), %%ymm0, %%ymm4\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vpmuludq 196(%[a]), %%ymm0, %%ymm0\n\t"
		"vbroadcastss 44(%[y]), %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpblendd $170, %%ymm0, %%ymm4, %%ymm0\n\t"
		"vbroadcastss 8(%[y]), %%ymm4\n\t"
		"vpsubd %%ymm7, %%ymm3, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vbroadcastss 12(%[y]), %%ymm3\n\t"
		"vpaddd %%ymm7, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm7, %%ymm8, %%ymm7\n\t"
		"vpsubd %%ymm15, %%ymm11, %%ymm8\n\t"
		"vpminud %%ymm8, %%ymm11, %%ymm8\n\t"
		"vpsrlq $32, %%ymm12, %%ymm11\n\t"
		"vpmuludq %%ymm10, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm0, %%ymm1, %%ymm0\n\t"
		"vmovdqa %%ymm8, (%[a])\n\t"
		"vpmuludq %%ymm10, %%ymm12, %%ymm10\n\t"
		"vpsrlq $32, %%ymm7, %%ymm8\n\t"
		"vpmuludq %%ymm5, %%ymm8, %%ymm8\n\t"
		"vpmuludq %%ymm5, %%ymm7, %%ymm5\n\t"
		"vpaddd %%ymm0, %%ymm13, %%ymm1\n\t"
		"vpsubd %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm13\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vpminud %%ymm13, %%ymm1, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm1\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpblendd $170, %%ymm11, %%ymm10, %%ymm11\n\t"
		"vpmulld %%ymm4, %%ymm12, %%ymm4\n\t"
		"vpminud %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm5, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm3, %%ymm7, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm7\n\t"
		"vmovdqa 256(%[a]), %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vpsubd %%ymm15, %%ymm2, %%ymm12\n\t"
		"vpsubd %%ymm15, %%ymm6, %%ymm10\n\t"
		"vpminud %%ymm12, %%ymm2, %%ymm12\n\t"
		"vpminud %%ymm10, %%ymm6, %%ymm10\n\t"
		"vbroadcastss 40(%[x]), %%ymm6\n\t"
		"vmovdqa 288(%[a]), %%ymm2\n\t"
		"vpsubd %%ymm11, %%ymm4, %%ymm11\n\t"
		"vmovdqa %%ymm12, 32(%[a])\n\t"
		"vbroadcastss 8(%[x]), %%ymm12\n\t"
		"vpsubd %%ymm15, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm8, %%ymm3, %%ymm8\n\t"
		"vpaddd %%ymm11, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm11, %%ymm7, %%ymm11\n\t"
		"vpminud %%ymm4, %%ymm9, %%ymm4\n\t"
		"vmovdqa %%ymm10, 64(%[a])\n\t"
		"vpaddd %%ymm8, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm10\n\t"
		"vmovdqa %%ymm4, 96(%[a])\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm8, %%ymm9\n\t"
		"vpminud %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpminud %%ymm10, %%ymm11, %%ymm10\n\t"
		"vpminud %%ymm4, %%ymm1, %%ymm4\n\t"
		"vpminud %%ymm9, %%ymm8, %%ymm9\n\t"
		"vpsubd %%ymm15, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm15, %%ymm10, %%ymm1\n\t"
		"vbroadcastss 48(%[y]), %%ymm11\n\t"
		"vpsubd %%ymm15, %%ymm4, %%ymm13\n\t"
		"vpsubd %%ymm15, %%ymm9, %%ymm7\n\t"
		"vpminud %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpminud %%ymm1, %%ymm10, %%ymm1\n\t"
		"vbroadcastss 16(%[y]), %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpminud %%ymm13, %%ymm4, %%ymm13\n\t"
		"vpminud %%ymm7, %%ymm9, %%ymm7\n\t"
		"vmovdqa %%ymm8, 128(%[a])\n\t"
		"vpmuludq 352(%[a]), %%ymm6, %%ymm8\n\t"
		"vpmuludq 356(%[a]), %%ymm6, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm4\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vmovdqa %%ymm1, 160(%[a])\n\t"
		"vbroadcastss 52(%[y]), %%ymm1\n\t"
		"vpmulld 352(%[a]), %%ymm12, %%ymm5\n\t"
		"vpminud %%ymm4, %%ymm2, %%ymm4\n\t"
		"vmovdqa %%ymm13, 192(%[a])\n\t"
		"vpmuludq 320(%[a]), %%ymm6, %%ymm13\n\t"
		"vpmuludq 324(%[a]), %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm4, %%ymm2\n\t"
		"vmovdqa %%ymm7, 224(%[a])\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vbroadcastss 20(%[y]), %%ymm7\n\t"
		"vpmulld 320(%[a]), %%ymm12, %%ymm12\n\t"
		"vpblendd $170, %%ymm9, %%ymm8, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpblendd $170, %%ymm6, %%ymm13, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm5, %%ymm9\n\t"
		"vpaddd %%ymm9, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsrlq $32, %%ymm4, %%ymm2\n\t"
		"vpmuludq %%ymm11, %%ymm4, %%ymm5\n\t"
		"vpsrlq $32, %%ymm9, %%ymm13\n\t"
		"vpmuludq %%ymm1, %%ymm9, %%ymm8\n\t"
		"vpmuludq %%ymm11, %%ymm2, %%ymm11\n\t"
		"vpmuludq %%ymm1, %%ymm13, %%ymm1\n\t"
		"vpsubd %%ymm6, %%ymm12, %%ymm6\n\t"
		"vpaddd %%ymm6, %%ymm0, %%ymm12\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpmulld %%ymm10, %%ymm4, %%ymm10\n\t"
		"vpmulld %%ymm7, %%ymm9, %%ymm7\n\t"
		"vmovdqa 384(%[a]), %%ymm9\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm0\n\t"
		"vbroadcastss 44(%[x]), %%ymm13\n\t"
		"vmovdqa 416(%[a]), %%ymm2\n\t"
		"vpblendd $170, %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpblendd $170, %%ymm1, %%ymm8, %%ymm1\n\t"
		"vpminud %%ymm4, %%ymm12, %%ymm4\n\t"
		"vpminud %%ymm0, %%ymm6, %%ymm0\n\t"
		"vbroadcastss 12(%[x]), %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm4, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm11, %%ymm10, %%ymm11\n\t"
		"vpsubd %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpaddd %%ymm11, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm11, %%ymm12, %%ymm11\n\t"
		"vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm1, %%ymm8, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpmuludq 480(%[a]), %%ymm13, %%ymm5\n\t"
		"vpmuludq 484(%[a]), %%ymm13, %%ymm3\n\t"
		"vpminud %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm9\n\t"
		"vpminud %%ymm12, %%ymm4, %%ymm12\n\t"
		"vpminud %%ymm7, %%ymm11, %%ymm7\n\t"
		"vpminud %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpminud %%ymm9, %%ymm1, %%ymm9\n\t"
		"vbroadcastss 56(%[y]), %%ymm1\n\t"
		"vpmulld 480(%[a]), %%ymm6, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm11\n\t"
		"vpsubd %%ymm15, %%ymm12, %%ymm4\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm11, %%ymm2, %%ymm11\n\t"
		"vpmulld 448(%[a]), %%ymm6, %%ymm6\n\t"
		"vpminud %%ymm4, %%ymm12, %%ymm4\n\t"
		"vbroadcastss 24(%[y]), %%ymm12\n\t"
		"vpblendd $170, %%ymm3, %%ymm5, %%ymm3\n\t"
		"vmovdqa %%ymm4, 256(%[a])\n\t"
		"vbroadcastss 60(%[y]), %%ymm4\n\t"
		"vpmuludq 448(%[a]), %%ymm13, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpmuludq 452(%[a]), %%ymm13, %%ymm13\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm3, %%ymm0, %%ymm3\n\t"
		"vpblendd $170, %%ymm13, %%ymm5, %%ymm13\n\t"
		"vbroadcastss 28(%[y]), %%ymm5\n\t"
		"vpsubd %%ymm15, %%ymm7, %%ymm0\n\t"
		"vpaddd %%ymm3, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm0, %%ymm7, %%ymm0\n\t"
		"vpsrlq $32, %%ymm11, %%ymm7\n\t"
		"vpmuludq %%ymm1, %%ymm11, %%ymm2\n\t"
		"vmovdqa %%ymm0, 288(%[a])\n\t"
		"vpmuludq %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpmulld %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpsrlq $32, %%ymm3, %%ymm11\n\t"
		"vpsubd %%ymm15, %%ymm10, %%ymm7\n\t"
		"vpmuludq %%ymm4, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm13, %%ymm6, %%ymm13\n\t"
		"vpminud %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpmuludq %%ymm4, %%ymm3, %%ymm4\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpaddd %%ymm13, %%ymm8, %%ymm10\n\t"
		"vpsubd %%ymm13, %%ymm8, %%ymm13\n\t"
		"vpsubd %%ymm15, %%ymm9, %%ymm8\n\t"
		"vmovdqa %%ymm7, 320(%[a])\n\t"
		"vpblendd $170, %%ymm1, %%ymm2, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm7\n\t"
		"vpminud %%ymm8, %%ymm9, %%ymm8\n\t"
		"vmovdqa 544(%[a]), %%ymm9\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm2, %%ymm10, %%ymm2\n\t"
		"vpminud %%ymm7, %%ymm13, %%ymm7\n\t"
		"vmovdqa %%ymm8, 352(%[a])\n\t"
		"vbroadcastss 48(%[x]), %%ymm8\n\t"
		"vmovdqa 512(%[a]), %%ymm13\n\t"
		"vbroadcastss 16(%[x]), %%ymm10\n\t"
		"vpblendd $170, %%ymm11, %%ymm4, %%ymm11\n\t"
		"vpmulld %%ymm5, %%ymm3, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vpsubd %%ymm1, %%ymm12, %%ymm1\n\t"
		"vpaddd %%ymm1, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm1, %%ymm3, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm12\n\t"
		"vpsubd %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpminud %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpminud %%ymm12, %%ymm1, %%ymm12\n\t"
		"vpaddd %%ymm11, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm11, %%ymm4, %%ymm11\n\t"
		"vpsubd %%ymm15, %%ymm3, %%ymm4\n\t"
		"vpsubd %%ymm15, %%ymm12, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm6\n\t"
		"vpminud %%ymm4, %%ymm3, %%ymm4\n\t"
		"vpmuludq 608(%[a]), %%ymm8, %%ymm3\n\t"
		"vpmuludq 612(%[a]), %%ymm8, %%ymm0\n\t"
		"vpminud %%ymm2, %%ymm9, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm9\n\t"
		"vmovdqa %%ymm4, 384(%[a])\n\t"
		"vpminud %%ymm9, %%ymm13, %%ymm9\n\t"
		"vpminud %%ymm5, %%ymm7, %%ymm5\n\t"
		"vpminud %%ymm6, %%ymm11, %%ymm6\n\t"
		"vpminud %%ymm1, %%ymm12, %%ymm1\n\t"
		"vbroadcastss 96(%[y]), %%ymm12\n\t"
		"vbroadcastss 64(%[y]), %%ymm11\n\t"
		"vbroadcastss 100(%[y]), %%ymm7\n\t"
		"vpmulld 608(%[a]), %%ymm10, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm4\n\t"
		"vmovdqa %%ymm1, 416(%[a])\n\t"
		"vpsrlq $32, %%ymm3, %%ymm3\n\t"
		"vpmulld 576(%[a]), %%ymm10, %%ymm10\n\t"
		"vpmuludq 576(%[a]), %%ymm8, %%ymm1\n\t"
		"vpblendd $170, %%ymm0, %%ymm3, %%ymm0\n\t"
		"vpmuludq 580(%[a]), %%ymm8, %%ymm8\n\t"
		"vbroadcastss 68(%[y]), %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpsubd %%ymm15, %%ymm5, %%ymm1\n\t"
		"vpsubd %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpminud %%ymm1, %%ymm5, %%ymm1\n\t"
		"vpsubd %%ymm15, %%ymm6, %%ymm5\n\t"
		"vpaddd %%ymm0, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm0, %%ymm4, %%ymm0\n\t"
		"vpminud %%ymm5, %%ymm6, %%ymm5\n\t"
		"vmovdqa %%ymm1, 448(%[a])\n\t"
		"vpsrlq $32, %%ymm2, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm2, %%ymm6\n\t"
		"vpsrlq $32, %%ymm0, %%ymm4\n\t"
		"vpmuludq %%ymm7, %%ymm0, %%ymm13\n\t"
		"vmovdqa %%ymm5, 480(%[a])\n\t"
		"vpmuludq %%ymm12, %%ymm1, %%ymm12\n\t"
		"vpmuludq %%ymm7, %%ymm4, %%ymm7\n\t"
		"vpsubd %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpaddd %%ymm8, %%ymm9, %%ymm10\n\t"
		"vpsubd %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpmulld %%ymm11, %%ymm2, %%ymm11\n\t"
		"vpmulld %%ymm3, %%ymm0, %%ymm3\n\t"
		"vmovdqa 640(%[a]), %%ymm0\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm9\n\t"
		"vbroadcastss 52(%[x]), %%ymm4\n\t"
		"vmovdqa 672(%[a]), %%ymm1\n\t"
		"vpblendd $170, %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpblendd $170, %%ymm7, %%ymm13, %%ymm7\n\t"
		"vpminud %%ymm2, %%ymm10, %%ymm2\n\t"
		"vpminud %%ymm9, %%ymm8, %%ymm9\n\t"
		"vbroadcastss 20(%[x]), %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm13\n\t"
		"vpsubd %%ymm12, %%ymm11, %%ymm12\n\t"
		"vpsubd %%ymm7, %%ymm3, %%ymm7\n\t"
		"vpaddd %%ymm12, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm12, %%ymm10, %%ymm12\n\t"
		"vpaddd %%ymm7, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm7, %%ymm13, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm13\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm11\n\t"
		"vpmuludq 736(%[a]), %%ymm4, %%ymm6\n\t"
		"vpmuludq 740(%[a]), %%ymm4, %%ymm5\n\t"
		"vpminud %%ymm13, %%ymm0, %%ymm13\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm0\n\t"
		"vpminud %%ymm10, %%ymm2, %%ymm10\n\t"
		"vpminud %%ymm3, %%ymm12, %%ymm3\n\t"
		"vpminud %%ymm11, %%ymm9, %%ymm11\n\t"
		"vpminud %%ymm0, %%ymm7, %%ymm0\n\t"
		"vbroadcastss 104(%[y]), %%ymm7\n\t"
		"vpmulld 736(%[a]), %%ymm8, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm12\n\t"
		"vpsubd %%ymm15, %%ymm10, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpminud %%ymm12, %%ymm1, %%ymm12\n\t"
		"vpmulld 704(%[a]), %%ymm8, %%ymm8\n\t"
		"vpminud %%ymm2, %%ymm10, %%ymm2\n\t"
		"vbroadcastss 72(%[y]), %%ymm10\n\t"
		"vpblendd $170, %%ymm5, %%ymm6, %%ymm5\n\t"
		"vmovdqa %%ymm2, 512(%[a])\n\t"
		"vbroadcastss 108(%[y]), %%ymm2\n\t"
		"vpmuludq 704(%[a]), %%ymm4, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpmuludq 708(%[a]), %%ymm4, %%ymm4\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpblendd $170, %%ymm4, %%ymm6, %%ymm4\n\t"
		"vbroadcastss 76(%[y]), %%ymm6\n\t"
		"vpsubd %%ymm15, %%ymm3, %%ymm9\n\t"
		"vpaddd %%ymm5, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm5, %%ymm1, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpminud %%ymm9, %%ymm3, %%ymm9\n\t"
		"vpsrlq $32, %%ymm12, %%ymm3\n\t"
		"vpmuludq %%ymm7, %%ymm12, %%ymm1\n\t"
		"vmovdqa %%ymm9, 544(%[a])\n\t"
		"vpmuludq %%ymm7, %%ymm3, %%ymm7\n\t"
		"vpmulld %%ymm10, %%ymm12, %%ymm10\n\t"
		"vpsrlq $32, %%ymm5, %%ymm12\n\t"
		"vpsubd %%ymm15, %%ymm11, %%ymm3\n\t"
		"vpmuludq %%ymm2, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm4, %%ymm8, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm11, %%ymm3\n\t"
		"vpmuludq %%ymm2, %%ymm5, %%ymm2\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm4, %%ymm13, %%ymm11\n\t"
		"vpsubd %%ymm4, %%ymm13, %%ymm4\n\t"
		"vpsubd %%ymm15, %%ymm0, %%ymm13\n\t"
		"vmovdqa %%ymm3, 576(%[a])\n\t"
		"vpblendd $170, %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm4, %%ymm3\n\t"
		"vpminud %%ymm13, %%ymm0, %%ymm13\n\t"
		"vmovdqa 800(%[a]), %%ymm0\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpminud %%ymm1, %%ymm11, %%ymm1\n\t"
		"vpminud %%ymm3, %%ymm4, %%ymm3\n\t"
		"vmovdqa %%ymm13, 608(%[a])\n\t"
		"vbroadcastss 56(%[x]), %%ymm13\n\t"
		"vmovdqa 768(%[a]), %%ymm4\n\t"
		"vbroadcastss 24(%[x]), %%ymm11\n\t"
		"vpblendd $170, %%ymm12, %%ymm2, %%ymm12\n\t"
		"vpmulld %%ymm6, %%ymm5, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpaddd %%ymm7, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm7, %%ymm5, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm10\n\t"
		"vpsubd %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpminud %%ymm5, %%ymm1, %%ymm5\n\t"
		"vpminud %%ymm10, %%ymm7, %%ymm10\n\t"
		"vpaddd %%ymm12, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm12, %%ymm2, %%ymm12\n\t"
		"vpsubd %%ymm15, %%ymm5, %%ymm2\n\t"
		"vpsubd %%ymm15, %%ymm10, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm8\n\t"
		"vpminud %%ymm2, %%ymm5, %%ymm2\n\t"
		"vpmuludq 864(%[a]), %%ymm13, %%ymm5\n\t"
		"vpmuludq 868(%[a]), %%ymm13, %%ymm9\n\t"
		"vpminud %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm0\n\t"
		"vmovdqa %%ymm2, 640(%[a])\n\t"
		"vpminud %%ymm0, %%ymm4, %%ymm0\n\t"
		"vpminud %%ymm6, %%ymm3, %%ymm6\n\t"
		"vpminud %%ymm8, %%ymm12, %%ymm8\n\t"
		"vpminud %%ymm7, %%ymm10, %%ymm7\n\t"
		"vbroadcastss 112(%[y]), %%ymm10\n\t"
		"vbroadcastss 80(%[y]), %%ymm12\n\t"
		"vbroadcastss 116(%[y]), %%ymm3\n\t"
		"vpmulld 864(%[a]), %%ymm11, %%ymm4\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm2\n\t"
		"vmovdqa %%ymm7, 672(%[a])\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpmulld 832(%[a]), %%ymm11, %%ymm11\n\t"
		"vpmuludq 832(%[a]), %%ymm13, %%ymm7\n\t"
		"vpblendd $170, %%ymm9, %%ymm5, %%ymm9\n\t"
		"vpmuludq 836(%[a]), %%ymm13, %%ymm13\n\t"
		"vbroadcastss 84(%[y]), %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpblendd $170, %%ymm13, %%ymm7, %%ymm13\n\t"
		"vpsubd %%ymm15, %%ymm6, %%ymm7\n\t"
		"vpsubd %%ymm9, %%ymm4, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm7, %%ymm6, %%ymm7\n\t"
		"vpsubd %%ymm15, %%ymm8, %%ymm6\n\t"
		"vpaddd %%ymm9, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpminud %%ymm6, %%ymm8, %%ymm6\n\t"
		"vmovdqa %%ymm7, 704(%[a])\n\t"
		"vpsrlq $32, %%ymm1, %%ymm7\n\t"
		"vpmuludq %%ymm10, %%ymm1, %%ymm8\n\t"
		"vpsrlq $32, %%ymm9, %%ymm2\n\t"
		"vpmuludq %%ymm3, %%ymm9, %%ymm4\n\t"
		"vmovdqa %%ymm6, 736(%[a])\n\t"
		"vpmuludq %%ymm10, %%ymm7, %%ymm10\n\t"
		"vpmuludq %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpsubd %%ymm13, %%ymm11, %%ymm13\n\t"
		"vpaddd %%ymm13, %%ymm0, %%ymm11\n\t"
		"vpsubd %%ymm13, %%ymm0, %%ymm13\n\t"
		"vpmulld %%ymm12, %%ymm1, %%ymm12\n\t"
		"vpmulld %%ymm5, %%ymm9, %%ymm5\n\t"
		"vmovdqa 896(%[a]), %%ymm9\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm13, %%ymm0\n\t"
		"vbroadcastss 60(%[x]), %%ymm2\n\t"
		"vmovdqa 928(%[a]), %%ymm7\n\t"
		"vpblendd $170, %%ymm10, %%ymm8, %%ymm10\n\t"
		"vpblendd $170, %%ymm3, %%ymm4, %%ymm3\n\t"
		"vpminud %%ymm1, %%ymm11, %%ymm1\n\t"
		"vpminud %%ymm0, %%ymm13, %%ymm0\n\t"
		"vbroadcastss 28(%[x]), %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpsubd %%ymm10, %%ymm12, %%ymm10\n\t"
		"vpsubd %%ymm3, %%ymm5, %%ymm3\n\t"
		"vpaddd %%ymm10, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm10, %%ymm11, %%ymm10\n\t"
		"vpaddd %%ymm3, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm3, %%ymm4, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm11\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm12\n\t"
		"vpmuludq 992(%[a]), %%ymm2, %%ymm8\n\t"
		"vpmuludq 996(%[a]), %%ymm2, %%ymm6\n\t"
		"vpminud %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm9\n\t"
		"vpminud %%ymm11, %%ymm1, %%ymm11\n\t"
		"vpminud %%ymm5, %%ymm10, %%ymm5\n\t"
		"vpminud %%ymm12, %%ymm0, %%ymm12\n\t"
		"vpminud %%ymm9, %%ymm3, %%ymm9\n\t"
		"vbroadcastss 120(%[y]), %%ymm3\n\t"
		"vpmulld 992(%[a]), %%ymm13, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm10\n\t"
		"vpsubd %%ymm15, %%ymm11, %%ymm1\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpminud %%ymm10, %%ymm7, %%ymm10\n\t"
		"vpmulld 960(%[a]), %%ymm13, %%ymm13\n\t"
		"vpminud %%ymm1, %%ymm11, %%ymm1\n\t"
		"vbroadcastss 88(%[y]), %%ymm11\n\t"
		"vpblendd $170, %%ymm6, %%ymm8, %%ymm6\n\t"
		"vmovdqa %%ymm1, 768(%[a])\n\t"
		"vbroadcastss 124(%[y]), %%ymm1\n\t"
		"vpmuludq 960(%[a]), %%ymm2, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpmuludq 964(%[a]), %%ymm2, %%ymm2\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpblendd $170, %%ymm2, %%ymm8, %%ymm2\n\t"
		"vbroadcastss 92(%[y]), %%ymm8\n\t"
		"vpsubd %%ymm15, %%ymm5, %%ymm0\n\t"
		"vpaddd %%ymm6, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsrlq $32, %%ymm10, %%ymm5\n\t"
		"vpmuludq %%ymm3, %%ymm10, %%ymm7\n\t"
		"vmovdqa %%ymm0, 800(%[a])\n\t"
		"vpmuludq %%ymm3, %%ymm5, %%ymm3\n\t"
		"vpmulld %%ymm11, %%ymm10, %%ymm11\n\t"
		"vpsrlq $32, %%ymm6, %%ymm10\n\t"
		"vpsubd %%ymm15, %%ymm12, %%ymm5\n\t"
		"vpmuludq %%ymm1, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm2, %%ymm13, %%ymm2\n\t"
		"vpminud %%ymm5, %%ymm12, %%ymm5\n\t"
		"vpmuludq %%ymm1, %%ymm6, %%ymm1\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpaddd %%ymm2, %%ymm4, %%ymm12\n\t"
		"vpsubd %%ymm2, %%ymm4, %%ymm2\n\t"
		"vpsubd %%ymm15, %%ymm9, %%ymm4\n\t"
		"vmovdqa %%ymm5, 832(%[a])\n\t"
		"vpblendd $170, %%ymm3, %%ymm7, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm5\n\t"
		"vpminud %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm12, %%ymm7\n\t"
		"vpminud %%ymm5, %%ymm2, %%ymm5\n\t"
		"vmovdqa %%ymm4, 864(%[a])\n\t"
		"vpblendd $170, %%ymm10, %%ymm1, %%ymm10\n\t"
		"vpmulld %%ymm8, %%ymm6, %%ymm8\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm3, %%ymm11, %%ymm3\n\t"
		"vpaddd %%ymm3, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm3, %%ymm6, %%ymm3\n\t"
		"vpsubd %%ymm10, %%ymm8, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm6\n\t"
		"vpaddd %%ymm10, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm10, %%ymm1, %%ymm10\n\t"
		"vpminud %%ymm8, %%ymm7, %%ymm8\n\t"
		"vpminud %%ymm6, %%ymm3, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm7\n\t"
		"vpsubd %%ymm15, %%ymm8, %%ymm1\n\t"
		"vpsubd %%ymm15, %%ymm6, %%ymm11\n\t"
		"vpminud %%ymm3, %%ymm5, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpminud %%ymm1, %%ymm8, %%ymm1\n\t"
		"vpminud %%ymm11, %%ymm6, %%ymm11\n\t"
		"vpsubd %%ymm15, %%ymm3, %%ymm6\n\t"
		"vpsubd %%ymm15, %%ymm7, %%ymm8\n\t"
		"vmovdqa %%ymm1, 896(%[a])\n\t"
		"vpminud %%ymm6, %%ymm3, %%ymm6\n\t"
		"vpminud %%ymm8, %%ymm7, %%ymm8\n\t"
		"vmovdqa %%ymm11, 928(%[a])\n\t"
		"vmovdqa %%ymm6, 960(%[a])\n\t"
		"vmovdqa %%ymm8, 992(%[a])\n\t"
		"add $1024, %[a]\n\t"
		"add $64, %[x]\n\t"
		"add $128, %[y]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y, const Vec* from) {
	Vec* const end = a + 4 * count;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[from]), %%ymm0\n\t"
		"vmovdqa 64(%[from]), %%ymm1\n\t"
		"vbroadcastss 32(%[y]), %%ymm2\n\t"
		"vbroadcastss 36(%[y]), %%ymm3\n\t"
		"vbroadcastss (%[y]), %%ymm4\n\t"
		"vbroadcastss 4(%[y]), %%ymm5\n\t"
		"vbroadcastss 32(%[x]), %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm7\n\t"
		"vpaddd 32(%[from]), %%ymm0, %%ymm0\n\t"
		"vpsubd 32(%[from]), %%ymm7, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm9\n\t"
		"vpaddd 96(%[from]), %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm7, %%ymm10\n\t"
		"vpmuludq %%ymm2, %%ymm7, %%ymm11\n\t"
		"vpsubd 96(%[from]), %%ymm8, %%ymm8\n\t"
		"vpminud %%ymm9, %%ymm0, %%ymm9\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vpmuludq %%ymm2, %%ymm10, %%ymm2\n\t"
		"vpsrlq $32, %%ymm8, %%ymm10\n\t"
		"vpmuludq %%ymm3, %%ymm8, %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm13\n\t"
		"vpmuludq %%ymm3, %%ymm10, %%ymm3\n\t"
		"vpmulld %%ymm4, %%ymm7, %%ymm4\n\t"
		"vpminud %%ymm13, %%ymm1, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm1\n\t"
		"vmovdqa 128(%[from]), %%ymm7\n\t"
		"vpsrlq $32, %%ymm11, %%ymm11\n\t"
		"vpmulld %%ymm5, %%ymm8, %%ymm5\n\t"
		"vpsubd %%ymm13, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm13, %%ymm9, %%ymm13\n\t"
		"vmovdqa 192(%[from]), %%ymm9\n\t"
		"vpblendd $170, %%ymm2, %%ymm11, %%ymm2\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsrlq $32, %%ymm1, %%ymm11\n\t"
		"vpmuludq %%ymm6, %%ymm1, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm3, %%ymm12, %%ymm3\n\t"
		"vpmuludq %%ymm6, %%ymm11, %%ymm11\n\t"
		"vbroadcastss 40(%[y]), %%ymm12\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpminud %%ymm10, %%ymm13, %%ymm10\n\t"
		"vbroadcastss 8(%[y]), %%ymm13\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm10, (%[a])\n\t"
		"vpsubd %%ymm2, %%ymm4, %%ymm2\n\t"
		"vpblendd $170, %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpmulld %%ymm0, %%ymm1, %%ymm1\n\t"
		"vbroadcastss 44(%[y]), %%ymm8\n\t"
		"vpsubd %%ymm3, %%ymm5, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vbroadcastss 12(%[y]), %%ymm4\n\t"
		"vpsubd %%ymm3, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm2\n\t"
		"vpaddd 160(%[from]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm5, %%ymm10\n\t"
		"vpmulld %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsubd 160(%[from]), %%ymm2, %%ymm2\n\t"
		"vpmuludq %%ymm6, %%ymm5, %%ymm5\n\t"
		"vpmuludq %%ymm6, %%ymm10, %%ymm6\n\t"
		"vpsubd %%ymm11, %%ymm1, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm1\n\t"
		"vpsrlq $32, %%ymm2, %%ymm10\n\t"
		"vpaddd 224(%[from]), %%ymm9, %%ymm9\n\t"
		"vmovdqa %%ymm11, 64(%[a])\n\t"
		"vpsubd 224(%[from]), %%ymm1, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm13, %%ymm2, %%ymm13\n\t"
		"vbroadcastss 36(%[x]), %%ymm11\n\t"
		"vpmuludq %%ymm12, %%ymm2, %%ymm12\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm1, %%ymm2\n\t"
		"vpblendd $170, %%ymm6, %%ymm5, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm5\n\t"
		"vpmuludq %%ymm8, %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm5, %%ymm7, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm7\n\t"
		"vpmuludq %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm4, %%ymm1, %%ymm4\n\t"
		"vpminud %%ymm7, %%ymm9, %%ymm7\n\t"
		"vbroadcastss 4(%[x]), %%ymm9\n\t"
		"vpblendd $170, %%ymm10, %%ymm12, %%ymm10\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm12\n\t"
		"vpaddd %%ymm7, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm7, %%ymm12, %%ymm7\n\t"
		"vpminud %%ymm1, %%ymm3, %%ymm1\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vmovdqa 256(%[from]), %%ymm0\n\t"
		"vmovdqa 320(%[from]), %%ymm3\n\t"
		"vpsrlq $32, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm7, %%ymm12\n\t"
		"vmovdqa %%ymm1, 32(%[a])\n\t"
		"vpblendd $170, %%ymm2, %%ymm8, %%ymm2\n\t"
		"vpmuludq %%ymm11, %%ymm12, %%ymm12\n\t"
		"vmovdqa %%ymm6, 96(%[a])\n\t"
		"vpmuludq %%ymm11, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm10, %%ymm13, %%ymm10\n\t"
		"vpminud %%ymm8, %%ymm5, %%ymm8\n\t"
		"vbroadcastss 48(%[y]), %%ymm5\n\t"
		"vbroadcastss 16(%[y]), %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm1\n\t"
		"vmovdqa %%ymm8, 128(%[a])\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm2, %%ymm4, %%ymm2\n\t"
		"vpblendd $170, %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpmulld %%ymm9, %%ymm7, %%ymm7\n\t"
		"vbroadcastss 52(%[y]), %%ymm6\n\t"
		"vpsubd %%ymm2, %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpaddd %%ymm2, %%ymm10, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vbroadcastss 20(%[y]), %%ymm4\n\t"
		"vpaddd 288(%[from]), %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm8\n\t"
		"vpmulld %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd 288(%[from]), %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm8, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm8\n\t"
		"vpsubd 352(%[from]), %%ymm8, %%ymm8\n\t"
		"vpaddd 352(%[from]), %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm12, %%ymm7, %%ymm12\n\t"
		"vpsrlq $32, %%ymm10, %%ymm7\n\t"
		"vpmuludq %%ymm5, %%ymm7, %%ymm7\n\t"
		"vmovdqa %%ymm12, 192(%[a])\n\t"
		"vpmuludq %%ymm5, %%ymm10, %%ymm5\n\t"
		"vpsrlq $32, %%ymm8, %%ymm12\n\t"
		"vpmulld %%ymm13, %%ymm10, %%ymm13\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vbroadcastss 40(%[x]), %%ymm10\n\t"
		"vpblendd $170, %%ymm11, %%ymm1, %%ymm11\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm6, %%ymm12, %%ymm12\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vbroadcastss 8(%[x]), %%ymm0\n\t"
		"vpmuludq %%ymm6, %%ymm8, %%ymm6\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpmulld %%ymm4, %%ymm8, %%ymm4\n\t"
		"vpminud %%ymm5, %%ymm3, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm3\n\t"
		"vpsubd %%ymm5, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm5, %%ymm1, %%ymm5\n\t"
		"vpsubd %%ymm11, %%ymm9, %%ymm11\n\t"
		"vmovdqa 384(%[from]), %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm1\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpminud %%ymm1, %%ymm2, %%ymm1\n\t"
		"vmovdqa %%ymm11, 224(%[a])\n\t"
		"vmovdqa 448(%[from]), %%ymm11\n\t"
		"vpsrlq $32, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm8\n\t"
		"vpblendd $170, %%ymm12, %%ymm6, %%ymm12\n\t"
		"vpsubd %%ymm7, %%ymm13, %%ymm7\n\t"
		"vpminud %%ymm8, %%ymm5, %%ymm8\n\t"
		"vmovdqa %%ymm1, 160(%[a])\n\t"
		"vbroadcastss 56(%[y]), %%ymm1\n\t"
		"vbroadcastss 24(%[y]), %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm13\n\t"
		"vpmuludq %%ymm10, %%ymm3, %%ymm6\n\t"
		"vmovdqa %%ymm8, 256(%[a])\n\t"
		"vpmuludq %%ymm10, %%ymm2, %%ymm2\n\t"
		"vpmulld %%ymm0, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm12, %%ymm4, %%ymm12\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vbroadcastss 60(%[y]), %%ymm4\n\t"
		"vpsubd %%ymm12, %%ymm13, %%ymm13\n\t"
		"vpblendd $170, %%ymm2, %%ymm6, %%ymm2\n\t"
		"vpaddd %%ymm12, %%ymm7, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm7\n\t"
		"vbroadcastss 28(%[y]), %%ymm6\n\t"
		"vpaddd 416(%[from]), %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm13, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpsubd 416(%[from]), %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm10, %%ymm8, %%ymm8\n\t"
		"vpmulld %%ymm0, %%ymm13, %%ymm0\n\t"
		"vpmuludq %%ymm10, %%ymm13, %%ymm10\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm13\n\t"
		"vpaddd 480(%[from]), %%ymm11, %%ymm11\n\t"
		"vpsubd 480(%[from]), %%ymm13, %%ymm13\n\t"
		"vpmulld %%ymm5, %%ymm7, %%ymm5\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpsrlq $32, %%ymm7, %%ymm3\n\t"
		"vpmuludq %%ymm1, %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm1, %%ymm3, %%ymm1\n\t"
		"vmovdqa %%ymm2, 320(%[a])\n\t"
		"vpsrlq $32, %%ymm13, %%ymm2\n\t"
		"vbroadcastss 44(%[x]), %%ymm3\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm4, %%ymm2, %%ymm2\n\t"
		"vpblendd $170, %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpmuludq %%ymm4, %%ymm13, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm10\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpminud %%ymm10, %%ymm9, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm9\n\t"
		"vpblendd $170, %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpmulld %%ymm6, %%ymm13, %%ymm6\n\t"
		"vpminud %%ymm9, %%ymm11, %%ymm9\n\t"
		"vbroadcastss 12(%[x]), %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm13\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm9, %%ymm13, %%ymm13\n\t"
		"vpaddd %%ymm9, %%ymm10, %%ymm9\n\t"
		"vmovdqa 512(%[from]), %%ymm10\n\t"
		"vpblendd $170, %%ymm2, %%ymm4, %%ymm2\n\t"
		"vpsrlq $32, %%ymm13, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm12, %%ymm7\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm7, %%ymm12, %%ymm7\n\t"
		"vmovdqa %%ymm8, 352(%[a])\n\t"
		"vmovdqa 576(%[from]), %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm12\n\t"
		"vpsubd %%ymm1, %%ymm5, %%ymm1\n\t"
		"vpmuludq %%ymm3, %%ymm4, %%ymm4\n\t"
		"vpminud %%ymm12, %%ymm9, %%ymm12\n\t"
		"vmovdqa %%ymm7, 288(%[a])\n\t"
		"vbroadcastss 96(%[y]), %%ymm7\n\t"
		"vpmuludq %%ymm3, %%ymm13, %%ymm9\n\t"
		"vbroadcastss 64(%[y]), %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vmovdqa %%ymm12, 384(%[a])\n\t"
		"vpsubd %%ymm2, %%ymm6, %%ymm2\n\t"
		"vbroadcastss 100(%[y]), %%ymm6\n\t"
		"vpsubd %%ymm2, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpaddd %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpsrlq $32, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm3, %%ymm0, %%ymm12\n\t"
		"vpblendd $170, %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpmulld %%ymm11, %%ymm13, %%ymm13\n\t"
		"vpmuludq %%ymm3, %%ymm1, %%ymm3\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vbroadcastss 68(%[y]), %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm9\n\t"
		"vpmulld %%ymm11, %%ymm0, %%ymm11\n\t"
		"vpminud %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm2\n\t"
		"vpaddd 544(%[from]), %%ymm10, %%ymm10\n\t"
		"vpsubd 544(%[from]), %%ymm2, %%ymm2\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vmovdqa %%ymm9, 416(%[a])\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm10, %%ymm0\n\t"
		"vpsubd 608(%[from]), %%ymm9, %%ymm9\n\t"
		"vpaddd 608(%[from]), %%ymm8, %%ymm8\n\t"
		"vpminud %%ymm0, %%ymm10, %%ymm0\n\t"
		"vpblendd $170, %%ymm3, %%ymm12, %%ymm3\n\t"
		"vpsrlq $32, %%ymm2, %%ymm12\n\t"
		"vpmuludq %%ymm7, %%ymm2, %%ymm10\n\t"
		"vpmulld %%ymm5, %%ymm2, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm8, %%ymm2\n\t"
		"vpmuludq %%ymm7, %%ymm12, %%ymm7\n\t"
		"vpminud %%ymm2, %%ymm8, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm4, %%ymm13, %%ymm4\n\t"
		"vbroadcastss 48(%[x]), %%ymm13\n\t"
		"vbroadcastss 16(%[x]), %%ymm8\n\t"
		"vmovdqa %%ymm4, 448(%[a])\n\t"
		"vpsrlq $32, %%ymm9, %%ymm4\n\t"
		"vpmuludq %%ymm6, %%ymm9, %%ymm12\n\t"
		"vpmuludq %%ymm6, %%ymm4, %%ymm6\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm7, %%ymm10, %%ymm7\n\t"
		"vpmulld %%ymm1, %%ymm9, %%ymm1\n\t"
		"vpsubd %%ymm2, %%ymm4, %%ymm2\n\t"
		"vpsubd %%ymm3, %%ymm11, %%ymm3\n\t"
		"vmovdqa 640(%[from]), %%ymm11\n\t"
		"vmovdqa 704(%[from]), %%ymm4\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm2, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vmovdqa %%ymm3, 480(%[a])\n\t"
		"vpblendd $170, %%ymm6, %%ymm12, %%ymm6\n\t"
		"vpminud %%ymm10, %%ymm0, %%ymm10\n\t"
		"vbroadcastss 104(%[y]), %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vmovdqa %%ymm10, 512(%[a])\n\t"
		"vbroadcastss 72(%[y]), %%ymm10\n\t"
		"vpsubd %%ymm7, %%ymm5, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm5\n\t"
		"vpmuludq %%ymm13, %%ymm2, %%ymm12\n\t"
		"vpmuludq %%ymm13, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpmulld %%ymm8, %%ymm2, %%ymm2\n\t"
		"vbroadcastss 108(%[y]), %%ymm1\n\t"
		"vpsubd %%ymm6, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm7\n\t"
		"vpaddd 672(%[from]), %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm5, %%ymm3\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsubd 672(%[from]), %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm13, %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm9, %%ymm12, %%ymm9\n\t"
		"vpmulld %%ymm8, %%ymm5, %%ymm8\n\t"
		"vbroadcastss 76(%[y]), %%ymm12\n\t"
		"vpmuludq %%ymm13, %%ymm5, %%ymm13\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm4, %%ymm5\n\t"
		"vpaddd 736(%[from]), %%ymm4, %%ymm4\n\t"
		"vpsubd 736(%[from]), %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm10, %%ymm7, %%ymm10\n\t"
		"vpsrlq $32, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm2\n\t"
		"vpmuludq %%ymm0, %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm0, %%ymm2, %%ymm0\n\t"
		"vpblendd $170, %%ymm3, %%ymm13, %%ymm3\n\t"
		"vmovdqa %%ymm9, 576(%[a])\n\t"
		"vpsrlq $32, %%ymm5, %%ymm9\n\t"
		"vbroadcastss 52(%[x]), %%ymm13\n\t"
		"vpsubd %%ymm14, %%ymm11, %%ymm2\n\t"
		"vpmuludq %%ymm1, %%ymm9, %%ymm9\n\t"
		"vpminud %%ymm2, %%ymm11, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm11\n\t"
		"vpmuludq %%ymm1, %%ymm5, %%ymm1\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpminud %%ymm11, %%ymm4, %%ymm11\n\t"
		"vbroadcastss 20(%[x]), %%ymm4\n\t"
		"vpblendd $170, %%ymm0, %%ymm7, %%ymm0\n\t"
		"vpmulld %%ymm12, %%ymm5, %%ymm12\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm5\n\t"
		"vpaddd %%ymm11, %%ymm2, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm11, %%ymm5, %%ymm11\n\t"
		"vpsubd %%ymm3, %%ymm8, %%ymm3\n\t"
		"vmovdqa 768(%[from]), %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm5\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm5, %%ymm6, %%ymm5\n\t"
		"vmovdqa %%ymm3, 608(%[a])\n\t"
		"vmovdqa 832(%[from]), %%ymm3\n\t"
		"vpsrlq $32, %%ymm11, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpblendd $170, %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpminud %%ymm7, %%ymm2, %%ymm7\n\t"
		"vmovdqa %%ymm5, 544(%[a])\n\t"
		"vbroadcastss 112(%[y]), %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm0, %%ymm10, %%ymm0\n\t"
		"vmovdqa %%ymm7, 640(%[a])\n\t"
		"vbroadcastss 80(%[y]), %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpmuludq %%ymm13, %%ymm11, %%ymm2\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm12, %%ymm9\n\t"
		"vpmulld %%ymm4, %%ymm11, %%ymm11\n\t"
		"vbroadcastss 116(%[y]), %%ymm12\n\t"
		"vpsubd %%ymm9, %%ymm10, %%ymm10\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpaddd %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm0\n\t"
		"vbroadcastss 84(%[y]), %%ymm1\n\t"
		"vpaddd 800(%[from]), %%ymm8, %%ymm8\n\t"
		"vpblendd $170, %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpmulld %%ymm4, %%ymm10, %%ymm4\n\t"
		"vpsubd 800(%[from]), %%ymm0, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpaddd 864(%[from]), %%ymm3, %%ymm3\n\t"
		"vpsubd 864(%[from]), %%ymm2, %%ymm2\n\t"
		"vpmulld %%ymm7, %%ymm0, %%ymm7\n\t"
		"vpsubd %%ymm6, %%ymm11, %%ymm6\n\t"
		"vpsrlq $32, %%ymm0, %%ymm11\n\t"
		"vpmuludq %%ymm5, %%ymm0, %%ymm0\n\t"
		"vpmuludq %%ymm5, %%ymm11, %%ymm5\n\t"
		"vmovdqa %%ymm6, 704(%[a])\n\t"
		"vpsrlq $32, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm8, %%ymm11\n\t"
		"vpminud %%ymm11, %%ymm8, %%ymm11\n\t"
		"vbroadcastss 56(%[x]), %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm6, %%ymm6\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpmuludq %%ymm12, %%ymm2, %%ymm12\n\t"
		"vpblendd $170, %%ymm5, %%ymm0, %%ymm5\n\t"
		"vpmulld %%ymm1, %%ymm2, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vbroadcastss 24(%[x]), %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm11, %%ymm3\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm11, %%ymm2\n\t"
		"vpsrlq $32, %%ymm10, %%ymm11\n\t"
		"vpblendd $170, %%ymm6, %%ymm12, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm13, %%ymm11, %%ymm13\n\t"
		"vpsrlq $32, %%ymm3, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm5, %%ymm7, %%ymm5\n\t"
		"vmovdqa 896(%[from]), %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm12\n\t"
		"vpminud %%ymm12, %%ymm9, %%ymm12\n\t"
		"vmovdqa 960(%[from]), %%ymm9\n\t"
		"vpmuludq %%ymm8, %%ymm11, %%ymm11\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vmovdqa %%ymm12, 672(%[a])\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm12\n\t"
		"vpsubd %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpblendd $170, %%ymm13, %%ymm10, %%ymm13\n\t"
		"vbroadcastss 120(%[y]), %%ymm10\n\t"
		"vpmuludq %%ymm8, %%ymm3, %%ymm1\n\t"
		"vpsubd %%ymm6, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpmulld %%ymm0, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm6, %%ymm5, %%ymm6\n\t"
		"vbroadcastss 88(%[y]), %%ymm5\n\t"
		"vpmulld %%ymm0, %%ymm12, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm11, %%ymm1, %%ymm11\n\t"
		"vpsubd %%ymm13, %%ymm4, %%ymm13\n\t"
		"vbroadcastss 124(%[y]), %%ymm4\n\t"
		"vpsrlq $32, %%ymm12, %%ymm1\n\t"
		"vpmuludq %%ymm8, %%ymm12, %%ymm12\n\t"
		"vpmuludq %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vmovdqa %%ymm13, 736(%[a])\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm13\n\t"
		"vbroadcastss 92(%[y]), %%ymm1\n\t"
		"vpaddd 928(%[from]), %%ymm7, %%ymm7\n\t"
		"vpsubd 928(%[from]), %%ymm13, %%ymm13\n\t"
		"vpsrlq $32, %%ymm12, %%ymm12\n\t"
		"vpblendd $170, %%ymm8, %%ymm12, %%ymm8\n\t"
		"vpsubd %%ymm11, %%ymm3, %%ymm11\n\t"
		"vpaddd %%ymm14, %%ymm9, %%ymm3\n\t"
		"vpsrlq $32, %%ymm13, %%ymm12\n\t"
		"vpsubd 992(%[from]), %%ymm3, %%ymm3\n\t"
		"vpmuludq %%ymm10, %%ymm12, %%ymm12\n\t"
		"vpmulld %%ymm5, %%ymm13, %%ymm5\n\t"
		"vpaddd 992(%[from]), %%ymm9, %%ymm9\n\t"
		"vmovdqa %%ymm11, 832(%[a])\n\t"
		"vpmuludq %%ymm10, %%ymm13, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm3, %%ymm13\n\t"
		"vbroadcastss 60(%[x]), %%ymm11\n\t"
		"vpmuludq %%ymm4, %%ymm13, %%ymm13\n\t"
		"vpmuludq %%ymm4, %%ymm3, %%ymm4\n\t"
		"vpsrlq $32, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm1, %%ymm3, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm7, %%ymm0\n\t"
		"vbroadcastss 28(%[x]), %%ymm3\n\t"
		"vpblendd $170, %%ymm12, %%ymm10, %%ymm12\n\t"
		"vpminud %%ymm0, %%ymm7, %%ymm0\n\t"
		"vmovdqa %%ymm8, 864(%[a])\n\t"
		"vpsubd %%ymm14, %%ymm9, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm12, %%ymm12\n\t"
		"vpminud %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpminud %%ymm7, %%ymm2, %%ymm7\n\t"
		"vpblendd $170, %%ymm13, %%ymm4, %%ymm13\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd %%ymm8, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm2\n\t"
		"vmovdqa %%ymm7, 768(%[a])\n\t"
		"vpmulld %%ymm15, %%ymm13, %%ymm13\n\t"
		"vpsubd %%ymm8, %%ymm4, %%ymm8\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpminud %%ymm2, %%ymm6, %%ymm2\n\t"
		"vpsubd %%ymm12, %%ymm5, %%ymm12\n\t"
		"vpsrlq $32, %%ymm8, %%ymm5\n\t"
		"vpmuludq %%ymm11, %%ymm8, %%ymm6\n\t"
		"vpminud %%ymm4, %%ymm0, %%ymm4\n\t"
		"vmovdqa %%ymm2, 800(%[a])\n\t"
		"vpaddd %%ymm14, %%ymm12, %%ymm2\n\t"
		"vpmuludq %%ymm11, %%ymm5, %%ymm5\n\t"
		"vmovdqa %%ymm4, 896(%[a])\n\t"
		"vpsubd %%ymm13, %%ymm1, %%ymm13\n\t"
		"vpsubd %%ymm13, %%ymm2, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm3, %%ymm8, %%ymm8\n\t"
		"vpaddd %%ymm13, %%ymm12, %%ymm13\n\t"
		"vpsrlq $32, %%ymm2, %%ymm12\n\t"
		"vpmuludq %%ymm11, %%ymm2, %%ymm1\n\t"
		"vpblendd $170, %%ymm5, %%ymm6, %%ymm5\n\t"
		"vpmulld %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpmuludq %%ymm11, %%ymm12, %%ymm11\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpsubd %%ymm14, %%ymm13, %%ymm12\n\t"
		"vpminud %%ymm12, %%ymm13, %%ymm12\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vmovdqa %%ymm12, 928(%[a])\n\t"
		"vpblendd $170, %%ymm11, %%ymm1, %%ymm11\n\t"
		"vpsubd %%ymm5, %%ymm8, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm11, %%ymm11\n\t"
		"vmovdqa %%ymm5, 960(%[a])\n\t"
		"vpsubd %%ymm11, %%ymm3, %%ymm11\n\t"
		"vmovdqa %%ymm11, 992(%[a])\n\t"
		"add $1024, %[a]\n\t"
		"add $1024, %[from]\n\t"
		"add $64, %[x]\n\t"
		"add $128, %[y]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y), [from] "+r"(from)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}

namespace blocked_poly {

inline constexpr std::uint32_t kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(std::size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr std::size_t footprint(std::size_t n) { return (n + 7) / 8 * 8 + 16; }
	std::span<std::uint32_t> take(std::size_t n) {
		if (std::size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

private:
	FpsWords storage_;
	std::uint32_t *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add;
using blocked_ntt::detail::broadcast;
using blocked_ntt::detail::diff;
using blocked_ntt::detail::Factor;
using blocked_ntt::detail::kP;
using blocked_ntt::detail::kR;
using blocked_ntt::detail::reduce;
using blocked_ntt::detail::slot;
using blocked_ntt::detail::Vec;

inline Vec load(const std::uint32_t* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(std::uint32_t* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const std::uint32_t* roots, std::size_t p) {
	const std::uint32_t* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const std::uint32_t* table, std::size_t g)
		: x(entry(table, g)), y(entry(table, 2 * g)), z(entry(table, 2 * g + 1)) {}
};

[[gnu::always_inline]] inline void forward_h1(Vec (&f)[4], const Group& w) {
	const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], w.x), d = times(f[3], w.x);
	const Vec ac = low(add(a, c)), amc = low_difference(a, c);
	const Vec bd = times(add(b, d), w.y), bmd = times(diff(b, d), w.z);
	f[0] = add(ac, bd), f[1] = diff(ac, bd), f[2] = add(amc, bmd), f[3] = diff(amc, bmd);
}

[[gnu::always_inline]] inline void inverse_h1(Vec (&f)[4], const Group& w) {
	const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
	const Vec amb = times(diff(f[0], f[1]), w.y), cmd = times(diff(f[2], f[3]), w.z);
	f[0] = low(add(ab, cd)), f[1] = low(add(amb, cmd));
	f[2] = times(diff(ab, cd), w.x), f[3] = times(diff(amb, cmd), w.x);
}

struct alignas(64) Window {
	std::uint32_t word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const std::uint32_t* roots,
												std::size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const std::uint32_t* b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2, t3;
	asm("vpbroadcastd (%[b]), %[bi]\n\t"
		"vpmuludq 36(%[w]), %[bi], %[odd]\n\t"
		"vpmuludq 32(%[w]), %[bi], %[even]\n\t"
		"vpbroadcastd 4(%[b]), %[bi]\n\t"
		"vpmuludq 32(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t1]\n\t"
		"vpbroadcastd 8(%[b]), %[bi]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 24(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 12(%[b]), %[bi]\n\t"
		"vpmuludq 24(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpbroadcastd 16(%[b]), %[bi]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 16(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 20(%[b]), %[bi]\n\t"
		"vpmuludq 16(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpbroadcastd 24(%[b]), %[bi]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 8(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 28(%[b]), %[bi]\n\t"
		"vpmuludq 8(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 4(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"

		"vpmuludq %[ni], %[even], %[t0]\n\t"
		"vpmuludq %[ni], %[odd], %[t1]\n\t"
		"vpmuludq %[p], %[t0], %[t0]\n\t"
		"vpmuludq %[p], %[t1], %[t1]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpsrlq $32, %[even], %[even]\n\t"
		"vpblendd $0xaa, %[odd], %[even], %[even]\n\t"
		"vpsubd %[p2], %[even], %[t0]\n\t"
		"vpminud %[t0], %[even], %[even]"
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2),
		[t3] "=&x"(t3)
		: [w] "r"(window.word), [b] "r"(b), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)),
		"m"(*reinterpret_cast<const std::uint32_t(*)[8]>(b)));
	return even;
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const std::uint32_t* roots;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		if (count % 8 == 0)
			return kernels::forward_bottom(reinterpret_cast<Vec*>(a), count, roots + slot(first), roots + slot(2 * first));
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			const Group w(roots, first + j);
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, w);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
};

struct InverseBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* in;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const std::uint32_t* from = in + 32 * first;
		if (count % 8 == 0)
			return kernels::inverse_bottom(reinterpret_cast<Vec*>(a), count, inverse_roots + slot(first),
										inverse_roots + slot(2 * first), reinterpret_cast<const Vec*>(from));
		for (std::size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const std::uint32_t* roots;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* b;

	[[gnu::always_inline]] void prepare(const std::uint32_t* a, std::size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(std::uint32_t* a, std::size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const std::uint32_t* a;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[4]) const {
		const std::uint32_t* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <std::size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(std::size_t g, Window (&window)[K][4]) const {
		for (std::size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t));
				for (std::size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const std::uint32_t* b = product.b + 32 * (first + j);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const std::uint32_t* roots, const std::uint32_t* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const std::size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr std::size_t kTile = 256;

	void tile(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if constexpr (Bottom::kForward) {
			for (std::size_t h = nv / 4; h >= 16; h /= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (std::size_t h = 16; h < nv; h *= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(std::uint32_t* a) { return reinterpret_cast<Vec*>(a); }

	void forward(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const std::uint32_t *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const std::uint32_t* in, std::size_t size, std::size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(std::size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool within(std::size_t lo, std::size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const std::uint32_t* in, std::size_t shift, std::size_t end, std::size_t v) {
		alignas(32) std::uint32_t x[8] = {};
		for (std::size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const std::uint32_t* in_;
	std::size_t shift_, end_;
	std::size_t first_, count_;
	std::size_t first_touched_, touched_;
};

[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, std::size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (std::size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

inline void inverse_top4(Vec* f, std::size_t h, Half output, const std::uint32_t* inverse_roots, const Factor& s) {
	if (output == Half::kBoth) return blocked_ntt::detail::inverse_radix4(f, h, inverse_roots, s);
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output == Half::kLower) {
			f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		} else {
			f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
		}
	}
}

inline void inverse_top2(Vec* f, std::size_t h, Half output, std::uint32_t scale) {
	if (output == Half::kBoth) {
		alignas(32) std::uint32_t s[16] = {};
		s[1] = scale;
		s[9] = blocked_ntt::detail::quotient(scale);
		return blocked_ntt::kernels::scale_radix2(f, h, s);
	}
	const Factor s(scale);
	for (std::size_t j = 0; j < h; ++j) {
		if (output == Half::kLower) f[j] = reduce(times(add(f[j], f[j + h]), s), kP);
		else f[j + h] = reduce(times(diff(f[j], f[j + h]), s), kP);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr std::size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const std::size_t table = blocked_ntt::detail::table_words(lg_max), entries = (std::size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const std::uint32_t* roots() const { return roots_; }
	const std::uint32_t* inverse_roots() const { return inverse_roots_; }

	void forward(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(std::span<std::uint32_t> a) const { forward(a, 0, a); }

	void forward_upper(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const std::size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (std::size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const std::size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(std::span<const std::uint32_t> in, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = power(std::uint32_t(out.size() / 8), kP - 2);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(std::span<std::uint32_t> a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b, Half output = Half::kBoth, std::uint32_t c = 1) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(multiply_mod(power(std::uint32_t(out.size() / 8), kP - 2), kR), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(std::span<const std::uint32_t> a, std::span<const std::uint32_t> b, std::span<std::uint32_t> out,
						Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(power(std::uint32_t(out.size() / 8), kP - 2), kR);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		std::span<const std::uint32_t> a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(power(std::uint32_t(out.size() / 8), kP - 2), kR);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(std::span<std::uint32_t> c, std::span<const std::uint32_t> a,
					std::span<const std::uint32_t> b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

private:
	void check_length(std::size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <std::size_t K>
	void inverse_products(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(power(std::uint32_t(out.size() / 8), kP - 2), kR);
		detail::InverseProductSumBottom<K> bottom{};
		for (std::size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	static detail::Source source(std::span<const std::uint32_t> in, std::size_t shift, std::size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(std::span<std::uint32_t> a, const detail::Source& in, const Bottom& bottom, std::uint32_t scale,
			Half output) const {
		using namespace detail;
		check_length(a.size());
		const detail::Recursion recursion(roots_, inverse_roots_, bottom);
		const std::size_t nv = a.size() / 8;
		auto* v = reinterpret_cast<Vec*>(a.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			if constexpr (Bottom::kForward) forward_top4(in, v, h, roots_);
			for (std::size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
			if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots_, Factor(scale));
		} else {
			const std::size_t h = nv / 2;
			if constexpr (Bottom::kForward) forward_top2(in, v, h);
			recursion.visit(a.data(), h, 0);
			recursion.visit(a.data() + 8 * h, h, 1);
			if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
		}
	}

	template <class Combine>
	void products(std::uint32_t* out, const std::uint32_t* a, const std::uint32_t* b, std::size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (std::size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	std::uint32_t *roots_, *inverse_roots_;
};

}

namespace blocked_poly {

namespace detail {

inline Vec load_unaligned(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(std::uint32_t* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec montgomery(Vec a, Vec b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b);
	Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

class Indices {
public:
	explicit Indices(std::size_t k) {
		alignas(32) std::uint32_t x[8];
		for (std::uint32_t i = 0; i < 8; ++i) x[i] = std::uint32_t((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr std::uint32_t kStep = std::uint32_t((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline std::uint32_t scalar_inverse(std::uint32_t x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(std::size_t first, std::span<std::uint32_t> out, const A& a, std::uint32_t factor = 1) {
	constexpr std::size_t kLanes = 32;
	const std::size_t n = out.size();
	if (n == 0) return;
	const std::size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(std::uint32_t(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](std::size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(std::uint32_t(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(std::uint32_t(n)), lanes);
	};
	const auto masked = [&](Vec value, std::size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (std::size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	alignas(32) std::uint32_t total[kLanes], inverse[kLanes];
	for (int v = 0; v < 4; ++v) store(total + 8 * v, canonical(prefix[v]));
	std::uint32_t running = 1;
	for (std::size_t l = 0; l < kLanes; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (std::size_t l = kLanes; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	Vec inv[4];
	for (int v = 0; v < 4; ++v) inv[v] = load(inverse + 8 * v);
	const auto put = [&](std::size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (std::size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const std::size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * std::size_t(v) < n) put(8 * v, inv[v]);
}

}

inline void derivative(std::span<const std::uint32_t> f, std::span<std::uint32_t> d) {
	using namespace detail;
	const std::size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(load_unaligned(f.data() + i + 1), index.value())));
	for (std::size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(std::uint32_t(i + 1), f[i + 1]) : 0;
}

inline void divide_by_index(std::span<const std::uint32_t> a, std::size_t first, std::span<std::uint32_t> q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	const std::size_t n = a.size();
	detail::divide_by_index(first, q, [a, n](std::size_t at) __attribute__((target("avx2"))) {
		if (at + 8 <= n) return detail::load_unaligned(a.data() + at);
		alignas(32) std::uint32_t rest[8] = {};
		std::copy(a.begin() + at, a.end(), rest);
		return detail::load(rest);
	});
}

}

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kInverseBase = 32;

inline void inverse_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	const std::uint32_t inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (std::size_t j = 1; j <= std::min(i, f.size() - 1); ++j) {
			sum += std::uint64_t(f[j]) * g[i - j] % kP;
		}
		g[i] = multiply_mod(std::uint32_t(sum % kP ? kP - sum % kP : 0), inv0);
	}
}

}

inline int inverse_log(std::size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)));
}

inline std::size_t inverse_scratch(std::size_t n) { return 2 * Arena::footprint(std::size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
						std::span<std::uint32_t> to, std::span<std::uint32_t> gt, std::span<std::uint32_t> work) {
	const std::size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, detail::kP - 1);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
					std::span<std::uint32_t> scratch) {
	const std::size_t n = g.size();
	std::size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const std::size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

}

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kLogBase = 64;

inline constexpr std::size_t kLogBlocks = 4;

inline void log_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	std::uint32_t a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (std::size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / std::uint32_t(i), inv[kP % i]);
	g[0] = 0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = multiply_mod(std::uint32_t(i), a[i]);
		for (std::size_t k = 1; k < i; ++k) sum += kP - std::uint64_t(kg[k]) * a[i - k] % kP;
		kg[i] = std::uint32_t(sum % kP);
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline std::size_t log_block(std::size_t n) {
	return std::max<std::size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline std::size_t log_blocks(std::size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

inline void subtract_from_derivative(std::span<const std::uint32_t> f, std::size_t first, std::size_t count,
									std::uint32_t* out, std::size_t size) {
	const std::size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (std::size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(load_unaligned(f.data() + first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (std::size_t c = full; c < count; ++c) {
		const std::uint32_t d = c < inside ? blocked_ntt::detail::multiply_mod(std::uint32_t(first + 1 + c), f[first + 1 + c]) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	std::uint32_t* w3;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (std::size_t t = 0; t < 4; ++t) {
			const std::size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const std::uint32_t* side;

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + leaf), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline std::size_t log_buffers(std::size_t n) { return std::max<std::size_t>(log_blocks(n) + 2, 3); }

template <class Sink>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, std::span<const std::uint32_t> f, std::size_t n,
												std::span<std::uint32_t> scratch, const Sink& sink) {
	const std::size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](std::size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const std::span<std::uint32_t> ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](std::size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, f, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (std::size_t s = 1; s < blocks; ++s) {
		const std::size_t from = std::min(f.size(), (s - 1) * k);
		t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (std::size_t j = 0; j < blocks; ++j) {
		const std::size_t first = j * k, count = std::min(k, n - 1 - first);
		const std::span<std::uint32_t> work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
		} else {
			if (j == 1) {
				t.inverse_product(window(1), q0, work, Half::kUpper);
			} else if (blocks == 3) {
				const Transform::Pair pairs[2] = {{window(2), q0}, {window(1), work}};
				t.inverse_product_sum(pairs, work, Half::kUpper);
			} else if (j == 2) {
				t.inverse_with(ToeplitzBottom{t.roots(), t.inverse_roots(), window(1).data(), window(2).data(), q0.data(),
											work.data(), window(3).data()},
							work, Half::kUpper);
			} else {
				const InverseProductBottom product{{t.roots(), t.inverse_roots(), work.data()}, window(1).data()};
				t.inverse_with(SideProductBottom{product, window(3).data()}, work, Half::kUpper);
			}
			subtract_from_derivative(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, std::span<const std::uint32_t>(work.first(count)));
		if (j + 1 < blocks) t.forward(work.first(k), 0, work);
	}
}

inline int log_derivative_log(std::size_t n) { return std::countr_zero(2 * log_block(n)); }

inline std::size_t log_derivative_scratch(std::size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}

inline int log_log(std::size_t n) {
	return n <= detail::kLogBase ? Transform::kMinLog : std::countr_zero(2 * detail::log_block(n));
}

inline std::size_t log_scratch(std::size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_buffers(n) * Arena::footprint(2 * detail::log_block(n));
}

inline void log(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](std::size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

}

namespace blocked_poly {
inline void native_log(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool integrate) {
	out.reserve(n);
	out.emplace_back(0);
	const std::size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(c) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(c), scratch = arena.take(log_scratch(n));
	for (std::size_t i = 0; i < c; ++i) {
		const unsigned v = in[i].v;
		f[i] = std::min(v, v - Mint::mod());
	}
	detail::log_derivative(t, f, n, scratch, [&out, integrate, n](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<std::uint32_t *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](std::size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
		}
		const auto *values = integrate ? q.data() + detail::log_block(n) : q.data();
		if (integrate)
			for (std::size_t i = 0; i < q.size(); ++i) out.emplace_back(Mint::word(values[i]));
		else
			for (std::size_t i = 0; i < q.size(); ++i) out.emplace_back(Mint::raw(values[i]));
	});
}
}
}
#pragma GCC pop_options
#endif
#endif
