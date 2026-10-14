#include "math/mint.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

int main() {
    constexpr int64_t mod = 998244353;
    auto normalize = [](int64_t x) { x %= mod; return x < 0 ? x + mod : x; };
    for (int64_t x : {int64_t(0), int64_t(-1), mod, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()})
        assert(mint(x).val() == normalize(x));
    std::mt19937_64 random(123);
    for (int i = 0; i < 10000; ++i) {
        int64_t a = random() % mod, b = random() % mod;
        assert((mint(a) + mint(b)).val() == (a + b) % mod);
        assert((mint(a) - mint(b)).val() == normalize(a - b));
        assert((mint(a) * mint(b)).val() == a * b % mod);
        if (b) assert((mint(a) / mint(b) * mint(b)) == mint(a));
        assert(mint(a).pow(0) == mint(1));
        assert(mint(a).pow(3) == mint(a) * mint(a) * mint(a));
    }
}
