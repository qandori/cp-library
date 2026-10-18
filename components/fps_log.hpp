#ifndef CP_COMPONENTS_FPS_LOG_HPP
#define CP_COMPONENTS_FPS_LOG_HPP

#include "ntt.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {

#if defined(__x86_64__) || defined(__i386__)
struct LogTransform {
	using U = NTT::U;
	using I8 = NTT::I8;
	static constexpr U mod = NTT::mod, twice = NTT::twice, one = NTT::one;
	const NTT &plan;

	__attribute__((target("avx2"))) void forward(U *a, int n, U norm = 1, bool half = false) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		if (half) std::copy_n(a, n / 2, a + n / 2);
		plan.template inverse_partial8<false>(a, n, half ? n / 2 : n);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 r = _mm256_set1_epi32(norm), rq = _mm256_set1_epi32(NTT::quotient(norm));
		for (int i = 0; i < n; i += 8) {
			I8 x = NTT::load8(a + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			if (norm != 1) x = NTT::fixed_mul8(x, r, rq);
			NTT::store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
	}
	__attribute__((target("avx2"))) void inverse(U *a, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		plan.template inverse_partial8<true>(a, n);
	}
	template <bool upper> __attribute__((target("avx2"))) void inverse_half(U *a, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		int h = 8;
		for (; 4 * h < n; h <<= 2) plan.template radix4_8<true>(a, h, n);
		if (2 * h == n) {
			for (int i = 0; i < h; i += 8) {
				I8 x = upper ? NTT::sub8(NTT::load8(a + i), NTT::load8(a + h + i)) : NTT::add8(NTT::load8(a + i), NTT::load8(a + h + i));
				NTT::store8(a + (upper ? h : 0) + i, x);
			}
		} else {
			I8 im = _mm256_set1_epi32(plan.inverse_roots[1]), iq = _mm256_set1_epi32(plan.inverse_root_quotients[1]);
			for (int i = 0; i < h; i += 8) {
				I8 x0 = NTT::load8(a + i), x1 = NTT::load8(a + h + i), x2 = NTT::load8(a + 2 * h + i), x3 = NTT::load8(a + 3 * h + i);
				I8 a0 = NTT::add8(x0, x1), a1 = NTT::sub8(x0, x1), a2 = NTT::add8(x2, x3), a3 = NTT::fixed_mul8(NTT::sub8(x2, x3), im, iq);
				NTT::store8(a + (upper ? 2 * h : 0) + i, upper ? NTT::sub8(a0, a2) : NTT::add8(a0, a2));
				NTT::store8(a + (upper ? 3 * h : h) + i, upper ? NTT::sub8(a1, a3) : NTT::add8(a1, a3));
			}
		}
	}
	__attribute__((target("avx2"))) void products(U *dst, const U *const *a, const U *const *b, int terms, int n, const U *side = nullptr) const {
		assert(n >= 64 && n % 64 == 0 && terms >= 1 && terms <= 2 && static_cast<int>(plan.roots.size()) >= n / 16);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = k & 1 ? mod - plan.roots[k / 2] : plan.roots[k / 2];
				qs[j] = k & 1 ? ~plan.root_quotients[k / 2] : plan.root_quotients[k / 2];
			}
			alignas(32) U x[64], sum[64];
			for (int t = 0; t < terms; ++t) {
				std::copy_n(a[t] + i, 64, x);
				plan.template inverse_leaf_products8<8, true>(x, const_cast<U *>(b[t]) + i, ws, qs);
				for (int j = 0; j < 64; j += 8) NTT::store8(sum + j, t ? NTT::add8(NTT::load8(sum + j), NTT::load8(x + j)) : NTT::load8(x + j));
			}
			for (int j = 0; j < 64; j += 8)
				NTT::store8(dst + i + j, side ? NTT::add8(NTT::load8(sum + j), NTT::load8(side + i + j)) : NTT::load8(sum + j));
		}
	}

	__attribute__((target("avx2"))) static void integrate(U *dst, const U *src, U *prefix, int first, int n) {
		assert(n >= 0 && first >= 1 && static_cast<unsigned long long>(first) + n <= mod);
		if (!n) return;
		I8 x[4], prod[4], inv[4], step = _mm256_set1_epi32(NTT::in(32)), onev = _mm256_set1_epi32(one);
		alignas(32) U initial[32];
		for (int i = 0; i < 32; ++i) initial[i] = NTT::in(first + i);
		const I8 lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), limit = _mm256_set1_epi32(n);
		auto mask = [&](int at) __attribute__((target("avx2"))) { return _mm256_cmpgt_epi32(limit, _mm256_add_epi32(_mm256_set1_epi32(at), lanes)); };
		auto factor = [&](I8 a, int at) __attribute__((target("avx2"))) { return at + 8 <= n ? a : _mm256_blendv_epi8(onev, a, mask(at)); };
		for (int v = 0; v < 4; ++v) x[v] = NTT::load8(initial + 8 * v), prod[v] = factor(x[v], 8 * v);
		for (int i = 32; i < n; i += 32)
			for (int v = 0; v < 4; ++v) {
				NTT::store8(prefix + i - 32 + 8 * v, prod[v]);
				x[v] = NTT::add8(x[v], step);
				prod[v] = NTT::mul8(prod[v], factor(x[v], i + 8 * v));
			}
		for (int v = 0; v < 4; ++v) inv[v] = NTT::inverse8(prod[v]);
		auto put = [&](int at, I8 reciprocal) __attribute__((target("avx2"))) {
			if (at >= n) return;
			I8 a = at + 8 <= n ? NTT::load8(src + at) : _mm256_maskload_epi32(reinterpret_cast<const int *>(src + at), mask(at));
			I8 r = NTT::mul8(a, reciprocal);
			if (at + 8 <= n)
				NTT::store8(dst + at, r);
			else
				_mm256_maskstore_epi32(reinterpret_cast<int *>(dst + at), mask(at), r);
		};
		for (int i = (n - 1) / 32 * 32; i >= 32; i -= 32)
			for (int v = 0; v < 4; ++v) {
				int at = i + 8 * v;
				if (at < n) put(at, NTT::mul8(inv[v], NTT::load8(prefix + at - 32)));
				inv[v] = NTT::mul8(inv[v], factor(x[v], at));
				x[v] = NTT::sub8(x[v], step);
			}
		for (int v = 0; v < 4; ++v) put(8 * v, inv[v]);
	}
	__attribute__((target("avx2"))) void toeplitz(U *dst, U *side, const U *w1, const U *w2, const U *q0, const U *q1, int n) const {
		assert(n >= 64 && n % 64 == 0 && static_cast<int>(plan.roots.size()) >= n / 16);
		I8 p = _mm256_set1_epi32(mod);
		auto canonical = [&](I8 x) __attribute__((target("avx2"))) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, p)); };
		for (int i = 0; i < n; i += 64) {
			alignas(32) U x[64], y[64], z[64];
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = k & 1 ? mod - plan.roots[k / 2] : plan.roots[k / 2];
				qs[j] = k & 1 ? ~plan.root_quotients[k / 2] : plan.root_quotients[k / 2];
				NTT::store8(x + 8 * j, canonical(_mm256_add_epi32(NTT::load8(q0 + i + 8 * j), NTT::load8(q1 + i + 8 * j))));
				NTT::store8(y + 8 * j, canonical(NTT::sub8(NTT::load8(w1 + i + 8 * j), NTT::load8(w2 + i + 8 * j))));
				NTT::store8(z + 8 * j, canonical(NTT::sub8(NTT::load8(side + i + 8 * j), NTT::load8(w2 + i + 8 * j))));
			}
			plan.template inverse_leaf_products8<8, true>(x, const_cast<U *>(w2) + i, ws, qs);
			plan.template inverse_leaf_products8<8, true>(y, const_cast<U *>(q1) + i, ws, qs);
			plan.template inverse_leaf_products8<8, true>(z, const_cast<U *>(q0) + i, ws, qs);
			for (int j = 0; j < 64; j += 8) {
				NTT::store8(dst + i + j, NTT::add8(NTT::load8(x + j), NTT::load8(y + j)));
				NTT::store8(side + i + j, NTT::add8(NTT::load8(x + j), NTT::load8(z + j)));
			}
		}
	}
};
#endif

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
