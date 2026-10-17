#include "math/mod_sqrt.hpp"
#include "test/benchmark/mod_sqrt_baseline.hpp"
#include "components/fast_io.hpp"

template <class M>
std::optional<std::uint64_t> cipolla(std::uint64_t a, const M &m) {
	using U = typename M::U;
	U p = m.mod, value = m.in(U(a));
	if (m.pow(value, (p - 1) / 2) != m.one) return std::nullopt;
	SplitMix64 random(a ^ p ^ 0x9e3779b97f4a7c15ULL);
	U t, w;
	for (;;) {
		t = m.in(U(random() % p));
		w = m.mul(t, t);
		w = w >= value ? w - value : w + (p - value);
		if (!w) {
			std::uint64_t root = m.reduce(t);
			return std::min(root, m.mod - root);
		}
		if (m.pow(w, (p - 1) / 2) == p - m.one) break;
	}
	U x = m.one, y = 0;
	U exponent = (p >> 1) + 1;
	for (int i = std::bit_width(exponent); i--;) {
		U nx = m.add(m.mul(x, x), m.mul(w, m.mul(y, y)));
		U ny = m.mul(x, y);
		x = nx; y = m.add(ny, ny);
		if ((exponent >> i) & 1) {
			nx = m.add(m.mul(x, t), m.mul(y, w));
			ny = m.add(x, m.mul(y, t));
			x = nx; y = ny;
		}
	}
	std::uint64_t root = m.reduce(x);
	return std::min(root, m.mod - root);
}

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t a = 0, p = 0;
		input.read(a), input.read(p);
		std::optional<std::uint64_t> root;
		if (p == 2 || (a %= p) <= 1) root = p == 2 ? a & 1 : a;
#if MOD_SQRT_BENCH_VARIANT == 1
		else root = mod_sqrt_baseline_detail::sqrt(a, Montgomery64(p));
#elif MOD_SQRT_BENCH_VARIANT == 2
		else if (p <= std::numeric_limits<std::uint32_t>::max()) root = cipolla(a, Montgomery32(std::uint32_t(p)));
		else root = cipolla(a, Montgomery64(p));
#else
		else root = mod_sqrt(a, p);
#endif
		if (root) output.println(*root);
		else output.println(-1);
	}
}
