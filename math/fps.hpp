#ifndef FPS_998244353_HPP
#define FPS_998244353_HPP
#include "../template.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

#include "mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct NTT {
	using T = Mint;
	typedef uint U __attribute__((may_alias));
	using V = vec<uint>;
	static constexpr U mod = 998244353, twice = 2 * mod, one = 301989884;
	V roots, inverse_roots, twists, inverse_twists;
	V root_quotients, inverse_root_quotients;
	mutable V cube_roots, inverse_cube_roots, cube_quotients, inverse_cube_quotients;
	void prepare_cubes(int n) const {
		int begin = cube_roots.size();
		if (begin >= n) return;
		cube_roots.resize(n);
		inverse_cube_roots.resize(n);
		cube_quotients.resize(n);
		inverse_cube_quotients.resize(n);
		rep(k, begin, n) {
			U x = (unsigned long long)roots[k] * roots[2 * k] % mod;
			U y = (unsigned long long)inverse_roots[k] * inverse_roots[2 * k] % mod;
			cube_roots[k] = x;
			inverse_cube_roots[k] = y;
			cube_quotients[k] = quotient(x);
			inverse_cube_quotients[k] = quotient(y);
		}
	}
	static U reduce(unsigned long long x) {
		return (x + (unsigned long long)(U(x) * 998244351u) * mod) >> 32;
	}
	static U mul(U a, U b) { return reduce((unsigned long long)a * b); }
	static U add(U a, U b) {
		U c = a + b;
		return c < twice ? c : c - twice;
	}
	static U sub(U a, U b) {
		U c = a - b;
		return c < twice ? c : c + twice;
	}
	static U in(U a) { return mul(a, 932051910); }
	static U out(U a) {
		a = reduce(a);
		return a < mod ? a : a - mod;
	}
	static U quotient(U r) { return ((unsigned long long)r << 32) / mod; }
	static U fixed_mul(U a, U r, U q) {
		return a * r - U(((unsigned long long)a * q) >> 32) * mod;
	}
	NTT(int size, bool composition = true)
		: roots(size, one),
		  inverse_roots(size, one),
		  twists(composition ? size : 0, one),
		  inverse_twists(composition ? size : 0, one),
		  root_quotients(size),
		  inverse_root_quotients(size) {
		for (int h = 1; h < size; h <<= 1) {
			U r = in(T(3).pow((mod - 1) / (4 * h)).val());
			U ir = in(T(3).pow(mod - 1 - (mod - 1) / (4 * h)).val());
			U t = in(T(3).pow((mod - 1) / (2 * h)).val());
			U it = in(T(3).pow(mod - 1 - (mod - 1) / (2 * h)).val());
			U a = in(mod - (mod - 1) / h), b = a;
			rep(i, h) {
				roots[h + i] = mul(roots[i], r);
				inverse_roots[h + i] = mul(inverse_roots[i], ir);
				if (composition) twists[h + i] = a, inverse_twists[h + i] = b;
				a = mul(a, t);
				b = mul(b, it);
			}
		}
		rep(i, size) {
			root_quotients[i] = (roots[i] < mod ? roots[i] : roots[i] - mod) * 998244351u;
			inverse_root_quotients[i] = (inverse_roots[i] < mod ? inverse_roots[i] : inverse_roots[i] - mod) * 998244351u;
			roots[i] = out(roots[i]);
			inverse_roots[i] = out(inverse_roots[i]);
			if (composition) twists[i] = out(twists[i]), inverse_twists[i] = out(inverse_twists[i]);
		}
	}

#if defined(__ARM_NEON)
	using I = uint32x4_t;
	static I mul4(I a, I b) {
		uint64x2_t x = vmull_u32(vget_low_u32(a), vget_low_u32(b));
		uint64x2_t y = vmull_u32(vget_high_u32(a), vget_high_u32(b));
		auto reduce2 = [](uint64x2_t z) {
			uint32x2_t q = vmul_u32(vmovn_u64(z), vdup_n_u32(998244351));
			return vshrn_n_u64(vaddq_u64(z, vmull_u32(q, vdup_n_u32(mod))), 32);
		};
		return vcombine_u32(reduce2(x), reduce2(y));
	}
	static I add4(I a, I b) {
		I c = vaddq_u32(a, b);
		return vminq_u32(c, vsubq_u32(c, vdupq_n_u32(twice)));
	}
	static I sub4(I a, I b) {
		I c = vsubq_u32(a, b);
		return vminq_u32(c, vaddq_u32(c, vdupq_n_u32(twice)));
	}
	static I fixed_mul4(I a, I r, I q) {
		uint64x2_t x = vmull_u32(vget_low_u32(a), vget_low_u32(q));
		uint64x2_t y = vmull_u32(vget_high_u32(a), vget_high_u32(q));
		I t = vcombine_u32(vshrn_n_u64(x, 32), vshrn_n_u64(y, 32));
		return vmlsq_u32(vmulq_u32(a, r), t, vdupq_n_u32(mod));
	}
	template <bool inverse>
	static void fixed_butterfly4(I &x, I &y, I r, I q, bool unit = false) {
		if constexpr (!inverse)
			if (!unit) y = fixed_mul4(y, r, q);
		I z = sub4(x, y);
		x = add4(x, y);
		y = inverse && !unit ? fixed_mul4(z, r, q) : z;
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	static bool vectorized() {
#if defined(__AVX2__)
		return true;
#else
		static const bool available = __builtin_cpu_supports("avx2");
		return available;
#endif
	}
	using I8 = __m256i;
	__attribute__((target("avx2"))) static I8 load8(const U *a) {
		return _mm256_loadu_si256(reinterpret_cast<const I8 *>(a));
	}
	__attribute__((target("avx2"))) static void store8(U *a, I8 x) {
		_mm256_storeu_si256(reinterpret_cast<I8 *>(a), x);
	}
	__attribute__((target("avx2"))) static I8 mul8(I8 a, I8 b) {
		I8 x = _mm256_mul_epu32(a, b);
		I8 y = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
		I8 r = _mm256_set1_epi32(998244351), p = _mm256_set1_epi32(mod);
		x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, r), p));
		y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, r), p));
		return _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
	}
	__attribute__((target("avx2"))) static I8 fixed_mul8(I8 a, I8 r, I8 q) {
		I8 x = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		I8 y = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		I8 t = _mm256_blend_epi32(x, y, 0xaa);
		return _mm256_sub_epi32(_mm256_mullo_epi32(a, r),
								_mm256_mullo_epi32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 fixed_even8(I8 a, I8 r, I8 q) {
		I8 t = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		return _mm256_sub_epi64(_mm256_mul_epu32(a, r),
								_mm256_mul_epu32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 add8(I8 a, I8 b) {
		I8 x = _mm256_add_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(twice)));
	}
	__attribute__((target("avx2"))) static I8 sub8(I8 a, I8 b) {
		I8 x = _mm256_sub_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_add_epi32(x, _mm256_set1_epi32(twice)));
	}

	template <int stride>
	__attribute__((target("avx2"))) static I8 rates8(const V &values, int k) {
		if constexpr (stride == 1)
			return _mm256_cvtepu32_epi64(
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(values.data() + 4 * k)));
		if constexpr (stride == 2)
			return _mm256_permute4x64_epi64(
				_mm256_cvtepu32_epi64(
					_mm_loadl_epi64(reinterpret_cast<const __m128i *>(values.data() + 2 * k))),
				0x50);
		if constexpr (stride == 4) return _mm256_set1_epi32(values[k]);
	}
	template <int stride, bool inverse>
	__attribute__((target("avx2"))) I8 small8(I8 z, int k) const {
		if constexpr (stride == 4) {
			if (k == 0) {
				I8 x = _mm256_permute2x128_si256(z, z, 0x00),
				   y = _mm256_permute2x128_si256(z, z, 0x11);
				return _mm256_blend_epi32(add8(x, y), sub8(x, y), 0xf0);
			}
		}
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		I8 r = rates8<stride>(w, k), q = rates8<stride>(quot, k);
		if constexpr (inverse) {
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
			I8 y = _mm256_srli_epi64(z, 32), x = add8(z, y);
			I8 d = fixed_even8(sub8(z, y), r, q);
			z = _mm256_blend_epi32(x, _mm256_slli_epi64(d, 32), 0xaa);
		} else {
			if constexpr (stride == 1) z = _mm256_shuffle_epi32(z, 0xb1);
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0x72);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(4, 0, 5, 1, 6, 2, 7, 3));
			I8 y = fixed_even8(z, r, q);
			I8 x = _mm256_blend_epi32(_mm256_srli_epi64(z, 32), z, 0xaa);
			y = _mm256_blend_epi32(
				y, _mm256_sub_epi32(_mm256_set1_epi32(twice), _mm256_slli_epi64(y, 32)), 0xaa);
			z = add8(x, y);
		}
		if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
		if constexpr (stride == 4)
			z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
		return z;
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void fft8(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
		if constexpr (!inverse) {
			int h = limit / 2;
			for (; h >= 16; h >>= 2) radix4_8<false>(a, h / 2, length);
			if (h == 8) {
				if (limit >= 32)
					for (int i = 0; i < length; i += 8) {
						I8 x = load8(a + i), p = _mm256_set1_epi32(twice);
						store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
					}
				stage8<false>(a, h, length);
			}
		}
		for (int j = 0; j < length; j += 8) {
			I8 z = load8(a + j);
			if constexpr (!inverse)
				if ((limit >= 32) && (__builtin_ctz((uint)limit) & 1)) {
					I8 p = _mm256_set1_epi32(twice);
					z = _mm256_min_epu32(z, _mm256_sub_epi32(z, p));
				}
			if constexpr (inverse)
				z = small8<4, true>(small8<2, true>(small8<1, true>(z, j / 8), j / 8), j / 8);
			else
				z = small8<1, false>(small8<2, false>(small8<4, false>(z, j / 8), j / 8),
									 j / 8);
			store8(a + j, z);
		}
		if constexpr (inverse) {
			int h = 8;
			for (; 4 * h <= limit; h <<= 2) radix4_8<true>(a, h, length);
			if (h < limit) stage8<true>(a, h, length);
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void radix4_8(U *a, int stride, int length) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		prepare_cubes(length / (4 * stride));
		const V &w3 = inverse ? inverse_cube_roots : cube_roots;
		const V &q3s = inverse ? inverse_cube_quotients : cube_quotients;
		I8 p = _mm256_set1_epi32(twice);
		I8 im = _mm256_set1_epi32(w[1]), iq = _mm256_set1_epi32(q[1]);
		for (int j = 0, k = 0; j < length; j += 4 * stride, ++k) {
			I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			I8 r3 = _mm256_set1_epi32(w3[k]), q3 = _mm256_set1_epi32(q3s[k]);
			for (int i = 0; i < stride; i += 8) {
				I8 x0 = load8(a + j + i), x1 = load8(a + j + stride + i);
				I8 x2 = load8(a + j + 2 * stride + i), x3 = load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					I8 u = add8(x0, x1), v = sub8(x0, x1);
					I8 s = add8(x2, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x2, p), x3), im, iq);
					x0 = add8(u, s);
					x1 = _mm256_add_epi32(v, t);
					x2 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					} else {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
				} else {
					x0 = _mm256_min_epu32(x0, _mm256_sub_epi32(x0, p));
					if (!k) {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					}
					I8 u = add8(x0, x2), v = sub8(x0, x2), s = add8(x1, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x1, p), x3), im, iq);
					x0 = _mm256_add_epi32(u, s);
					x1 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x2 = _mm256_add_epi32(v, t);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
				}
				store8(a + j + i, x0);
				store8(a + j + stride + i, x1);
				store8(a + j + 2 * stride + i, x2);
				store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void stage8(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		if (stride < 8) {
			for (int j = 0; j < length; j += 8) {
				I8 z = load8(a + j);
				if (stride == 1)
					z = small8<1, inverse>(z, j / 8);
				else if (stride == 2)
					z = small8<2, inverse>(z, j / 8);
				else
					z = small8<4, inverse>(z, j / 8);
				store8(a + j, z);
			}
		} else {
			for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
				I8 r = _mm256_set1_epi32(w[k]), q = _mm256_set1_epi32(quot[k]);
				for (int i = 0; i < stride; i += 8) {
					I8 x = load8(a + j + i), y = load8(a + j + stride + i);
					if constexpr (!inverse)
						if (k) y = fixed_mul8(y, r, q);
					I8 z = sub8(x, y);
					x = add8(x, y);
					if constexpr (inverse)
						if (k) z = fixed_mul8(z, r, q);
					store8(a + j + i, x);
					store8(a + j + stride + i, z);
				}
			}
		}
	}

	template <bool inverse, bool lazy = false>
	__attribute__((target("avx2"), always_inline)) static void fixed_butterfly8(I8 &x, I8 &y, I8 r, I8 q, bool unit = false) {
		if constexpr (lazy && !inverse) {
			I8 p = _mm256_set1_epi32(twice);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = unit ? _mm256_min_epu32(y, _mm256_sub_epi32(y, p)) : fixed_mul8(y, r, q);
			I8 z = _mm256_sub_epi32(_mm256_add_epi32(x, p), y);
			x = _mm256_add_epi32(x, y);
			y = z;
		} else {
			if constexpr (!inverse)
				if (!unit) y = fixed_mul8(y, r, q);
			I8 z = sub8(x, y);
			x = add8(x, y);
			y = inverse && !unit ? fixed_mul8(z, r, q) : z;
		}
	}
	template <bool inverse, bool lazy = false>
	__attribute__((target("avx2"))) void stage_pair8(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		for (int j = 0, k = first; j < length; j += 4 * stride, ++k) {
			I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			I8 r1 = _mm256_set1_epi32(w[2 * k + 1]), q1 = _mm256_set1_epi32(q[2 * k + 1]);
			for (int i = 0; i < stride; i += 8) {
				I8 x0 = load8(a + j + i), x1 = load8(a + j + stride + i);
				I8 x2 = load8(a + j + 2 * stride + i), x3 = load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					fixed_butterfly8<inverse, lazy>(x0, x1, r0, q0, k == 0);
					fixed_butterfly8<inverse, lazy>(x2, x3, r1, q1, false);
					fixed_butterfly8<inverse, lazy>(x0, x2, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x1, x3, r, rq, k == 0);
				} else {
					fixed_butterfly8<inverse, lazy>(x0, x2, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x1, x3, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x0, x1, r0, q0, k == 0);
					fixed_butterfly8<inverse, lazy>(x2, x3, r1, q1, false);
				}
				store8(a + j + i, x0);
				store8(a + j + stride + i, x1);
				store8(a + j + 2 * stride + i, x2);
				store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	__attribute__((target("avx2"))) static void fold8(U *a, int columns) {
		for (int i = 0; i < columns; i += 8) {
			I8 x = add8(load8(a + i), load8(a + columns + i));
			store8(a + i, x);
			store8(a + columns + i, x);
		}
	}
	__attribute__((target("avx2"))) void project8_8(U *a, int rows) const {
		rep(j, rows) {
			I8 x0 = load8(a + 16 * j + 0);
			x0 = small8<4, true>(small8<2, true>(small8<1, true>(x0, 0), 0), 0);
			I8 x1 = load8(a + 16 * j + 8);
			x1 = small8<4, true>(small8<2, true>(small8<1, true>(x1, 1), 1), 1);
			x0 = add8(x0, x1);
			x1 = x0;
			x0 = small8<1, false>(small8<2, false>(small8<4, false>(x0, 0), 0), 0);
			store8(a + 16 * j + 0, x0);
			x1 = small8<1, false>(small8<2, false>(small8<4, false>(x1, 1), 1), 1);
			store8(a + 16 * j + 8, x1);
		}
	}
	__attribute__((target("avx2"))) void project16_8(U *a, int rows) const {
		rep(j, rows) {
			I8 x0 = load8(a + 32 * j + 0);
			x0 = small8<4, true>(small8<2, true>(small8<1, true>(x0, 0), 0), 0);
			I8 x1 = load8(a + 32 * j + 8);
			x1 = small8<4, true>(small8<2, true>(small8<1, true>(x1, 1), 1), 1);
			I8 x2 = load8(a + 32 * j + 16);
			x2 = small8<4, true>(small8<2, true>(small8<1, true>(x2, 2), 2), 2);
			I8 x3 = load8(a + 32 * j + 24);
			x3 = small8<4, true>(small8<2, true>(small8<1, true>(x3, 3), 3), 3);
			fixed_butterfly8<true>(x0, x1, _mm256_set1_epi32(inverse_roots[0]), _mm256_set1_epi32(inverse_root_quotients[0]), true);
			fixed_butterfly8<true>(x2, x3, _mm256_set1_epi32(inverse_roots[1]), _mm256_set1_epi32(inverse_root_quotients[1]), false);
			x0 = add8(x0, x2);
			x2 = x0;
			x1 = add8(x1, x3);
			x3 = x1;
			fixed_butterfly8<false>(x0, x1, _mm256_set1_epi32(roots[0]), _mm256_set1_epi32(root_quotients[0]), true);
			fixed_butterfly8<false>(x2, x3, _mm256_set1_epi32(roots[1]), _mm256_set1_epi32(root_quotients[1]), false);
			x0 = small8<1, false>(small8<2, false>(small8<4, false>(x0, 0), 0), 0);
			store8(a + 32 * j + 0, x0);
			x1 = small8<1, false>(small8<2, false>(small8<4, false>(x1, 1), 1), 1);
			store8(a + 32 * j + 8, x1);
			x2 = small8<1, false>(small8<2, false>(small8<4, false>(x2, 2), 2), 2);
			store8(a + 32 * j + 16, x2);
			x3 = small8<1, false>(small8<2, false>(small8<4, false>(x3, 3), 3), 3);
			store8(a + 32 * j + 24, x3);
		}
	}
	__attribute__((target("avx2"))) static void scale_rows8(U *a, int rows, int columns, const U *factors) {
		if (columns == 2) {
			int j = 0;
			for (; j + 1 < rows; j += 2) {
				U w0 = factors[j], w1 = factors[j + 1], q0 = quotient(w0), q1 = quotient(w1);
				I8 w = _mm256_setr_epi32(w0, w0, w0, w0, w1, w1, w1, w1);
				I8 q = _mm256_setr_epi32(q0, q0, q0, q0, q1, q1, q1, q1);
				store8(a + 4 * j, fixed_mul8(load8(a + 4 * j), w, q));
			}
			if (j < rows)
				rep(i, 4) a[4 * j + i] = fixed_mul(a[4 * j + i], factors[j], quotient(factors[j]));
		} else {
			rep(j, rows) {
				U w = factors[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				for (int i = 0; i < 2 * columns; i += 8) store8(a + 2 * columns * j + i, fixed_mul8(load8(a + 2 * columns * j + i), v, q));
			}
		}
	}
	__attribute__((target("avx2"))) void project4_8(U *a, int rows) const {
		rep(j, rows) {
			I8 x = small8<2, true>(small8<1, true>(load8(a + 8 * j), 0), 0);
			x = add8(x, _mm256_permute2x128_si256(x, x, 1));
			x = small8<1, false>(small8<2, false>(x, 0), 0);
			store8(a + 8 * j, x);
		}
	}
	__attribute__((target("avx2"))) static void load_pairs(const U *a, I8 &x, I8 &y) {
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		I8 u = _mm256_permutevar8x32_epi32(load8(a), index),
		   v = _mm256_permutevar8x32_epi32(load8(a + 8), index);
		x = _mm256_permute2x128_si256(u, v, 0x20);
		y = _mm256_permute2x128_si256(u, v, 0x31);
	}
	__attribute__((target("avx2"))) static void store_pairs(U *a, I8 x, I8 y) {
		I8 index = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
		store8(a, _mm256_permutevar8x32_epi32(_mm256_permute2x128_si256(x, y, 0x20), index));
		store8(a + 8,
			   _mm256_permutevar8x32_epi32(_mm256_permute2x128_si256(x, y, 0x31), index));
	}
	__attribute__((target("avx2"))) static void scale_copy8(U *dst, const U *src, int length, U r) {
		U quotient_r = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient_r);
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(dst + i, fixed_mul8(load8(src + i), v, q));
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, quotient_r);
	}
	__attribute__((target("avx2"))) static void scale_normalize8(U *a, int n, U r) {
		U qr = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(qr), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = fixed_mul8(load8(a + i), v, q);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (; i < n; ++i) {
			U x = fixed_mul(a[i], r, qr);
			a[i] = x < mod ? x : x - mod;
		}
	}
	__attribute__((target("avx2"))) static void square_fold8(U *a, const U *q, int n, bool negative) {
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(a + i), y = load8(a + n + i);
			I8 u = mul8(load8(q + i), load8(q + i));
			I8 v = mul8(load8(q + n + i), load8(q + n + i));
			x = add8(x, x);
			store8(a + i, negative ? sub8(x, u) : add8(x, u));
			store8(a + n + i, add8(add8(y, y), v));
		}
	}
	__attribute__((target("avx2"))) static void scale8(U *a, int length, U r) {
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient(r));
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(a + i, fixed_mul8(load8(a + i), v, q));
		for (; i < length; ++i) a[i] = fixed_mul(a[i], r, quotient(r));
	}
	__attribute__((target("avx2"))) static void pair_product8(U *a, const U *b, int length) {
		I8 ni = _mm256_set1_epi32(998244351), modulus = _mm256_set1_epi32(mod), p = _mm256_set1_epi32(twice);
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		for (int i = 0; i < length; i += 8) {
			I8 x = load8(b + 2 * i), y = load8(b + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			x = _mm256_mul_epu32(x, _mm256_srli_epi64(x, 32));
			y = _mm256_mul_epu32(y, _mm256_srli_epi64(y, 32));
			x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, ni), modulus));
			y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, ni), modulus));
			x = _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
			store8(a + i, _mm256_permutevar8x32_epi32(x, index));
		}
	}
	__attribute__((target("avx2"))) static void pair_multiply8(U *a, const U *b, int length) {
		I8 index0 = _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3), index1 = _mm256_setr_epi32(4, 4, 5, 5, 6, 6, 7, 7);
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < length; i += 8) {
			I8 r = load8(b + i), x = load8(a + 2 * i), y = load8(a + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			x = _mm256_shuffle_epi32(x, 0xb1);
			y = _mm256_shuffle_epi32(y, 0xb1);
			store8(a + 2 * i, mul8(x, _mm256_permutevar8x32_epi32(r, index0)));
			store8(a + 2 * i + 8, mul8(y, _mm256_permutevar8x32_epi32(r, index1)));
		}
	}
	__attribute__((target("avx2"))) static void difference8(U *a, const U *b, const U *c,
															int length) {
		for (int i = 0; i < length; i += 8) store8(a + i, sub8(load8(b + i), load8(c + i)));
	}
	__attribute__((target("avx2"))) static void project1_8(U *a, int rows) {
		for (int j = 0; j < rows; j += 8) {
			I8 x, y;
			load_pairs(a + 2 * j, x, y);
			x = add8(x, y);
			store_pairs(a + 2 * j, x, x);
		}
	}
	__attribute__((target("avx2"))) void project2_8(U *a, int rows) const {
		const I8 index = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
		for (int j = 0; j < rows; j += 8) {
			U *p = a + 4 * j;
			I8 u = load8(p), v = load8(p + 8), w = load8(p + 16), z = load8(p + 24);
			I8 uv0 = _mm256_unpacklo_epi32(u, v), uv1 = _mm256_unpackhi_epi32(u, v);
			I8 wz0 = _mm256_unpacklo_epi32(w, z), wz1 = _mm256_unpackhi_epi32(w, z);
			I8 a0 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(uv0, wz0), index);
			I8 a1 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(uv0, wz0), index);
			I8 a2 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(uv1, wz1), index);
			I8 a3 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(uv1, wz1), index);
			I8 x = add8(add8(a0, a1), add8(a2, a3));
			I8 y =
				add8(sub8(a0, a1), fixed_mul8(sub8(a2, a3), _mm256_set1_epi32(inverse_roots[1]),
											  _mm256_set1_epi32(inverse_root_quotients[1])));
			a0 = add8(x, y);
			a1 = sub8(x, y);
			y = fixed_mul8(y, _mm256_set1_epi32(roots[1]),
						   _mm256_set1_epi32(root_quotients[1]));
			a2 = add8(x, y);
			a3 = sub8(x, y);
			uv0 = _mm256_unpacklo_epi32(a0, a1);
			uv1 = _mm256_unpackhi_epi32(a0, a1);
			wz0 = _mm256_unpacklo_epi32(a2, a3);
			wz1 = _mm256_unpackhi_epi32(a2, a3);
			u = _mm256_unpacklo_epi64(uv0, wz0);
			v = _mm256_unpackhi_epi64(uv0, wz0);
			w = _mm256_unpacklo_epi64(uv1, wz1);
			z = _mm256_unpackhi_epi64(uv1, wz1);
			store8(p, _mm256_permute2x128_si256(u, v, 0x20));
			store8(p + 8, _mm256_permute2x128_si256(w, z, 0x20));
			store8(p + 16, _mm256_permute2x128_si256(u, v, 0x31));
			store8(p + 24, _mm256_permute2x128_si256(w, z, 0x31));
		}
	}
#endif

#if defined(__ARM_NEON)
	template <int stride, bool inverse>
	void small4(I &a, I &b, int k) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		if constexpr (stride == 4) {
			if (k == 0) {
				I x = sub4(a, b);
				a = add4(a, b);
				b = x;
			} else
				fixed_butterfly4<inverse>(a, b, vdupq_n_u32(w[k]), vdupq_n_u32(quot[k]));
		}
		if constexpr (stride == 2) {
			I x = vcombine_u32(vget_low_u32(a), vget_low_u32(b));
			I y = vcombine_u32(vget_high_u32(a), vget_high_u32(b));
			fixed_butterfly4<inverse>(
				x, y, vcombine_u32(vdup_n_u32(w[2 * k]), vdup_n_u32(w[2 * k + 1])),
				vcombine_u32(vdup_n_u32(quot[2 * k]), vdup_n_u32(quot[2 * k + 1])));
			a = vcombine_u32(vget_low_u32(x), vget_low_u32(y));
			b = vcombine_u32(vget_high_u32(x), vget_high_u32(y));
		}
		if constexpr (stride == 1) {
			uint32x4x2_t z = vuzpq_u32(a, b);
			fixed_butterfly4<inverse>(z.val[0], z.val[1], vld1q_u32(w.data() + 4 * k),
									  vld1q_u32(quot.data() + 4 * k));
			z = vzipq_u32(z.val[0], z.val[1]);
			a = z.val[0];
			b = z.val[1];
		}
	}
	template <bool inverse>
	void fft4(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
		if constexpr (!inverse) {
			int h = limit / 2;
			for (; h >= 16; h >>= 2) stage_pair<false>(a, h / 2, length);
			if (h == 8) stage<false>(a, h, length);
		}
		for (int j = 0; j < length; j += 8) {
			I x = vld1q_u32(a + j), y = vld1q_u32(a + j + 4);
			if constexpr (inverse) {
				small4<1, true>(x, y, j / 8);
				small4<2, true>(x, y, j / 8);
				small4<4, true>(x, y, j / 8);
			} else {
				small4<4, false>(x, y, j / 8);
				small4<2, false>(x, y, j / 8);
				small4<1, false>(x, y, j / 8);
			}
			vst1q_u32(a + j, x);
			vst1q_u32(a + j + 4, y);
		}
		if constexpr (inverse) {
			int h = 8;
			for (; 4 * h <= limit; h <<= 2) stage_pair<true>(a, h, length);
			if (h < limit) stage<true>(a, h, length);
		}
	}
#endif
	template <bool inverse>
	void stage(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			stage8<inverse>(a, stride, length, first);
			return;
		}
#endif
		for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
			U r = w[k], q = quot[k];
			int i = 0;
#if defined(__ARM_NEON)
			for (; i + 4 <= stride; i += 4) {
				I x = vld1q_u32(a + j + i), y = vld1q_u32(a + j + stride + i);
				if constexpr (!inverse)
					if (k) y = fixed_mul4(y, vdupq_n_u32(r), vdupq_n_u32(q));
				I z = sub4(x, y);
				x = add4(x, y);
				if constexpr (inverse)
					if (k) z = fixed_mul4(z, vdupq_n_u32(r), vdupq_n_u32(q));
				y = z;
				vst1q_u32(a + j + i, x);
				vst1q_u32(a + j + stride + i, y);
			}
#endif
			for (; i < stride; ++i) {
				U x = a[j + i], y = a[j + stride + i];
				if constexpr (!inverse)
					if (k) y = fixed_mul(y, r, q);
				a[j + i] = add(x, y);
				a[j + stride + i] = inverse && k ? fixed_mul(sub(x, y), r, q) : sub(x, y);
			}
		}
	}

	template <bool inverse>
	void stage_pair(U *a, int stride, int length, int first = 0) const {
#if defined(__x86_64__) || defined(__i386__)
		if (stride >= 8 && vectorized()) {
			stage_pair8<inverse>(a, stride, length, first);
			return;
		}
#endif
#if defined(__ARM_NEON)
		if (stride >= 4) {
			const V &w = inverse ? inverse_roots : roots;
			const V &q = inverse ? inverse_root_quotients : root_quotients;
			for (int j = 0, k = first; j < length; j += 4 * stride, ++k) {
				I r = vdupq_n_u32(w[k]), rq = vdupq_n_u32(q[k]);
				I r0 = vdupq_n_u32(w[2 * k]), q0 = vdupq_n_u32(q[2 * k]);
				I r1 = vdupq_n_u32(w[2 * k + 1]), q1 = vdupq_n_u32(q[2 * k + 1]);
				for (int i = 0; i < stride; i += 4) {
					I x0 = vld1q_u32(a + j + i), x1 = vld1q_u32(a + j + stride + i);
					I x2 = vld1q_u32(a + j + 2 * stride + i),
					  x3 = vld1q_u32(a + j + 3 * stride + i);
					if constexpr (inverse) {
						fixed_butterfly4<true>(x0, x1, r0, q0, k == 0);
						fixed_butterfly4<true>(x2, x3, r1, q1);
						fixed_butterfly4<true>(x0, x2, r, rq, k == 0);
						fixed_butterfly4<true>(x1, x3, r, rq, k == 0);
					} else {
						fixed_butterfly4<false>(x0, x2, r, rq, k == 0);
						fixed_butterfly4<false>(x1, x3, r, rq, k == 0);
						fixed_butterfly4<false>(x0, x1, r0, q0, k == 0);
						fixed_butterfly4<false>(x2, x3, r1, q1);
					}
					vst1q_u32(a + j + i, x0);
					vst1q_u32(a + j + stride + i, x1);
					vst1q_u32(a + j + 2 * stride + i, x2);
					vst1q_u32(a + j + 3 * stride + i, x3);
				}
			}
			return;
		}
#endif
		if constexpr (inverse)
			stage<true>(a, stride, length, 2 * first), stage<true>(a, 2 * stride, length, first);
		else
			stage<false>(a, 2 * stride, length, first), stage<false>(a, stride, length, 2 * first);
	}
	template <bool inverse>
	void fft(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
#if defined(__ARM_NEON)
		if (limit >= 8) {
			fft4<inverse>(a, length, limit);
			return;
		}
#elif defined(__x86_64__) || defined(__i386__)
		if (limit >= 8 && vectorized()) {
			fft8<inverse>(a, length, limit);
			return;
		}
#endif
		if constexpr (inverse) {
			for (int h = 1; h < limit; h <<= 1) stage<true>(a, h, length);
		} else {
			for (int h = limit / 2; h; h >>= 1) stage<false>(a, h, length);
		}
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void product8(U *dst, const U *a, const U *b, int n) {
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = mul8(load8(a + i), load8(b + i));
			store8(dst + i, x);
		}
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
	__attribute__((target("avx2"))) static void weighted8(U *dst, const U *src, int n, int first) {
		alignas(32) U factors[8];
		rep(j, 8) factors[j] = in(first + j);
		I8 r = load8(factors), step = _mm256_set1_epi32(in(8));
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(dst + i, mul8(load8(src + i), r));
			r = add8(r, step);
		}
		for (; i < n; ++i) dst[i] = mul(src[i], in(first + i));
	}
	__attribute__((target("avx2"), always_inline)) static I8 inverse8(I8 a) {
		I8 a2 = mul8(a, a), a3 = mul8(a2, a);
		I8 a7 = mul8(mul8(a3, a3), a), a31 = a7;
		rep(2) a31 = mul8(a31, a31);
		a31 = mul8(a31, a3);
		I8 r = a7;
		rep(3) r = mul8(r, r);
		r = mul8(r, a3);
		r = mul8(r, r);
		rep(j, 4) {
			rep(5) r = mul8(r, r);
			r = mul8(r, a31);
		}
		rep(3) r = mul8(r, r);
		return mul8(r, a7);
	}
	__attribute__((target("avx2"))) static void batch_inverse8(U *dst, const U *src, int n) {
		if (n < 16) {
			rep(i, n) dst[i] = T::word(src[i]).inv().v;
			return;
		}
		int count = n / 16 * 16;
		I8 p = load8(src), q = load8(src + 8);
		store8(dst, p);
		store8(dst + 8, q);
		for (int i = 16; i < count; i += 16) {
			p = mul8(p, load8(src + i));
			q = mul8(q, load8(src + i + 8));
			store8(dst + i, p);
			store8(dst + i + 8, q);
		}
		alignas(32) U lanes[16];
		store8(lanes, p);
		store8(lanes + 8, q);
		for (auto x : lanes) assert(out(x) != 0);
		I8 inverse = inverse8(p), other = inverse8(q);
		for (int i = count - 16; i >= 16; i -= 16) {
			store8(dst + i, mul8(inverse, load8(dst + i - 16)));
			store8(dst + i + 8, mul8(other, load8(dst + i - 8)));
			inverse = mul8(inverse, load8(src + i));
			other = mul8(other, load8(src + i + 8));
		}
		store8(dst, inverse);
		store8(dst + 8, other);
		rep(i, count, n) dst[i] = T::word(src[i]).inv().v;
	}
	__attribute__((target("avx2"))) static void evaluate_leaf8(U *dst, const U *cur, const U *tree, int degree, const U *points, int count) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 x = load8(cur);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		alignas(32) U window[16] = {}, coefficients[8] = {}, polynomial[8], tail[8] = {};
		store8(window + 8, x);
		copy_n(tree, min(degree, 8), coefficients);
		I8 y = load8(coefficients);
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
		store8(coefficients, _mm256_min_epu32(y, _mm256_sub_epi32(y, p)));
		I8 e = _mm256_setzero_si256(), o = e;
		rep(i, 8) {
			I8 z = load8(window + 8 - i), r = _mm256_set1_epi32(coefficients[i]);
			e = _mm256_add_epi64(e, _mm256_mul_epu32(z, r));
			o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
		}
		I8 ni = _mm256_set1_epi32(998244351);
		e = _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, ni), p));
		o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, ni), p));
		x = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		store8(polynomial, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		if (count == 8) x = load8(points);
		else {
			copy_n(points, count, tail);
			x = load8(tail);
		}
		I8 value = _mm256_setzero_si256();
		rep(i, 8) value = add8(mul8(value, x), _mm256_set1_epi32(polynomial[i]));
		if (count == 8) store8(dst, value);
		else {
			store8(tail, value);
			copy_n(tail, count, dst);
		}
	}
	__attribute__((target("avx2"), always_inline)) static U sum_product8(I8 a, I8 b) {
		I8 p = _mm256_set1_epi32(mod);
		a = _mm256_min_epu32(a, _mm256_sub_epi32(a, p));
		b = _mm256_min_epu32(b, _mm256_sub_epi32(b, p));
		I8 sum = _mm256_add_epi64(_mm256_mul_epu32(a, b), _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
		sum = _mm256_add_epi64(sum, _mm256_permute2x128_si256(sum, sum, 1));
		sum = _mm256_add_epi64(sum, _mm256_shuffle_epi32(sum, 0x4e));
		U result = reduce((unsigned long long)_mm_cvtsi128_si64(_mm256_castsi256_si128(sum)));
		return result < twice ? result : result - twice;
	}
	__attribute__((target("avx2"))) static void interpolate_leaf8(U *dst, const U *q, const U *points, const U *weights) {
		I8 x = load8(points), w = load8(weights), c = _mm256_set1_epi32(one);
		I8 sum = add8(w, _mm256_permute2x128_si256(w, w, 1));
		sum = add8(sum, _mm256_shuffle_epi32(sum, 0x4e));
		sum = add8(sum, _mm256_shuffle_epi32(sum, 0xb1));
		dst[7] = _mm_cvtsi128_si32(_mm256_castsi256_si128(sum));
		per(k, 7) {
			c = add8(mul8(c, x), _mm256_set1_epi32(q[k + 1]));
			dst[k] = sum_product8(c, w);
		}
	}
	__attribute__((target("avx2"))) static void evaluate_middle8(U *left, U *right, const U *cur, const U *ql, int dl, const U *qr, int dr) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		alignas(32) U window[16], coefficients[2][9] = {};
		for (int i = 0; i < 16; i += 8) {
			I8 x = load8(cur + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			store8(window + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		rep(i, min(dl, 9)) {
			U x = ql[i];
			if (x >= twice) x -= twice;
			if (x >= mod) x -= mod;
			coefficients[0][i] = x;
		}
		rep(i, min(dr, 9)) {
			U x = qr[i];
			if (x >= twice) x -= twice;
			if (x >= mod) x -= mod;
			coefficients[1][i] = x;
		}
		I8 e[2] = {}, o[2] = {};
		rep(i, 9) {
			I8 z = load8(window + 8 - i), high = _mm256_srli_epi64(z, 32);
			rep(k, 2) {
				I8 r = _mm256_set1_epi32(coefficients[k][i]);
				e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(z, r));
				o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(high, r));
			}
		}
		U *dst[2] = {left, right};
		rep(k, 2) {
			e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(_mm256_mul_epu32(e[k], ni), p));
			o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(_mm256_mul_epu32(o[k], ni), p));
			I8 x = _mm256_blend_epi32(_mm256_srli_epi64(e[k], 32), o[k], 0xaa);
			store8(dst[k], _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
	}
	__attribute__((target("avx2"))) static void inverse_sequence8(U *dst, int first, int n) {

		alignas(32) U initial[8];
		rep(j, 8) initial[j] = in(first + j);
		I8 factor = load8(initial), step = _mm256_set1_epi32(in(8));
		I8 p = factor;
		store8(dst, p);
		for (int i = 8; i < n; i += 8) {
			factor = add8(factor, step);
			p = mul8(p, factor);
			store8(dst + i, p);
		}
		I8 inverse = _mm256_set1_epi32(one);
		for (U e = mod - 2; e; e >>= 1, p = mul8(p, p))
			if (e & 1) inverse = mul8(inverse, p);
		for (int i = n - 8; i >= 8; i -= 8) {
			store8(dst + i, mul8(inverse, load8(dst + i - 8)));
			inverse = mul8(inverse, factor);
			factor = sub8(factor, step);
		}
		store8(dst, inverse);
	}
	__attribute__((target("avx2"))) static void normalize8(U *a, int n) {
		I8 p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = load8(a + i);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
#endif
	static void weighted(U *dst, const U *src, int n, int first) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			weighted8(dst, src, n, first);
			return;
		}
#endif
		rep(i, n) dst[i] = mul(src[i], in(first + i));
	}
	static void scale_normalize(U *a, int n, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			scale_normalize8(a, n, r);
			return;
		}
#endif
		U qr = quotient(r);
		int i = 0;
#if defined(__ARM_NEON)
		I v = vdupq_n_u32(r), q = vdupq_n_u32(qr), p = vdupq_n_u32(mod);
		for (; i + 4 <= n; i += 4) {
			I x = fixed_mul4(vld1q_u32(a + i), v, q);
			vst1q_u32(a + i, vminq_u32(x, vsubq_u32(x, p)));
		}
#endif
		for (; i < n; ++i) {
			U x = fixed_mul(a[i], r, qr);
			a[i] = x < mod ? x : x - mod;
		}
	}
	static void square_fold(U *a, const U *q, int n, bool negative) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			square_fold8(a, q, n, negative);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(a + i), y = vld1q_u32(a + n + i);
			I u = mul4(vld1q_u32(q + i), vld1q_u32(q + i));
			I v = mul4(vld1q_u32(q + n + i), vld1q_u32(q + n + i));
			x = add4(x, x);
			vst1q_u32(a + i, negative ? sub4(x, u) : add4(x, u));
			vst1q_u32(a + n + i, add4(add4(y, y), v));
		}
#endif
		for (; i < n; ++i) {
			U u = mul(q[i], q[i]), v = mul(q[n + i], q[n + i]);
			U x = add(a[i], a[i]);
			a[i] = negative ? sub(x, u) : add(x, u);
			a[n + i] = add(add(a[n + i], a[n + i]), v);
		}
	}
	static void normalize(U *a, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			normalize8(a, n);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(a + i);
			vst1q_u32(a + i, vminq_u32(x, vsubq_u32(x, vdupq_n_u32(mod))));
		}
#endif
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
	static void product(U *dst, const U *a, const U *b, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			product8(dst, a, b, n);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = mul4(vld1q_u32(a + i), vld1q_u32(b + i));
			vst1q_u32(dst + i, x);
		}
#endif
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
#if defined(__x86_64__) || defined(__i386__)
	template <bool inverse>
	__attribute__((target("avx2"), always_inline)) void convolution_stage8(U *a, int stride, int k) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
		for (int i = 0; i < stride; i += 8) {
			I8 x = load8(a + i), y = load8(a + stride + i);
			fixed_butterfly8<inverse, true>(x, y, r, rq, k == 0);
			store8(a + i, x);
			store8(a + stride + i, y);
		}
	}
	__attribute__((target("avx2"), always_inline)) void leaf8(U *a, const U *b, int k) const {
		U w = roots[k / 2], q = root_quotients[k / 2];
		if (k & 1) w = mod - w, q = ~q;
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 x = load8(a), y = load8(b);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
		alignas(32) U window[16], coefficients[8];
		I8 low = fixed_mul8(x, _mm256_set1_epi32(w), _mm256_set1_epi32(q));
		store8(window, _mm256_min_epu32(low, _mm256_sub_epi32(low, p)));
		store8(window + 8, x);
		store8(coefficients, y);
		I8 e = _mm256_setzero_si256(), o = e;
		rep(i, 8) {
			I8 z = load8(window + 8 - i), r = _mm256_set1_epi32(coefficients[i]);
			e = _mm256_add_epi64(e, _mm256_mul_epu32(z, r));
			o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
		}
		I8 ni = _mm256_set1_epi32(998244351);
		e = _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, ni), p));
		o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, ni), p));
		x = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		store8(a, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	template <int count, bool second = false>
	__attribute__((target("avx2"), always_inline)) void inverse_leaf_products8(U *a, U *b, const U *ws, const U *qs) const {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		alignas(32) U window[count][16], coefficients[count][8];
		I8 e[count], o[count];
		rep(k, count) {
			I8 x = load8(a + 8 * k), y = load8(b + 8 * k);
			if constexpr (!second) x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			I8 low = fixed_mul8(x, _mm256_set1_epi32(ws[k]), _mm256_set1_epi32(qs[k]));
			store8(window[k], _mm256_min_epu32(low, _mm256_sub_epi32(low, p)));
			store8(window[k] + 8, x);
			store8(coefficients[k], y);
			if constexpr (!second) store8(b + 8 * k, y);
			e[k] = o[k] = _mm256_setzero_si256();
		}
		rep(i, 8)
			rep(k, count) {
			I8 z = load8(window[k] + 8 - i), r = _mm256_set1_epi32(coefficients[k][i]);
			e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(z, r));
			o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
		}
		I8 ni = _mm256_set1_epi32(998244351);
		rep(k, count) {
			I8 x = e[k], y = o[k];
			x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, ni), p));
			y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, ni), p));
			x = _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
			store8(a + 8 * k, _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
	}
	__attribute__((target("avx2"))) static U reverse_dot8(const U *a, const U *b, int n) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		I8 reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0), total = _mm256_setzero_si256();
		int i = 0;
		for (; i + 8 <= n;) {
			I8 even = _mm256_setzero_si256(), odd = even;
			int end = min(n / 8 * 8, i + 64);
			for (; i < end; i += 8) {
				I8 x = load8(a + i), y = _mm256_permutevar8x32_epi32(load8(b - i - 7), reverse);
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			total = add8(total, value);
		}
		total = add8(total, _mm256_permute2x128_si256(total, total, 1));
		total = add8(total, _mm256_shuffle_epi32(total, 0x4e));
		total = add8(total, _mm256_shuffle_epi32(total, 0xb1));
		U result = _mm_cvtsi128_si32(_mm256_castsi256_si128(total));
		for (; i < n; ++i) {
			U x = a[i], y = b[-i];
			if (x >= twice) x -= twice;
			if (y >= twice) y -= twice;
			if (x >= mod) x -= mod;
			if (y >= mod) y -= mod;
			result = add(result, mul(x, y));
		}
		return result;
	}

	__attribute__((target("avx2"))) static void inverse_fold8(U *a, const U *f, int n) {
		int i = 0;
		for (; i + 8 <= n; i += 8) store8(a + i, add8(load8(a + i), load8(f + i)));
		for (; i < n; ++i) {
			a[i] += f[i];
			if (a[i] >= twice) a[i] -= twice;
		}
	}
	__attribute__((target("avx2"))) static void inverse_recover8(U *g, const U *a, int m) {
		int n = 2 * m;
		U ratio = T(3).pow((mod - 1) / (2 * (n / 8))).inv().val();
		U first = mod - (mod - 1) / (2 * (n / 8));
		U phase = T(3).pow((mod - 1) / 4).inv().val();
		U powers[8];
		powers[0] = first;
		rep(j, 1, 8) powers[j] = (unsigned long long)powers[j - 1] * ratio % mod;
		U step = T(ratio).pow(8).val();
		I8 factors = load8(powers), s = _mm256_set1_epi32(step), sq = _mm256_set1_epi32(quotient(step));
		I8 vphase = _mm256_set1_epi32(phase), qphase = _mm256_set1_epi32(quotient(phase)), p = _mm256_set1_epi32(mod), z = _mm256_setzero_si256();
		for (int i = 0; i < m; i += 64) {
			store8(powers, factors);
			rep(j, 8) {
				int at = i + 8 * j;
				U w = powers[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				I8 r = _mm256_add_epi32(load8(a + at), fixed_mul8(load8(a + m + at), vphase, qphase));
				r = fixed_mul8(r, v, q);
				store8(g + m + at, sub8(z, add8(r, load8(g + m + at))));
			}
			factors = fixed_mul8(factors, s, sq);
			factors = _mm256_min_epu32(factors, _mm256_sub_epi32(factors, p));
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void inverse_partial8(U *a, int n, int limit = 0) const {
		if (!limit) limit = n;
		if constexpr (!inverse) {
			int stride = limit / 2;
			for (; stride >= 16; stride >>= 2) radix4_8<false>(a, stride / 2, n);
			if (stride == 8)
				for (int i = 0; i < n; i += 16) convolution_stage8<false>(a + i, 8, i / 16);
		} else {
			int stride = 8;
			for (; 4 * stride <= n; stride <<= 2) radix4_8<true>(a, stride, n);
			if (stride < n) stage8<true>(a, stride, n);
		}
	}
	__attribute__((target("avx2"))) static void inverse_twist8(U *a, U *b, int n, U ratio, U first = one) {
		U powers[8];
		powers[0] = out(first);
		U r = out(ratio);
		rep(j, 1, 8) powers[j] = (unsigned long long)powers[j - 1] * r % mod;
		U step = T::word(ratio).pow(8).val();
		I8 factors = load8(powers), s = _mm256_set1_epi32(step), sq = _mm256_set1_epi32(quotient(step)), p = _mm256_set1_epi32(mod);
		for (int i = 0; i < n; i += 64) {
			store8(powers, factors);
			for (int j = 0; j < 8 && i + 8 * j < n; ++j) {
				U w = powers[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				store8(a + i + 8 * j, fixed_mul8(load8(a + i + 8 * j), v, q));
				if (b && i + 8 * j < n / 2) store8(b + i + 8 * j, fixed_mul8(load8(b + i + 8 * j), v, q));
			}
			factors = fixed_mul8(factors, s, sq);
			factors = _mm256_min_epu32(factors, _mm256_sub_epi32(factors, p));
		}
	}
	template <bool negative>
	__attribute__((target("avx2"))) void inverse_cubic8(U *a, U *b, int n) const {
		U shift = one;
		if constexpr (negative) {
			T psi = T(3).pow((mod - 1) / (2 * (n / 8)));
			shift = psi.v;
			if (shift >= mod) shift -= mod;
			inverse_twist8(a, b, n, shift);
		}
		inverse_partial8<false>(a, n);
		if constexpr (negative) {
			copy_n(b, n / 2, b + n / 2);
			inverse_partial8<false>(b, n, n / 2);
		} else inverse_partial8<false>(b, n);
		for (int i = 0; i < n; i += 8 * 8) {
			U ws[8], qs[8];
			rep(j, 8) {
				int k = i / 8 + j;
				U w = roots[k / 2];
				if (k & 1) w = mod - w;
				if constexpr (negative) {
					w = mul(w, shift);
					if (w >= mod) w -= mod;
					qs[j] = quotient(w);
				} else qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
				ws[j] = w;
			}
			inverse_leaf_products8<8>(a + i, b + i, ws, qs);
			inverse_leaf_products8<8, true>(a + i, b + i, ws, qs);
			rep(j, 8) store8(a + i + 8 * j, sub8(load8(a + i + 8 * j), load8(b + i + 8 * j)));
		}
		inverse_partial8<true>(a, n);
		U normal = mod - (mod - 1) / (2 * (n / 8));
		if constexpr (!negative) scale8(a, n, normal);
	}
	template <int n>
	__attribute__((target("avx2"), always_inline)) void convolution_tile8(U *a, U *b, int k) const {
		if constexpr (n == 8) leaf8(a, b, k);
		else if constexpr (n == 16) {
			convolution_stage8<false>(a, n / 2, k);
			convolution_stage8<false>(b, n / 2, k);
			convolution_tile8<n / 2>(a, b, 2 * k);
			convolution_tile8<n / 2>(a + n / 2, b + n / 2, 2 * k + 1);
			convolution_stage8<true>(a, n / 2, k);
		} else {
			stage_pair8<false, true>(a, n / 4, n, k);
			stage_pair8<false, true>(b, n / 4, n, k);
			rep(i, 4) convolution_tile8<n / 4>(a + i * (n / 4), b + i * (n / 4), 4 * k + i);
			stage_pair8<true>(a, n / 4, n, k);
		}
	}
	__attribute__((target("avx2"))) void convolution8(U *a, U *b, int n, int k = 0) const {
		if (n == 256) {
			convolution_tile8<256>(a, b, k);
			return;
		}
		if (n == 128) {
			convolution_tile8<128>(a, b, k);
			return;
		}
		if (n == 64) {
			convolution_tile8<64>(a, b, k);
			return;
		}
		stage_pair8<false, true>(a, n / 4, n, k);
		stage_pair8<false, true>(b, n / 4, n, k);
		rep(i, 4) convolution8(a + i * (n / 4), b + i * (n / 4), n / 4, 4 * k + i);
		stage_pair8<true>(a, n / 4, n, k);
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	template <bool odd>
	__attribute__((target("avx2"))) static void reduce_pairs8(U *a, U *b, const U *twist, int n) {
		for (int i = 0; i < n; i += 8) {
			I8 x, y, u, v;
			load_pairs(a + 2 * i, x, y);
			load_pairs(b + 2 * i, u, v);
			x = mul8(x, v);
			y = mul8(y, u);
			if constexpr (odd) x = mul8(sub8(x, y), load8(twist + i));
			else x = add8(x, y);
			store8(a + i, x);
			store8(b + i, mul8(u, v));
		}
	}
	__attribute__((target("avx2"))) static void combine8(U *a, const U *b, const U *x, const U *y, int n) {
		for (int i = 0; i < n; i += 8) store8(a + i, add8(mul8(load8(a + i), load8(y + i)), mul8(load8(b + i), load8(x + i))));
	}
#endif

	template <bool odd>
	static void reduce_pairs(U *a, U *b, const U *twist, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			reduce_pairs8<odd>(a, b, twist, n);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			auto p = vld2q_u32(a + 2 * i), q = vld2q_u32(b + 2 * i);
			I x = mul4(p.val[0], q.val[1]), y = mul4(p.val[1], q.val[0]);
			if constexpr (odd) x = mul4(sub4(x, y), vld1q_u32(twist + i));
			else x = add4(x, y);
			vst1q_u32(a + i, x);
			vst1q_u32(b + i, mul4(q.val[0], q.val[1]));
		}
#endif
		for (; i < n; ++i) {
			U u = mul(a[2 * i], b[2 * i + 1]), v = mul(a[2 * i + 1], b[2 * i]);
			a[i] = odd ? mul(sub(u, v), twist[i]) : add(u, v);
			b[i] = mul(b[2 * i], b[2 * i + 1]);
		}
	}
	static void combine(U *a, const U *b, const U *x, const U *y, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			combine8(a, b, x, y, n);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) vst1q_u32(a + i, add4(mul4(vld1q_u32(a + i), vld1q_u32(y + i)), mul4(vld1q_u32(b + i), vld1q_u32(x + i))));
#endif
		for (; i < n; ++i) a[i] = add(mul(a[i], y[i]), mul(b[i], x[i]));
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void geometric8(U *a, int n, U first, U step) {
		alignas(32) U factors[8];
		U power = one;
		rep(i, 8) {
			factors[i] = mul(first, power);
			power = mul(power, step);
		}
		I8 r = load8(factors), ratio = _mm256_set1_epi32(power);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(a + i, mul8(load8(a + i), r));
			r = mul8(r, ratio);
		}
		store8(factors, r);
		for (int j = 0; i < n; ++i, ++j) a[i] = mul(a[i], factors[j]);
	}
#endif
	static void geometric(U *a, int n, U first, U step) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			geometric8(a, n, first, step);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		U factors[4], power = one;
		rep(j, 4) {
			factors[j] = mul(first, power);
			power = mul(power, step);
		}
		I r = vld1q_u32(factors), ratio = vdupq_n_u32(power);
		for (; i + 4 <= n; i += 4) {
			vst1q_u32(a + i, mul4(vld1q_u32(a + i), r));
			r = mul4(r, ratio);
		}
		vst1q_u32(factors, r);
		first = factors[0];
#endif
		for (; i < n; ++i) {
			a[i] = mul(a[i], first);
			first = mul(first, step);
		}
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void add_scaled8(U *a, const U *b, int n, U r) {
		U q = quotient(r);
		I8 v = _mm256_set1_epi32(r), vq = _mm256_set1_epi32(q);
		int i = 0;
		for (; i + 8 <= n; i += 8) store8(a + i, add8(load8(a + i), fixed_mul8(load8(b + i), v, vq)));
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
#endif
	static void add_scaled(U *a, const U *b, int n, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			add_scaled8(a, b, n, r);
			return;
		}
#endif
		int i = 0;
		U q = quotient(r);
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) vst1q_u32(a + i, add4(vld1q_u32(a + i), fixed_mul4(vld1q_u32(b + i), vdupq_n_u32(r), vdupq_n_u32(q))));
#endif
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
	void leaf(U *a, const U *b, int k) const {
		U w = roots[k / 2], q = root_quotients[k / 2];
		if (k & 1) w = mod - w, q = ~q;
		U window[16], coefficients[8];
		rep(i, 8) {
			U x = a[i] < mod ? a[i] : a[i] - mod;
			U z = fixed_mul(x, w, q);
			window[i] = z < mod ? z : z - mod;
			window[8 + i] = x;
			coefficients[i] = b[i] < mod ? b[i] : b[i] - mod;
		}
#if defined(__ARM_NEON)
		for (int j = 0; j < 8; j += 4) {
			uint64x2_t lo = vdupq_n_u64(0), hi = lo;
			rep(i, 8) {
				I z = vld1q_u32(window + 8 - i + j);
				lo = vmlal_n_u32(lo, vget_low_u32(z), coefficients[i]);
				hi = vmlal_n_u32(hi, vget_high_u32(z), coefficients[i]);
			}
			uint32x2_t x = vmul_n_u32(vmovn_u64(lo), 998244351u), y = vmul_n_u32(vmovn_u64(hi), 998244351u);
			I z = vcombine_u32(vshrn_n_u64(vmlal_n_u32(lo, x, mod), 32), vshrn_n_u64(vmlal_n_u32(hi, y, mod), 32));
			z = vminq_u32(z, vsubq_u32(z, vdupq_n_u32(twice)));
			vst1q_u32(a + j, vminq_u32(z, vsubq_u32(z, vdupq_n_u32(mod))));
		}
#else
		rep(j, 8) {
			unsigned long long sum = 0;
			rep(i, 8) sum += (unsigned long long)window[8 - i + j] * coefficients[i];
			U x = reduce(sum);
			if (x >= twice) x -= twice;
			a[j] = x < mod ? x : x - mod;
		}
#endif
	}
	template <int n>
	void convolution_tile(U *a, U *b, int k) const {
		if constexpr (n == 8) leaf(a, b, k);
		else if constexpr (n == 16) {
			stage<false>(a, n / 2, n, k);
			stage<false>(b, n / 2, n, k);
			convolution_tile<n / 2>(a, b, 2 * k);
			convolution_tile<n / 2>(a + n / 2, b + n / 2, 2 * k + 1);
			stage<true>(a, n / 2, n, k);
		} else {
			stage_pair<false>(a, n / 4, n, k);
			stage_pair<false>(b, n / 4, n, k);
			rep(i, 4) convolution_tile<n / 4>(a + i * (n / 4), b + i * (n / 4), 4 * k + i);
			stage_pair<true>(a, n / 4, n, k);
		}
	}
	void convolution(U *a, U *b, int n, int k = 0) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			convolution8(a, b, n, k);
			return;
		}
#endif
		if (n == 256) {
			convolution_tile<256>(a, b, k);
			return;
		}
		if (n == 128) {
			convolution_tile<128>(a, b, k);
			return;
		}
		if (n == 64) {
			convolution_tile<64>(a, b, k);
			return;
		}
		stage_pair<false>(a, n / 4, n, k);
		stage_pair<false>(b, n / 4, n, k);
		rep(i, 4) convolution(a + i * (n / 4), b + i * (n / 4), n / 4, 4 * k + i);
		stage_pair<true>(a, n / 4, n, k);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void accumulate8(U *dst, const U *a, const U *b, const U *c, int n, bool minus) {
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(b + i), y = load8(c + i);
			x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, _mm256_set1_epi32(twice)), y)
					  : _mm256_add_epi32(x, y);
			store8(dst + i, add8(load8(dst + i), mul8(load8(a + i), x)));
		}
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	template <int count>
	__attribute__((target("avx2"))) static void accumulate_batch8(U *dst, const U *const *a, const U *const *b,
																  const U *const *c, int n, bool minus) {
		static_assert(count >= 1 && count <= 3);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		int offset = minus ? n : 0;
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 3
			rep(k, count) {
				I8 x = load8(b[k] + offset + i), y = load8(c[k] + offset + i);
				x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, pp), y) : _mm256_add_epi32(x, y);
				I8 z = load8(a[k] + offset + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(z, x));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), _mm256_srli_epi64(x, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			store8(dst + i, add8(load8(dst + i), value));
		}
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	template <int count, bool first, bool canonical>
	__attribute__((target("avx2"))) static void accumulate_wide8(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
		static_assert(count >= 1 && count <= 7);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		int offset = minus ? n : 0;
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
			rep(k, count) {
				I8 x = load8(b[k] + offset + i), y = load8(c[k] + offset + i);
				x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, canonical ? p : pp), y) : _mm256_add_epi32(x, y);
				if constexpr (!canonical) x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
				I8 z = load8(a[k] + offset + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(z, x));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), _mm256_srli_epi64(x, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (count == 7) value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (!first) value = add8(load8(dst + i), value);
			store8(dst + i, value);
		}
	}
#endif
	template <int count, bool first, bool canonical>
	static void accumulate_wide(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate_wide8<count, first, canonical>(dst, a, b, c, n, minus);
			return;
		}
#endif
		if constexpr (first) fill_n(dst, n, 0u);
		int offset = minus ? n : 0;
		rep(k, count) accumulate(dst, a[k] + offset, b[k] + offset, c[k] + offset, n, minus);
	}
	template <int count>
	static void accumulate_batch(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate_batch8<count>(dst, a, b, c, n, minus);
			return;
		}
#endif
		int offset = minus ? n : 0;
		rep(k, count) accumulate(dst, a[k] + offset, b[k] + offset, c[k] + offset, n, minus);
	}
	static void accumulate(U *dst, const U *a, const U *b, const U *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate8(dst, a, b, c, n, minus);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(b + i), y = vld1q_u32(c + i);
			x = minus ? sub4(x, y) : add4(x, y);
			vst1q_u32(dst + i, add4(vld1q_u32(dst + i), mul4(vld1q_u32(a + i), x)));
		}
#endif
		for (; i < n; ++i) dst[i] = add(dst[i], mul(a[i], minus ? sub(b[i], c[i]) : add(b[i], c[i])));
	}
	static void scale_copy(U *dst, const U *src, int length, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			scale_copy8(dst, src, length, r);
			return;
		}
#endif
		U q = quotient(r);
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) vst1q_u32(dst + i, fixed_mul4(vld1q_u32(src + i), vdupq_n_u32(r), vdupq_n_u32(q)));
#endif
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, q);
	}
	void scale(U *a, int length, U r) const {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			scale8(a, length, r);
			return;
		}
#endif
		int i = 0;
		U q = quotient(r);
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4)
			vst1q_u32(a + i, fixed_mul4(vld1q_u32(a + i), vdupq_n_u32(r), vdupq_n_u32(q)));
#endif
		for (; i < length; ++i) a[i] = fixed_mul(a[i], r, q);
	}

	static void pair_product(U *a, const U *b, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			pair_product8(a, b, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) {
			uint32x4x2_t v = vld2q_u32(b + 2 * i);
			vst1q_u32(a + i, mul4(v.val[0], v.val[1]));
		}
#endif
		for (; i < length; ++i) a[i] = mul(b[2 * i], b[2 * i + 1]);
	}
	static void pair_multiply(U *a, const U *b, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			pair_multiply8(a, b, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) {
			uint32x4x2_t v = vld2q_u32(a + 2 * i);
			I x = mul4(v.val[1], vld1q_u32(b + i));
			v.val[1] = mul4(v.val[0], vld1q_u32(b + i));
			v.val[0] = x;
			vst2q_u32(a + 2 * i, v);
		}
#endif
		for (; i < length; ++i) {
			U x = a[2 * i];
			a[2 * i] = mul(b[i], a[2 * i + 1]);
			a[2 * i + 1] = mul(b[i], x);
		}
	}
	static void difference(U *a, const U *b, const U *c, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			difference8(a, b, c, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4)
			vst1q_u32(a + i, sub4(vld1q_u32(b + i), vld1q_u32(c + i)));
#endif
		for (; i < length; ++i) a[i] = sub(b[i], c[i]);
	}
	static void fold(U *a, int columns) {
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 8 && vectorized()) {
			fold8(a, columns);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= columns; i += 4) {
			I x = add4(vld1q_u32(a + i), vld1q_u32(a + columns + i));
			vst1q_u32(a + i, x);
			vst1q_u32(a + columns + i, x);
		}
#endif
		for (; i < columns; ++i) a[i] = a[columns + i] = add(a[i], a[columns + i]);
	}
	void project(U *a, int rows, int columns) const {
		if (columns == 1) {
#if defined(__x86_64__) || defined(__i386__)
			if (rows >= 8 && vectorized()) {
				project1_8(a, rows);
				return;
			}
#endif
			int j = 0;
#if defined(__ARM_NEON)
			for (; j + 4 <= rows; j += 4) {
				uint32x4x2_t v = vld2q_u32(a + 2 * j);
				v.val[0] = v.val[1] = add4(v.val[0], v.val[1]);
				vst2q_u32(a + 2 * j, v);
			}
#endif
			for (; j < rows; ++j) a[2 * j] = a[2 * j + 1] = add(a[2 * j], a[2 * j + 1]);
			return;
		}
		if (columns == 2) {
#if defined(__x86_64__) || defined(__i386__)
			if (rows >= 8 && vectorized()) {
				project2_8(a, rows);
				return;
			}
#endif
			int j = 0;
#if defined(__ARM_NEON)
			for (; j + 4 <= rows; j += 4) {
				uint32x4x4_t v = vld4q_u32(a + 4 * j);
				I x = add4(add4(v.val[0], v.val[1]), add4(v.val[2], v.val[3]));
				I y = add4(sub4(v.val[0], v.val[1]),
						   fixed_mul4(sub4(v.val[2], v.val[3]), vdupq_n_u32(inverse_roots[1]),
									  vdupq_n_u32(inverse_root_quotients[1])));
				v.val[0] = add4(x, y);
				v.val[1] = sub4(x, y);
				y = fixed_mul4(y, vdupq_n_u32(roots[1]), vdupq_n_u32(root_quotients[1]));
				v.val[2] = add4(x, y);
				v.val[3] = sub4(x, y);
				vst4q_u32(a + 4 * j, v);
			}
#endif
			for (; j < rows; ++j) {
				U *r = a + 4 * j;
				U x = add(add(r[0], r[1]), add(r[2], r[3]));
				U y = add(sub(r[0], r[1]), fixed_mul(sub(r[2], r[3]), inverse_roots[1],
													 inverse_root_quotients[1]));
				r[0] = add(x, y);
				r[1] = sub(x, y);
				y = fixed_mul(y, roots[1], root_quotients[1]);
				r[2] = add(x, y);
				r[3] = sub(x, y);
			}
			return;
		}
		if (columns == 4) {
#if defined(__x86_64__) || defined(__i386__)
			if (vectorized()) {
				project4_8(a, rows);
				return;
			}
#endif
#if defined(__ARM_NEON)
			rep(j, rows) {
				I x = vld1q_u32(a + 8 * j), y = vld1q_u32(a + 8 * j + 4);
				small4<1, true>(x, y, 0);
				small4<2, true>(x, y, 0);
				x = y = add4(x, y);
				small4<2, false>(x, y, 0);
				small4<1, false>(x, y, 0);
				vst1q_u32(a + 8 * j, x);
				vst1q_u32(a + 8 * j + 4, y);
			}
			return;
#endif
		}
#if defined(__x86_64__) || defined(__i386__)
		if (columns == 8 && vectorized()) {
			project8_8(a, rows);
			return;
		}
#endif
#if defined(__x86_64__) || defined(__i386__)
		if (columns == 16 && vectorized()) {
			project16_8(a, rows);
			return;
		}
#endif
		rep(j, rows) {
			U *row = a + 2 * columns * j;
			fft<true>(row, 2 * columns, columns);
			fold(row, columns);
			fft<false>(row, 2 * columns, columns);
		}
	}
	void composition_difference(U *a, const U *b, const U *c, int n) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			composition_difference8(a, b, c, n);
			return;
		}
#endif
		difference(a, b, c, n);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_difference8(U *a, const U *b, const U *c, int n) {
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(c + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			store8(a + i, sub8(load8(b + i), x));
		}
	}
#endif
	void composition_finish(U *p, const U *v, int columns, U factor) const {
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 8 && vectorized()) {
			composition_finish8(p, v, columns, out(factor));
			return;
		}
#endif
		rep(i, columns) p[i] = mul(sub(add(v[i], v[columns + i]), add(v[2 * columns + i], v[3 * columns + i])), factor);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_finish8(U *dst, const U *v, int n, U factor) {
		I8 r = _mm256_set1_epi32(factor), q = _mm256_set1_epi32(quotient(factor));
		for (int i = 0; i < n; i += 8) {
			I8 x = add8(load8(v + i), load8(v + n + i)), y = add8(load8(v + 2 * n + i), load8(v + 3 * n + i));
			store8(dst + i, fixed_mul8(sub8(x, y), r, q));
		}
	}
#endif
	struct CompositionDelete {
		void operator()(uint *p) const { ::operator delete[](p, align_val_t(64)); }
	};
	using CompositionWords = unique_ptr<uint[], CompositionDelete>;
	void composition_forward_y(U *a, int size, int columns) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			int h = size;
			bool lazy = false;
			for (; h / 2 > columns && h / 2 >= 8; h >>= 2) {
				radix4_8<false>(a, h / 2, 2 * size);
				lazy = true;
			}
			if (lazy && h > columns) composition_shrink8(a, 2 * size);
			for (; h / 2 > columns; h >>= 2) stage_pair<false>(a, h / 2, 2 * size);
			if (h > columns) stage<false>(a, h, 2 * size);
			return;
		}
#endif
		int h = size;
		for (; h / 2 > columns; h >>= 2) stage_pair<false>(a, h / 2, 2 * size);
		if (h > columns) stage<false>(a, h, 2 * size);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_shrink8(U *a, int n) {
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(a + i);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
	}
#endif
	void double_y(U *a, int rows, int columns, bool inverse) const {
		const int size = rows * columns;
		int h = 2 * columns;
		for (; h <= size / 2; h <<= 2) {
#if defined(__x86_64__) || defined(__i386__)
			if (h >= 8 && vectorized()) radix4_8<true>(a, h, 2 * size);
			else
#endif
				stage_pair<true>(a, h, 2 * size);
		}
		if (h <= size) stage<true>(a, h, 2 * size);
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 2 && vectorized()) {
			scale_rows8(a, rows, columns, (inverse ? inverse_twists : twists).data() + rows);
			return;
		}
#endif
		rep(j, rows)
			scale(a + 2 * columns * j, 2 * columns,
				  (inverse ? inverse_twists : twists)[rows + j]);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void projection_dot8(U *p, const U *q, int n) {
		I8 ni = _mm256_set1_epi32(998244351), modulus = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(p + 2 * i), y = load8(p + 2 * i + 8), a = load8(q + 2 * i), b = load8(q + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
			a = _mm256_min_epu32(a, _mm256_sub_epi32(a, pp));
			b = _mm256_min_epu32(b, _mm256_sub_epi32(b, pp));
			I8 lo = _mm256_add_epi64(_mm256_mul_epu32(x, _mm256_srli_epi64(a, 32)), _mm256_mul_epu32(_mm256_srli_epi64(x, 32), a));
			I8 hi = _mm256_add_epi64(_mm256_mul_epu32(y, _mm256_srli_epi64(b, 32)), _mm256_mul_epu32(_mm256_srli_epi64(y, 32), b));
			lo = _mm256_add_epi64(lo, _mm256_mul_epu32(_mm256_mul_epu32(lo, ni), modulus));
			hi = _mm256_add_epi64(hi, _mm256_mul_epu32(_mm256_mul_epu32(hi, ni), modulus));
			I8 r = _mm256_permutevar8x32_epi32(_mm256_blend_epi32(_mm256_srli_epi64(lo, 32), hi, 0xaa), index);
			store8(p + i, _mm256_min_epu32(r, _mm256_sub_epi32(r, pp)));
		}
	}
#endif

	static void projection_dot(U *p, const U *q, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			projection_dot8(p, q, n);
			return;
		}
#endif
		rep(i, n) {
			U x = p[2 * i], y = p[2 * i + 1], a = q[2 * i], b = q[2 * i + 1];
			if (x >= twice) x -= twice;
			if (y >= twice) y -= twice;
			if (a >= twice) a -= twice;
			if (b >= twice) b -= twice;
			U r = reduce((unsigned long long)x * b + (unsigned long long)y * a);
			p[i] = r < twice ? r : r - twice;
		}
	}
	void transpose_roots() {
		roots.swap(inverse_roots);
		root_quotients.swap(inverse_root_quotients);
		cube_roots.swap(inverse_cube_roots);
		cube_quotients.swap(inverse_cube_quotients);
	}
	U reversion_projection(U *p, U *q, int size) {
		int levels = __builtin_ctz((uint)size);
		V saved(levels, one);
		U q_one = one, p_one = one;
		for (int i = 1, columns = size / 2; columns > 1; ++i, columns >>= 1) {
			q_one = mul(mul(q_one, q_one), in(2 * columns));
			saved[i] = q_one;
		}
		for (int i = levels - 1, columns = 2; i >= 0; --i, columns <<= 1) {
			p_one = mul(p_one, saved[i]);
			if (i) p_one = mul(p_one, in(4 * columns));
		}
		U factor = T::word(mul(p_one, in(4 * size))).inv().v;
		fft<false>(q, 2 * size);
		rep(i, 2 * size) {
			U x = q[i];
			q[i] = add(x, one);
			q[2 * size + i] = sub(x, one);
		}

		rep(i, size) {
			U x = fixed_mul(sub(q[2 * i + 1], q[2 * i]), roots[i], root_quotients[i]);
			p[i] = i < size / 2 ? x : sub(0, x);
			p[size + i] = sub(0, p[i]);
		}
		q_one = one;
		for (int columns = size / 2, rows = 2; columns > 1; columns >>= 1, rows <<= 1) {
			pair_product(q, q, 2 * size);
			q_one = mul(q_one, q_one);
			project(q, rows, columns);
			copy_n(q, 2 * size, q + 2 * size);
			double_y(q + 2 * size, rows, columns, false);
			q_one = mul(q_one, in(2 * columns));
			U correction = add(q_one, q_one);
			rep(i, 2 * columns) q[2 * size + i] = sub(q[2 * size + i], correction);
			composition_forward_y(q + 2 * size, size, columns);
			transpose_roots();
			project(p, rows, columns);
			rep(i, 2 * size) p[2 * size + i] = sub(0, p[i]);
			double_y(p + 2 * size, rows, columns, true);
			composition_forward_y(p + 2 * size, size, columns);
			transpose_roots();
			projection_dot(p, q, 2 * size);
		}
		rep(i, size) p[i] = add(p[2 * i], p[2 * i + 1]);
		transpose_roots();
		fft<true>(p, size);
		transpose_roots();
		return factor;
	}

	void compose(U *p, const U *q, int rows, int columns, U &p_one, U &q_one, U *workspace = nullptr, int level = 0) const {
		const int size = rows * columns;
		if (columns == 1) {
			fft<false>(p, rows);
			per(i, rows) p[2 * i] = p[2 * i + 1] = p[i];
			return;
		}
		CompositionWords storage;
		if (!workspace) {
			int levels = __builtin_ctz((uint)columns);
			storage.reset(static_cast<uint *>(::operator new[]((4 * size + 16) * levels * sizeof(uint), align_val_t(64))));
			workspace = storage.get();
		}
		U *v = workspace + (4 * size + 16) * level;
		if (rows == 1) {
			fill_n(v + columns, columns, U(0));
			rep(i, columns) v[i] = sub(0, q[i]);
			fft<false>(v, 2 * columns);
			rep(i, 2 * columns) {
				U x = v[i];
				v[i] = add(x, one);
				v[i + 2 * columns] = sub(x, one);
			}
		} else {
			pair_product(v, q, 2 * size);
			q_one = mul(q_one, q_one);
			project(v, rows, columns);
			copy_n(v, 2 * size, v + 2 * size);
			U *upper = v + 2 * size;
			double_y(upper, rows, columns, false);
			q_one = mul(q_one, in(2 * columns));
			U correction = add(q_one, q_one);
			rep(i, 2 * columns) upper[i] = sub(upper[i], correction);
			composition_forward_y(upper, size, columns);
		}
		U saved = q_one;
		compose(p, v, 2 * rows, columns / 2, p_one, q_one, workspace, level + 1);
		pair_multiply(v, p, 2 * size);
		p_one = mul(p_one, saved);
		if (rows == 1) {
			fft<true>(v, 2 * columns, columns);
			fft<true>(v + 2 * columns, 2 * columns, columns);
			U factor = in(T(out(mul(p_one, in(4 * size)))).inv().val());
			composition_finish(p, v, columns, factor);
		} else {
			U *upper = v + 2 * size;
			double_y(upper, rows, columns, true);
			composition_forward_y(upper, size, columns);
			p_one = add(p_one, p_one);
			composition_difference(p, v, upper, 2 * size);
			project(p, rows, columns);
			p_one = mul(p_one, in(2 * columns));
		}
	}
};
} // namespace fps_detail

template <class T = fps_mint>
struct FormalPowerSeries : vec<T> {
	static_assert(T::mod() == 998244353, "FormalPowerSeries supports only mod 998244353");
	using W = FormalPowerSeries<fps_detail::Mint>;
	using base = vec<T>;
	using F = FormalPowerSeries;
	using base::base;

	FormalPowerSeries() = default;
	FormalPowerSeries(base v) : base(std::move(v)) {}
	F &operator=(base v) {
		base::operator=(std::move(v));
		return *this;
	}
	F &operator=(initializer_list<T> v) {
		base::operator=(v);
		return *this;
	}

	static F multiply(const F &a, const F &b) {
		if (a.empty() || b.empty()) return {};
		if constexpr (!is_same_v<T, fps_detail::Mint>) {
			W x = import_work(a);
			return export_work(&a == &b ? W::multiply(x, x) : W::multiply(x, import_work(b)));
		}
		if (min(a.size(), b.size()) <= 32) {
			F r(a.size() + b.size() - 1);
			const F &small = a.size() < b.size() ? a : b;
			const F &large = a.size() < b.size() ? b : a;
			if constexpr (is_same_v<T, fps_detail::Mint>) {
				rep(j, sz(small))
					fps_detail::NTT::add_scaled(words(r) + j, words(large), large.size(), small[j].val());
			} else
				rep(j, sz(small))
					rep(i, sz(large)) r[i + j] += large[i] * small[j];
			return r;
		}
		int n = 1, size = a.size() + b.size() - 1;
		while (n < size) n <<= 1;
		assert(n <= (1 << 23));
		F x = a, y;
		x.resize(n);
		if (&a != &b) {
			y = b;
			y.resize(n);
		}
		if (n >= 64 && &a != &b) {
			auto &t = transform(n / 16);
			t.convolution(words(x), words(y), n);
			x.resize(size);
			t.scale(words(x), size, T::mod() - (T::mod() - 1) / (n / 8));
			return x;
		}
		ntt(x);
		if (&a == &b) pointwise(x, x, x, n);
		else {
			ntt(y);
			pointwise(x, x, y, n);
		}
		intt(x);
		x.resize(size);
		return x;
	}

	F operator-() const {
		F r = *this;
		for (T &x : r) x = -x;
		return r;
	}
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (size_t i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
		return *this;
	}
	F &operator+=(T x) {
		if (this->empty()) this->resize(1);
		(*this)[0] += x;
		return *this;
	}
	F &operator-=(T x) { return *this += -x; }
	F &operator*=(const F &g) { return *this = multiply(*this, g); }
	F &operator*=(T x) {
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			scale_words(*this, x);
			return *this;
		}
		for (T &a : *this) a *= x;
		return *this;
	}
	F &operator/=(const F &g) { return *this = div_series(g); }
	F &operator%=(const F &g) { return *this = divmod(g).second; }
	F &operator/=(T x) { return *this *= x.inv(); }

	F &operator<<=(int d) {
		assert(d >= 0);
		const int n = sz((*this));
		if (d >= n) return *this = F(n);
		this->insert(this->begin(), d, T(0));
		this->resize(n);
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		const int n = sz((*this));
		if (d >= n) return *this = F(n);
		this->erase(this->begin(), this->begin() + d);
		this->resize(n);
		return *this;
	}

	F operator+(const F &g) const { return F(*this) += g; }
	F operator-(const F &g) const { return F(*this) -= g; }
	F operator*(const F &g) const { return multiply(*this, g); }
	F operator/(const F &g) const { return div_series(g); }
	F operator%(const F &g) const { return divmod(g).second; }
	F operator+(T x) const { return F(*this) += x; }
	F operator-(T x) const { return F(*this) -= x; }
	F operator*(T x) const { return F(*this) *= x; }
	F operator/(T x) const { return F(*this) /= x; }
	F operator<<(int d) const { return F(*this) <<= d; }
	F operator>>(int d) const { return F(*this) >>= d; }

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + min<int>(n, this->size()));
	}

	F div_series(const F &g, int deg = -1) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).div_series(import_work(g), deg));
		if (deg < 0) deg = this->size();
		assert(deg >= 0);
		if (!deg) return {};
		assert(!g.empty() && g[0] != T(0));
		if (this->empty()) return F(deg);
		if (deg <= 128 || g.size() < 64) {
			F r = multiply(*this, g.inv(deg)).pre(deg);
			r.resize(deg);
			return r;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		const int block = size / 32, length = 2 * block, count = (deg + block - 1) / block;
		F inverse = g.inv(block), r = multiply(pre(block), inverse).pre(block);
		r.resize(deg);
		inverse.resize(length);
		ntt_half(inverse);
		const Word norm = T::mod() - (T::mod() - 1) / length;
		transform(1).scale(words(inverse), length, norm);
		fps_detail::NTT::CompositionWords storage(new (align_val_t(64)) uint[(2 * count - 1) * length]);
		Word *denominator = storage.get(), *quotient = denominator + count * length;
		const int first = min<int>(block, g.size());
		copy_n(words(g), first, denominator);
		fill(denominator + first, denominator + block, 0u);
		ntt_half(denominator, length);
		base error(length);
		rep(i, 1, count) {
			const int offset = i * block, n = min(block, deg - offset);
			Word *h = denominator + i * length, *q = quotient + (i - 1) * length;
			const int available = max(0, min<int>(block, sz(g) - offset));
			if (available) copy_n(words(g) + offset, available, h);
			fill(h + available, h + block, 0u);
			ntt_half(h, length);
			copy_n(words(r) + offset - block, block, q);
			ntt_half(q, length);
			fps_detail::NTT::scale_normalize(q, length, norm);

			for (int j = 0; j < i; j += 4) {
				const Word *a[4], *b[4], *c[4];
				int terms = min(4, i - j);
				rep(k, terms) {
					a[k] = quotient + (j + k) * length;
					b[k] = denominator + (i - j - k) * length;
					c[k] = denominator + (i - j - k - 1) * length;
				}
				auto update = [&](auto number) {
					constexpr int count = decltype(number)::value;
					if (j == 0) {
						fps_detail::NTT::template accumulate_wide<count, true, false>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_wide<count, true, false>(words(error) + block, a, b, c, block, true);
					} else {
						fps_detail::NTT::template accumulate_wide<count, false, false>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_wide<count, false, false>(words(error) + block, a, b, c, block, true);
					}
				};
				if (terms == 4) update(integral_constant<int, 4>{});
				else if (terms == 3) update(integral_constant<int, 3>{});
				else if (terms == 2) update(integral_constant<int, 2>{});
				else update(integral_constant<int, 1>{});
			}
			ntt_inverse(error);
			rep(j, block) error[j] = (offset + j < sz((*this)) ? (*this)[offset + j] : T(0)) - error[j];
			ntt_half(error);
			pointwise(error, error, inverse, length);
			ntt_inverse(error);
			copy_n(error.begin(), n, r.begin() + offset);
		}
		return r;
	}

	F inv(int deg = -1) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).inv(deg));
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] != T(0));
		F g{(*this)[0].inv()};
		auto terms = sparse_terms(deg, 32);
		if (terms.size() <= 32) {
			g.resize(deg);
			rep(i, 1, deg) {
				for (auto [j, x] : terms) {
					if (j > i) break;
					g[i] -= x * g[i - j];
				}
				g[i] *= g[0];
			}
			return g;
		}

		if (deg > 256 && this->size() >= 64) {
#if defined(__x86_64__) || defined(__i386__)
			if (deg < 1024 || !fps_detail::NTT::vectorized()) return F{1}.div_series(*this, deg);
#else
			return F{1}.div_series(*this, deg);
#endif
		}
		g.resize(min(deg, 32));
		rep(i, 1, sz(g))
			g[i] =
				-g[0] * dot(this->data() + 1, g.data() + i - 1, min<int>(i, this->size() - 1));
#if defined(__x86_64__) || defined(__i386__)
		fps_detail::NTT::CompositionWords storage;
		Word *a = nullptr, *b = nullptr;
		if (deg > 96 && fps_detail::NTT::vectorized()) {
			int capacity = 64;
			while (capacity < deg && capacity < (1 << 23)) capacity <<= 1;
			storage.reset(new (align_val_t(64)) uint[2 * capacity]);
			a = storage.get();
			b = a + capacity;
			g.reserve(capacity);
			transform(max(32, capacity / 16));
		}
#endif
		while (sz(g) < deg) {
			const int m = sz(g), n = 2 * m;
			if (m >= 32 && (deg - m <= 32 || (m >= 512 && deg - m <= 96))) {
				g.resize(deg);
				rep(i, m, deg) {
					const int count = min<int>(i, this->size() - 1);
					g[i] = -g[0] * dot(this->data() + 1, g.data() + i - 1, count);
				}
				break;
			}
#if defined(__x86_64__) || defined(__i386__)
			if (m >= 64 && n <= (1 << 23) && fps_detail::NTT::vectorized()) {
				int lo = min<int>(m, this->size());
				copy_n(words(*this), lo, a);
				fill(a + lo, a + m, Word(0));
				copy_n(words(g), m, b);
				int hi = max(0, min<int>(m, this->size() - m));
				if (hi) fps_detail::NTT::inverse_fold8(a, words(*this) + m, hi);
				transform(max(1, m / 16)).template inverse_cubic8<false>(a, b, m);
				g.resize(n);
				copy_n(a, m, words(g) + m);
				lo = min<int>(n, this->size());
				copy_n(words(*this), lo, a);
				fill(a + lo, a + n, Word(0));
				copy_n(words(g), m, b);
				transform(max(1, n / 16)).template inverse_cubic8<true>(a, b, n);
				fps_detail::NTT::inverse_recover8(words(g), a, m);

			} else
#endif
			{
				base f = this->pre(n), r = g;
				f.resize(n);
				r.resize(n);
				ntt(f);
				ntt_half(r);
				pointwise(f, f, r, n);
				ntt_inverse(f);
				fill(f.begin(), f.begin() + m, T(0));
				ntt(f);
				pointwise(f, f, r, n);
				ntt_inverse(f);
				const T inv = T::raw(T::mod() - (T::mod() - 1) / n);
				const T scale = -inv * inv;
				g.resize(n);
				transform(1).scale(words(f) + m, m, scale.val());
				copy_n(f.begin() + m, m, g.begin() + m);
			}
		}
		g.resize(deg);
		return g;
	}

	F sqrt(int deg = -1) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).sqrt(deg));
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0);
		F f = pre(deg);
		f.trim();
		int first = 0;
		while (first < sz(f) && f[first] == T(0)) ++first;
		if (first == sz(f)) return F(deg);
		if (first & 1) return {};
		const T constant = mod_sqrt(f[first]);
		if (constant == T(0)) return {};
		if (first + 1 == sz(f)) {
			F result(deg);
			result[first / 2] = constant;
			return result;
		}
		const int shift = first / 2, target = deg - shift;
		f.erase(f.begin(), f.begin() + first);
		f.resize(target);
		const T half = T(2).inv(), scale = half / constant;
		F g(min(target, 32));
		g[0] = constant;
		rep(i, 1, sz(g)) {
			T sum = 0;
			rep(j, 1, i) sum += g[j] * g[i - j];
			g[i] = (f[i] - sum) * scale;
		}
		if (target > 256) {
			int size = 1;
			while (size < target) size <<= 1;
			const int block = size / 32, length = 2 * block, count = (target + block - 1) / block;
			g = f.pre(block).sqrt(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word half_norm = T::mod() - (T::mod() - 1) / (2 * length);
			const int levels = __builtin_ctz((uint)length);
			T root_norm = T::raw(T::mod() - (T::mod() - 1) / (1 << (levels / 2)));
			if (levels & 1) root_norm *= T::raw(441024591);
			transform(1).scale(words(inverse), length, half_norm);
			fps_detail::NTT::CompositionWords storage(new (align_val_t(64)) uint[(count - 1) * length]);
			Word *cache = storage.get();
			base error(length);
			base zero(length);
			g.resize(target);
			rep(i, 1, count) {
				int offset = i * block, n = min(block, target - offset);
				Word *q = cache + (i - 1) * length;
				copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, root_norm.val());

				for (int j = 0; j < (i + 1) / 2; j += 6) {
					const Word *a[6], *b[6], *c[6];
					int terms = min(6, (i + 1) / 2 - j);
					rep(k, terms) {
						a[k] = cache + (j + k) * length;
						b[k] = (j + k ? cache + (i - j - k) * length : words(zero));
						c[k] = cache + (i - j - k - 1) * length;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						if (j == 0) {
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error) + block, a, b, c, block, true);
						} else {
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error) + block, a, b, c, block, true);
						}
					};
					if (terms == 6) update(integral_constant<int, 6>{});
					else if (terms == 5) update(integral_constant<int, 5>{});
					else if (terms == 4) update(integral_constant<int, 4>{});
					else if (terms == 3) update(integral_constant<int, 3>{});
					else if (terms == 2) update(integral_constant<int, 2>{});
					else update(integral_constant<int, 1>{});
				}
				fps_detail::NTT::square_fold(words(error), cache + (i / 2) * length, block, i & 1);
				ntt_inverse(error);
				rep(j, block) error[j] = (offset + j < target ? f[offset + j] : T(0)) - error[j];
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				copy_n(error.begin(), n, g.begin() + offset);
			}
			g.insert(g.begin(), shift, T(0));
			return g;
		}
		F h;
		base cache;
		if (sz(g) < target) {
			h = g.pre(16).inv(16);
			cache = h;
			cache.resize(32);
			ntt(cache);
		}
		while (sz(g) < target) {
			const int m = sz(g), n = 2 * m;
			if (target - m <= 32) {
				g.resize(target);
				rep(i, m, target)
					g[i] = (f[i] - dot(g.data() + 1, g.data() + i - 1, i - 1)) * scale;
				break;
			}
			assert(n <= (1 << 23));
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			pointwise(error, a, a, n);
			ntt_inverse(error);
			fill(error.begin(), error.begin() + m, T(0));
			rep(i, m, n) error[i] = (i < target ? f[i] : T(0)) - error[i] * in;
			ntt(error);
			pointwise(error, error, cache, n);
			F::scale_words(error, half);
			ntt_inverse(error);
			g.resize(min(n, target));
			rep(i, m, sz(g)) g[i] = error[i] * in;
		}
		g.resize(target);
		g.insert(g.begin(), shift, T(0));
		return g;
	}

	static T bostan_mori(F p, F q, long long k) {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return T::raw(W::bostan_mori(import_work(p), import_work(q), k).val());
		assert(k >= 0 && !q.empty() && q[0] != T(0));
		p.trim();
		q.trim();
		if (p.empty()) return 0;
		auto coefficient = [](const base &p, const base &q, int k) {
			F inverse(q.begin(), q.begin() + min<int>(k + 1, q.size()));
			inverse = inverse.inv(k + 1);
			return dot(p.data(), inverse.data() + k, min<int>(k + 1, p.size()));
		};
		if (k < 32) return coefficient(p, q, k);
		T answer = 0;
		if (p.size() >= q.size()) {
			auto [quotient, remainder] = p.divmod(q);
			if (k < (long long)quotient.size()) answer = quotient[k];
			p = std::move(remainder);
		}
		if (p.empty()) return answer;
		if (q.size() == 2) return answer + p[0] / q[0] * (-q[1] / q[0]).pow(k);
		const int d = sz(q) - 1;
		if (k < d) return answer + coefficient(p, q, k);
		int n = 1;
		while (n < d) n <<= 1;
		assert(n <= (1 << 22));
		const T half = T(2).inv(), inv = T(n).inv();
		T correction = 1;
		const T step = T(3).pow((T::mod() - 1) / (2 * n));
		base powers(n), twists(n), a = p, b = q;
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			auto &plan = transform(n);
			rep(i, n) words(twists)[i] = plan.inverse_root_quotients[i] * (0u - T::mod());
		} else {
			const auto &roots = inverse_ntt_roots(2 * n);
			rep(i, n) twists[i] = roots[2 * i];
		}
		T lead = q.back();
		fill(all(powers), T(1));
		geometric_scale(powers, inv, step);
		a.resize(2 * n);
		b.resize(2 * n);
		ntt(a);
		ntt(b);
		auto doubling = [&](base &v, T leading) {
			if constexpr (is_same_v<T, fps_detail::Mint>) {
				Word *odd = words(v) + n;
				copy_n(words(v), n, odd);
				auto &plan = transform(max(1, n / 2));
				plan.template fft<true>(odd, n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
				fps_detail::NTT::product(odd, odd, words(powers), n);
				plan.template fft<false>(odd, n);
			} else {
				base odd = v;
				ntt_inverse(odd);
				odd[0] -= T(2 * n) * leading;
				pointwise(odd, odd, powers, n);
				ntt(odd);
				v.insert(v.end(), all(odd));
			}
		};
		while (k) {
			if constexpr (is_same_v<T, fps_detail::Mint>) {
				if (k & 1) fps_detail::NTT::reduce_pairs<true>(words(a), words(b), words(twists), n);
				else fps_detail::NTT::reduce_pairs<false>(words(a), words(b), words(twists), n);
			} else {
				rep(i, n) {
					T u = a[2 * i] * b[2 * i + 1], v = a[2 * i + 1] * b[2 * i];
					a[i] = k & 1 ? (u - v) * twists[i] : u + v;
					b[i] = b[2 * i] * b[2 * i + 1];
				}
			}
			correction *= half;
			if constexpr (!is_same_v<T, fps_detail::Mint>) {
				a.resize(n);
				b.resize(n);
			}
			lead = (d & 1 ? -lead : lead) * lead;
			k >>= 1;
			if (k < d) break;
			doubling(a, T(0));
			doubling(b, d == n ? lead : T(0));
		}
		a.resize(n);
		b.resize(n);
		if (k) {
			intt(a);
			intt(b);
			if (d == n) b[0] -= lead;
			return answer + correction * coefficient(a, b, k);
		}
		T numerator = 0, denominator = 0;
		rep(i, n) numerator += a[i], denominator += b[i];
		if (d == n) denominator -= T(n) * lead;
		return answer + correction * numerator / denominator;
	}

	static T linear_recurrence(const base &initial, const base &coefficients, long long k) {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return T::raw(W::linear_recurrence(import_work(initial), import_work(coefficients), k).val());
		assert(k >= 0 && initial.size() == coefficients.size());
		const int d = sz(initial);
		if (k < d) return initial[k];
		if (d == 0) return 0;
		F q(d + 1);
		q[0] = 1;
		rep(i, d) q[i + 1] = -coefficients[i];
		q.trim();
		if (q.size() == 1) return 0;
		F p = multiply(F(initial), q).pre(d);
		return bostan_mori(std::move(p), std::move(q), k);
	}

	F compose(const F &inner, int deg = -1) const {
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0 && (inner.empty() || inner[0] == T(0)));
		if (deg == 0) return {};
		F f = pre(deg), g = inner.pre(deg);
		f.trim();
		g.trim();
		if (f.size() <= 1 || g.size() <= 1) {
			F result(deg);
			if (!f.empty()) result[0] = f[0];
			return result;
		}
		int first = 1;
		while (g[first] == T(0)) ++first;
		if (first + 1 == sz(g)) {
			F result(deg);
			T power = 1;
			for (int i = 0; i < sz(f) && (long long)i * first < deg; ++i)
				result[i * first] = f[i] * power, power *= g[first];
			return result;
		}
		if (deg <= 32 || f.size() <= 8) {
			F result;
			per(i, sz(f)) {
				result = multiply(result, g).pre(deg);
				result += f[i];
			}
			result.resize(deg);
			return result;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		assert(size <= (1 << 22));
		CompositionNTT transform(size);
		vec<uint> p(2 * size), q(size);
		rep(i, sz(f)) {
			if constexpr (is_same_v<T, fps_detail::Mint>) p[i] = f[i].v;
			else p[i] = transform.in(f[i].val());
		}
		rep(i, sz(g)) {
			if constexpr (is_same_v<T, fps_detail::Mint>) q[i] = g[i].v;
			else q[i] = transform.in(g[i].val());
		}
		uint a = transform.one, b = transform.one;
		transform.compose(p.data(), q.data(), 1, size, a, b);
		F result(deg);
		rep(i, deg) {
			if constexpr (is_same_v<T, fps_detail::Mint>) result[i] = T::word(p[i]);
			else result[i] = T::raw(transform.out(p[i]));
		}
		return result;
	}

	F compositional_inverse(int deg = -1) const {
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0 && (this->empty() || (*this)[0] == T(0)));
		if (deg == 0) return {};
		if (deg == 1) return F(1);
		assert(this->size() >= 2 && (*this)[1] != T(0));
		if constexpr (!is_same_v<T, fps_detail::Mint>)
			return export_work(import_work(*this).compositional_inverse(deg));
		else {
			T w = (*this)[1].inv();
			int length = min<int>(deg, this->size());
			int first = 2;
			while (first < length && (*this)[first] == T(0)) ++first;
			if (first == length) {
				F result(deg);
				result[1] = w;
				return result;
			}
			int size = 1;
			while (size < deg) size <<= 1;
			assert(size <= (1 << 21));

			const int n = deg - 1;
			auto &inv = inverses(size);
			F h(n);
			{
				fps_detail::NTT t(size);
				fps_detail::NTT::CompositionWords storage(static_cast<uint *>(::operator new[](8 * size * sizeof(uint), align_val_t(64))));
				auto *p = storage.get(), *q = p + 4 * size;
				copy_n(words(*this), length, q);
				fill_n(q + length, 2 * size - length, 0u);
				t.geometric(q, length, T(-1).v, w.v);
				auto factor = t.reversion_projection(p, q, size);
				t.product(p + size - n, p + size - n, words(inv) + size - n, n);
				rep(i, n) h[i] = T::word(p[size - 1 - i]);
				scale_words(h, T(size - 1) * T::word(factor));
			}
			h = h.log(n);
			scale_words(h, -inv[size - 1]);
			h = h.exp(n);
			scale_words(h, w);
			F result(deg);
			copy(all(h), result.begin() + 1);
			return result;
		}
	}

	T eval(T x) const {
		T ans = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) ans = ans * x + *it;
		return ans;
	}

	vec<T> eval(const vec<T> &xs) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(W(import_work(*this).eval(import_work(xs))));
		if (xs.empty()) return {};
		if (this->size() <= 32 || xs.size() <= 32) {
			base r;
			r.reserve(xs.size());
			for (T x : xs) r.push_back(eval(x));
			return r;
		}
		constexpr int block = 8;
		int length = block;
		while (length < (int)max(this->size(), xs.size())) length <<= 1;
		assert(length <= (1 << 23));
		int size = length / block;
		vec<base> frequency;
		auto tree = product_tree(xs, size, block, true, &frequency);
		F root = *this;
		root.resize(length);
		reverse(all(root));
		root = root.div_series(tree[1], length);
		return evaluate_tree(xs, tree, frequency, size, block, std::move(root), true);
	}
	vec<T> multipoint_eval(const vec<T> &xs) const { return eval(xs); }

	static F interpolate(const vec<T> &xs, const vec<T> &ys) {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(W::interpolate(import_work(xs), import_work(ys)));
		assert(xs.size() == ys.size());
		const int n = sz(xs);
		if (n == 0) return {};
		constexpr int block = 8;
		int size = 1;
		vec<base> frequency;
		auto tree = product_tree(xs, size, block, true, &frequency);
		const int length = size * block;
		F root(length);
		rep(i, n) root[length - n + i] = T(n - i) * tree[1][i];
		auto flip = [&] {
			rep(i, 1, 2 * size) {
				reverse(all(tree[i]));
				base &a = frequency[i];
				if (a.empty()) continue;
				const int length = sz(a), d = sz(tree[i]) - 1;
				for (int j = 2; j < length; j <<= 1) reverse(a.begin() + j, a.begin() + 2 * j);
				if (d == length / 2) {
					rep(j, length / 2, length) a[j] = -a[j];
				} else {
					const auto &roots = ntt_roots(length);
					base phase(length, T(1));
					for (int half = 1; half < length; half <<= 1) {
						const T step = roots[half].pow(d);
						rep(j, half) phase[half + j] = phase[j] * step;
					}
					rep(j, length) a[j] *= phase[j];
				}
			}
		};
		root = root.div_series(tree[1], length);
		base den = evaluate_tree(xs, tree, frequency, size, block, std::move(root));
		flip();
		vec<F> value(2 * size);
		base weights(n);
		batch_inverse(weights, den);
		pointwise(weights, weights, ys, n);
		for (int i = 0; i * block < n; ++i) {
			const int d = min(block, n - i * block);
			F &f = value[size + i];
			f.resize(d);
			const F &q = tree[size + i];
#if defined(__x86_64__) || defined(__i386__)
			if constexpr (is_same_v<T, fps_detail::Mint>) {
				if (d == 8 && fps_detail::NTT::vectorized()) {
					fps_detail::NTT::interpolate_leaf8(words(f), words(q), words(xs) + i * block, words(weights) + i * block);
					continue;
				}
			}
#endif
			rep(j, i * block, i * block + d) {
				T coefficient = 1;
				f[d - 1] += weights[j];
				per(k, d - 1) {
					coefficient = q[k + 1] + xs[j] * coefficient;
					f[k] += weights[j] * coefficient;
				}
			}
		}
		per(i, 1, size) {
			if (value[2 * i + 1].empty()) {
				value[i] = std::move(value[2 * i]);
				continue;
			}
			const int length = sz(frequency[2 * i]);
			if (length && sz(frequency[2 * i + 1]) == length) {
				base a = std::move(value[2 * i]), b = std::move(value[2 * i + 1]);
				a.resize(length);
				b.resize(length);
				ntt(a);
				ntt(b);
				if constexpr (is_same_v<T, fps_detail::Mint>) fps_detail::NTT::combine(words(a), words(b), words(frequency[2 * i]), words(frequency[2 * i + 1]), length);
				else
					rep(j, length) a[j] = a[j] * frequency[2 * i + 1][j] + b[j] * frequency[2 * i][j];
				intt(a);
				a.resize(tree[i].size() - 1);
				value[i] = std::move(a);
			} else {
				value[i] = value[2 * i] * tree[2 * i + 1] + value[2 * i + 1] * tree[2 * i];
			}
		}
		return std::move(value[1]);
	}

	void onemul(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this *= T(1) + c;
			return;
		}
		per(i, d, sz((*this)))(*this)[i] += (*this)[i - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this /= T(1) + c;
			return;
		}
		rep(i, d, sz((*this)))(*this)[i] -= (*this)[i - d] * c;
	}

	F diff() const {
		const int n = sz((*this));
		F r(n);
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::weighted(words(r), words(*this) + 1, n - 1, 1);
		} else
			rep(i, 1, n) r[i - 1] = (*this)[i] * i;
		return r;
	}
	F integral() const {
		const int n = sz((*this));
		assert(unsigned(n) < unsigned(T::mod()));
		F r(n);
		const auto &inv = inverses(n);
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::product(words(r) + 1, words(*this), words(inv) + 1, n - 1);
		} else
			rep(i, 1, n) r[i] = (*this)[i - 1] * inv[i];
		return r;
	}

	F log(int deg = -1) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).log(deg));
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] == T(1));
		F derivative(min<int>(deg, this->size()));
		if (derivative.size() > 1) fps_detail::NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		const auto &inv = inverses(deg - 1);
		fps_detail::NTT::product(words(r), words(r), words(inv), deg);
		return r;
	}

	F exp(int deg = -1) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).exp(deg));
		if (deg < 0) deg = sz((*this));
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		const auto &iv = inverses(deg);
		auto terms = sparse_terms(deg, 64);
		if (terms.size() <= 64) {
			for (auto &[j, x] : terms) x *= T(j);
			F g(deg);
			g[0] = 1;
			rep(i, 1, deg) {
				for (auto [j, x] : terms) {
					if (j > i) break;
					g[i] += x * g[i - j];
				}
				g[i] *= iv[i];
			}
			return g;
		}
		const int initial = min(deg, 32);
		F g(initial);
		g[0] = 1;
		rep(i, 1, initial) {
			T sum = 0;
			for (int j = 1; j <= i && j < sz((*this)); ++j)
				sum += T(j) * (*this)[j] * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		if (deg > 256) {
			int size = 1;
			while (size < deg) size <<= 1;
			const int block = size / 32, length = 2 * block, count = (deg + block - 1) / block;
			g = pre(block).exp(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word norm = T::mod() - (T::mod() - 1) / length;
			transform(1).scale(words(inverse), length, norm);
			fps_detail::NTT::CompositionWords storage(new (align_val_t(64)) uint[(2 * count - 1) * length]);
			Word *derivative = storage.get(), *cache = derivative + count * length;
			fill(derivative, derivative + block, 0u);
			base error(length);
			const int initial_count = min<int>(block, this->size());
			if (initial_count > 1) fps_detail::NTT::weighted(derivative + 1, words(*this) + 1, initial_count - 1, 1);
			ntt_half(derivative, length);
			g.resize(deg);
			rep(i, 1, count) {
				int offset = i * block, n = min(block, deg - offset);
				Word *d = derivative + i * length;
				const int derivative_count = max(0, min<int>(n, sz((*this)) - offset));
				if (derivative_count) fps_detail::NTT::weighted(d, words(*this) + offset, derivative_count, offset);
				fill(d + derivative_count, d + block, 0u);
				ntt_half(d, length);
				Word *q = cache + (i - 1) * length;
				copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, norm);
				fill(all(error), T(0));
				for (int j = 0; j < i; j += 3) {
					const Word *a[3], *b[3], *c[3];
					int terms = min(3, i - j);
					rep(k, terms) {
						a[k] = cache + (j + k) * length;
						b[k] = derivative + (i - j - k) * length;
						c[k] = derivative + (i - j - k - 1) * length;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						fps_detail::NTT::template accumulate_batch<count>(words(error), a, b, c, block, false);
						fps_detail::NTT::template accumulate_batch<count>(words(error) + block, a, b, c, block, true);
					};
					if (terms == 3) update(integral_constant<int, 3>{});
					else if (terms == 2) update(integral_constant<int, 2>{});
					else update(integral_constant<int, 1>{});
				}
				ntt_inverse(error);
				fill(error.begin() + block, error.end(), T(0));
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				fps_detail::NTT::product(words(error), words(error), words(iv) + offset, n);
				fill(error.begin() + n, error.end(), T(0));
				ntt_half(error);
				fps_detail::NTT::product(words(error), words(error), cache, length);
				ntt_inverse(error);
				copy_n(error.begin(), n, g.begin() + offset);
			}
			return g;
		}
		F h = g.pre(16).inv(16);
		base cache = h;
		cache.resize(32);
		ntt(cache);
		while (sz(g) < deg) {
			const int m = sz(g), n = 2 * m;
			if (deg - m <= 32) {
				base derivative(min<int>(deg, this->size()));
				rep(i, 1, sz(derivative)) derivative[i] = T(i) * (*this)[i];
				derivative.resize(max<int>(1, derivative.size()));
				g.resize(deg);
				rep(i, m, deg) {
					const int count = min<int>(i, derivative.size() - 1);
					g[i] = dot(derivative.data() + 1, g.data() + i - 1, count) * iv[i];
				}
				break;
			}
			assert(n <= (1 << 23));
			const T im = T::raw(T::mod() - (T::mod() - 1) / m);
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, b(m), error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			fill(all(b), T(0));
			for (int i = 1; i < m && i < sz((*this)); ++i) b[i - 1] = T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			rep(i, m - 1) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			per(i, m, min(n, deg))
				error[i] = (i < sz((*this)) ? (*this)[i] : T(0)) + error[i - 1] * in * iv[i];
			fill(error.begin(), error.begin() + m, T(0));
			fill(error.begin() + min(n, deg), error.end(), T(0));
			ntt(error);
			pointwise(error, error, a, n);
			ntt_inverse(error);
			g.resize(min(n, deg));
			rep(i, m, sz(g)) g[i] = error[i] * in;
		}
		return g;
	}

	F pow(long long k) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).pow(k));
		assert(k >= 0);
		const int n = sz((*this));
		if (n == 0) return {};
		if (k == 0) {
			F r(n);
			r[0] = 1;
			return r;
		}
		if (k == 1) return *this;
		if (k <= 4) {
			F square = multiply(*this, *this).pre(n);
			return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
		}
		int first = 0;
		while (first < n && (*this)[first] == T(0)) ++first;
		if (first == n || first > (n - 1) / k) return F(n);
		const int shift = first * k;
		const int len = n - shift;
		const T lead = (*this)[first];
		F f(this->begin() + first, this->end());
		f.resize(len);
		f /= lead;
		f = f.log(len);
		f *= T(k);
		f = f.exp(len);
		f *= lead.pow(k);
		F r(n);
		rep(i, len) r[shift + i] = f[i];
		return r;
	}

	F shift(T c) {
		if constexpr (!is_same_v<T, fps_detail::Mint>) {
			W f = import_work(*this);
			f.shift(fps_detail::Mint::raw(c.val()));
			return *this = export_work(f);
		}
		const int n = sz((*this));
		if (n <= 1 || c == T(0)) return *this;
		assert(unsigned(n) < unsigned(T::mod()));
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			F a(n), b(n), weights;
			Word fact = fps_detail::NTT::one;
			rep(i, n) {
				words(a)[i] = fact;
				fact = (unsigned long long)fact * (i + 1) % T::mod();
			}
			const Word last = T::word(words(a)[n - 1]).inv().v;
			Word invfact = last;
			per(i, n) {
				words(b)[i] = invfact;
				invfact = (unsigned long long)invfact * i % T::mod();
			}
			fps_detail::NTT::product(words(a), words(a), words(*this), n);
			reverse(all(a));
			weights = b;
			geometric_scale(b, T(1), c);
			F prod = multiply(a, b);
			reverse(prod.begin(), prod.begin() + n);
			fps_detail::NTT::product(words(*this), words(prod), words(weights), n);
			return *this;
		}
		const auto &inv = inverses(n);
		F a(n), b(n);
		T fact = 1, invfact = 1, cpow = 1;
		rep(i, n) {
			a[n - 1 - i] = (*this)[i] * fact;
			b[i] = cpow * invfact;
			fact *= i + 1;
			invfact *= inv[i + 1];
			cpow *= c;
		}
		F prod = multiply(a, b);
		invfact = 1;
		rep(i, n) {
			(*this)[i] = prod[n - 1 - i] * invfact;
			invfact *= inv[i + 1];
		}
		return *this;
	}

	pair<F, F> divmod(const F &divisor) const {
		if constexpr (!is_same_v<T, fps_detail::Mint>) {
			auto [q, r] = import_work(*this).divmod(import_work(divisor));
			return {export_work(q), export_work(r)};
		}
		F a = *this;
		a.trim();
		int m = sz(divisor);
		while (m && divisor[m - 1] == T(0)) --m;
		assert(m);
		if (sz(a) < m) return {{}, std::move(a)};
		const int qn = sz(a) - m + 1;
		if (m <= 2 || (size_t)m * qn <= 4096) {
			F q(qn);
			const T inv = divisor[m - 1].inv();
			per(i, qn) {
				q[i] = a[i + m - 1] * inv;
				rep(j, m) a[i + j] -= q[i] * divisor[j];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F ra = a, rb(divisor.begin(), divisor.begin() + m);
		reverse(all(ra));
		reverse(all(rb));
		F q = ra.pre(qn).div_series(rb, qn);
		reverse(all(q));
		reverse(all(rb));
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		rep(i, min<int>(a.size(), product.size())) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

  private:
	static W import_work(const base &a) {
		W r(a.size());
		rep(i, sz(a)) r[i] = fps_detail::Mint::raw(a[i].val());
		return r;
	}
	static F export_work(const W &a) {
		F r(a.size());
		rep(i, sz(a)) r[i] = T::raw(a[i].val());
		return r;
	}
	static fps_detail::NTT &transform(int n) {
		static fps_detail::NTT t(1, false);
		if (sz(t.roots) < n) t = fps_detail::NTT(n, false);
		return t;
	}
	using Word = fps_detail::NTT::U;
	static Word *words(base &a) {
		static_assert(sizeof(T) == sizeof(Word) && is_trivially_copyable_v<T>);
		return reinterpret_cast<Word *>(a.data());
	}
	static const Word *words(const base &a) { return reinterpret_cast<const Word *>(a.data()); }
	template <bool inverse>
	static void transform_inplace(base &a) {
		int n = a.size();
		assert(n && !(n & (n - 1)) && n <= (1 << 23));
		auto &t = transform(max(1, n / 2));
		t.template fft<inverse>(words(a), n);
		if constexpr (!is_same_v<T, fps_detail::Mint>) t.normalize(words(a), n);
	}
	static void ntt(base &a) { transform_inplace<false>(a); }
	static void ntt_half(Word *a, int n) {
		copy_n(a, n / 2, a + n / 2);
		transform(max(1, n / 2)).template fft<false>(a, n, n / 2);
	}
	static void ntt_half(base &a) {
		int n = a.size();
		copy_n(a.begin(), n / 2, a.begin() + n / 2);
		transform(max(1, n / 2)).template fft<false>(words(a), n, n / 2);
	}
	static void ntt_inverse(base &a) { transform_inplace<true>(a); }
	static void scale_words(base &a, T factor) {
		if (a.empty()) return;
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			auto &t = transform(1);
			t.scale(words(a), a.size(), factor.val());
		} else
			for (T &x : a) x *= factor;
	}
	static void geometric_scale(base &a, T first, T step) {
		if constexpr (is_same_v<T, fps_detail::Mint>) fps_detail::NTT::geometric(words(a), a.size(), first.v, step.v);
		else
			for (T &x : a) {
				x *= first;
				first *= step;
			}
	}
	static void batch_inverse(base &dst, const base &src) {
		int n = src.size();
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			if (n >= 128 && fps_detail::NTT::vectorized()) {
				fps_detail::NTT::batch_inverse8(words(dst), words(src), n);
				return;
			}
		}
#endif
		base prefix(n);
		T p = 1;
		rep(i, n) prefix[i] = p, p *= src[i];
		assert(p != T(0));
		p = p.inv();
		per(i, n) {
			dst[i] = prefix[i] * p;
			p *= src[i];
		}
	}
	static void pointwise(base &r, const base &a, const base &b, int n) {
		if constexpr (is_same_v<T, fps_detail::Mint>) fps_detail::NTT::product(words(r), words(a), words(b), n);
		else
			rep(i, n) r[i] = a[i] * b[i];
	}
	vec<pair<int, T>> sparse_terms(int deg, int limit) const {
		vec<pair<int, T>> terms;
		for (int i = 1; i < min<int>(deg, this->size()) && sz(terms) <= limit; ++i)
			if ((*this)[i] != T(0)) terms.emplace_back(i, (*this)[i]);
		return terms;
	}
	static base evaluate_tree(const base &points, const vec<F> &tree,
							  vec<base> &frequency, int size, int block, F root, bool consume = false) {
		base result(points.size());
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			if (block == 8 && fps_detail::NTT::vectorized()) {
				const int total = size * block;
				auto &plan = transform(max(1, total / 2));
				fps_detail::NTT::CompositionWords scratch;
				if (!consume) scratch.reset(new (align_val_t(64)) uint[2 * total]);
				for (int level = 1; level <= size; level <<= 1) {
					const int length = total / level, half = length / 2;
					rep(node, level, 2 * level) {
						const int offset = (node - level) * length;
						if (offset >= sz(points)) break;
						Word *cur = words(root) + offset;
						if (length == 8) {
							fps_detail::NTT::evaluate_leaf8(words(result) + offset, cur, words(tree[node]), tree[node].size(), words(points) + offset, min<int>(8, points.size() - offset));
						} else if (offset + half >= sz(points)) {
							copy_n(cur + half, half, cur);
						} else if (!frequency[2 * node].empty()) {
							Word *a = consume ? words(frequency[2 * node]) : scratch.get();
							Word *b = consume ? words(frequency[2 * node + 1]) : scratch.get() + total;
							if (!consume) {
								copy_n(words(frequency[2 * node]), length, a);
								copy_n(words(frequency[2 * node + 1]), length, b);
							}
							plan.template fft<false>(cur, length);
							fps_detail::NTT::product(a, a, cur, length);
							fps_detail::NTT::product(b, b, cur, length);
							plan.template fft<true>(a, length);
							plan.template fft<true>(b, length);
							const Word norm = T::mod() - (T::mod() - 1) / length;
							fps_detail::NTT::scale_copy(cur, b + half, half, norm);
							fps_detail::NTT::scale_copy(cur + half, a + half, half, norm);
						} else {
							alignas(32) Word left[8], right[8];
							assert(length == 16);
							fps_detail::NTT::evaluate_middle8(left, right, cur, words(tree[2 * node + 1]), tree[2 * node + 1].size(), words(tree[2 * node]), tree[2 * node].size());
							copy_n(left, 8, cur);
							copy_n(right, 8, cur + 8);
						}
					}
				}
				return result;
			}
		}
#endif
		auto dfs = [&](auto &&self, int node, int l, int r, F cur) -> void {
			if (l * block >= sz(points)) return;
			const int length = (r - l) * block, half = length / 2;
			if (r - l == 1 || length <= 8) {
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (is_same_v<T, fps_detail::Mint>) {
					if (length == 8 && fps_detail::NTT::vectorized()) {
						const int count = min<int>(8, points.size() - l * block);
						fps_detail::NTT::evaluate_leaf8(words(result) + l * block, words(cur), words(tree[node]), tree[node].size(), words(points) + l * block, count);
						return;
					}
				}
#endif
				base polynomial(length);
				rep(i, length) for (int j = 0; j + i < length && j < sz(tree[node]); ++j)
					polynomial[length - 1 - i - j] += cur[i] * tree[node][j];
				rep(i, l * block, min<int>(r * block, points.size())) {
					T value = 0;
					per(j, length) value = value * points[i] + polynomial[j];
					result[i] = value;
				}
				return;
			}
			const int mid = (l + r) / 2;
			if (mid * block >= sz(points)) {
				self(self, 2 * node, l, mid, F(cur.begin() + half, cur.end()));
				return;
			}
			F left, right;
			if (!frequency[2 * node].empty()) {
				ntt(static_cast<base &>(cur));
				F a = consume ? std::move(frequency[2 * node]) : frequency[2 * node];
				F b = consume ? std::move(frequency[2 * node + 1]) : frequency[2 * node + 1];
				pointwise(a, a, cur, length);
				pointwise(b, b, cur, length);
				ntt_inverse(a);
				ntt_inverse(b);
				const Word norm = T::mod() - (T::mod() - 1) / length;
				fps_detail::NTT::scale_copy(words(b), words(b) + half, half, norm);
				fps_detail::NTT::scale_copy(words(a), words(a) + half, half, norm);
				b.resize(half);
				a.resize(half);
				left = std::move(b);
				right = std::move(a);
			} else {
				left.resize(half);
				right.resize(half);
				bool done = false;
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (is_same_v<T, fps_detail::Mint>) {
					if (length == 16 && fps_detail::NTT::vectorized()) {
						fps_detail::NTT::evaluate_middle8(words(left), words(right), words(cur), words(tree[2 * node + 1]), tree[2 * node + 1].size(), words(tree[2 * node]), tree[2 * node].size());
						done = true;
					}
				}
#endif
				if (!done) {
					F a = multiply(cur, tree[2 * node + 1]), b = multiply(cur, tree[2 * node]);
					copy_n(a.begin() + half, half, left.begin());
					copy_n(b.begin() + half, half, right.begin());
				}
			}
			self(self, 2 * node, l, mid, std::move(left));
			self(self, 2 * node + 1, mid, r, std::move(right));
		};
		dfs(dfs, 1, 0, size, std::move(root));
		return result;
	}
	static void inverse_extend(F &h, const base &a, const base &cache) {
		const int m = sz(cache);
		const T im = T::raw(T::mod() - (T::mod() - 1) / m);
		base b(m);
		pointwise(b, a, cache, m);
		ntt_inverse(b);
		fill(b.begin(), b.begin() + m / 2, T(0));
		ntt(b);
		pointwise(b, b, cache, m);
		ntt_inverse(b);
		h.resize(m);
		rep(i, m / 2, m) h[i] = -b[i] * im * im;
	}
	static T dot(const T *a, const T *b, int n) {

#if defined(__x86_64__) || defined(__i386__)
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			if (n >= 64 && fps_detail::NTT::vectorized())
				return T::word(fps_detail::NTT::reverse_dot8(reinterpret_cast<const Word *>(a), reinterpret_cast<const Word *>(b), n));
		}
#endif
		uint result = 0;
		for (int i = 0; i < n; i += 16) {
			unsigned long long sum = 0;
			rep(j, i, min(n, i + 16))
				sum += (unsigned long long)a[j].val() * b[-j].val();
			result += sum % T::mod();
			if (result >= T::mod()) result -= T::mod();
		}
		return T::raw(result);
	}

	using CompositionNTT = fps_detail::NTT;
	static void intt(base &a) {
		ntt_inverse(a);
		const T inv = T::raw(T::mod() - (T::mod() - 1) / sz(a));
		scale_words(a, inv);
	}
	static T mod_sqrt(T a) {
		if (a == T(0) || a.pow((T::mod() - 1) / 2) != T(1)) return 0;
		T r = a.pow(60), t = a.pow(119), c = T(3).pow(119);
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T u = t; u != T(1); u *= u) ++i;
			T b = c.pow(1 << (m - i - 1));
			r *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return r;
	}
	static const base &inverse_ntt_roots(int n) {
		static base roots{1};
		while (sz(roots) < n) {
			const int m = sz(roots);
			const T step = T(3).pow((T::mod() - 1) / (2 * m)).inv();
			roots.resize(2 * m);
			rep(i, m) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const base &ntt_roots(int n) {
		static base roots{1};
		while (sz(roots) < n) {
			const int m = sz(roots);
			const T step = T(3).pow((T::mod() - 1) / (2 * m));
			roots.resize(2 * m);
			rep(i, m) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static vec<F> product_tree(const vec<T> &xs, int &size, int block,
							   bool reciprocal,
							   vec<vec<T>> *frequency = nullptr) {
		const int groups = (sz(xs) + block - 1) / block;
		while (size < groups) size <<= 1;
		vec<F> tree(2 * size);
		if (frequency) frequency->resize(2 * size);
		rep(i, size) {
			if (i >= groups) {
				tree[size + i] = {1};
				continue;
			}
			F p;
			p.reserve(block + 1);
			p.push_back(T(1));
			rep(j, i * block, min<int>((i + 1) * block, xs.size())) {
				const int d = sz(p) - 1;
				if (reciprocal) {
					p.push_back(0);
					per(k, 1, d + 2) p[k] -= xs[j] * p[k - 1];
				} else {
					p.push_back(p.back());
					per(k, 1, d + 1) p[k] = p[k - 1] - xs[j] * p[k];
					p[0] *= -xs[j];
				}
			}
			tree[size + i] = std::move(p);
		}
		per(i, 1, size) {
			if (tree[2 * i + 1].size() == 1 && tree[2 * i + 1][0] == T(1)) {
				tree[i] = tree[2 * i];
				continue;
			}
			int depth = 31 - __builtin_clz((uint)i);
			int length = (size >> depth) * block;
			if (frequency && length >= 32) {
				auto &a = (*frequency)[2 * i];
				auto &b = (*frequency)[2 * i + 1];
				auto transform = [&](base &a, const F &p) {
					if (a.empty()) {
						a.resize(length);
						copy(all(p), a.begin());
						ntt(a);
					} else {
						const int half = length / 2;
						base odd(half);
						copy_n(p.begin(), min<int>(half, p.size()), odd.begin());
						if (sz(p) > half) odd[0] -= p[half];
						geometric_scale(odd, T(1), ntt_roots(length)[length / 2]);
						ntt(odd);
						a.insert(a.end(), all(odd));
					}
				};
				transform(a, tree[2 * i]);
				transform(b, tree[2 * i + 1]);
				vec<T> c(length);
				pointwise(c, a, b, length);
				if (i > 1) (*frequency)[i] = c;
				ntt_inverse(c);
				const int n = (int)(tree[2 * i].size() + tree[2 * i + 1].size() - 1);
				tree[i].resize(n);
				const T inv = T::raw(T::mod() - (T::mod() - 1) / length);
				if constexpr (is_same_v<T, fps_detail::Mint>) fps_detail::NTT::scale_copy(words(tree[i]), words(c), min(n, length), inv.val());
				else
					rep(j, min(n, length)) tree[i][j] = c[j] * inv;
				if (n == length + 1) {
					const T lead = tree[2 * i].back() * tree[2 * i + 1].back();
					tree[i][0] -= lead;
					tree[i][length] = lead;
				}
			} else {
				tree[i] = multiply(tree[2 * i], tree[2 * i + 1]);
			}
		}
		return tree;
	}
	void trim() {
		while (!this->empty() && this->back() == T(0)) this->pop_back();
	}
	static const vec<T> &inverses(int n) {
		assert(unsigned(n) < unsigned(T::mod()));
		static vec<T> inv{0, 1};
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (is_same_v<T, fps_detail::Mint>) {
			int first = inv.size(), count = (n + 1 - first) / 8 * 8;
			if (count >= 256 && fps_detail::NTT::vectorized()) {
				inv.reserve(n + 1);
				inv.resize(first + count);
				fps_detail::NTT::inverse_sequence8(reinterpret_cast<Word *>(inv.data()) + first, first, count);
			}
		}
#endif
		while (sz(inv) <= n) {
			const int i = sz(inv);
			inv.push_back(-inv[T::mod() % i] * (T::mod() / i));
		}
		return inv;
	}
};

using fps = FormalPowerSeries<fps_mint>;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
