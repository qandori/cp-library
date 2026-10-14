#define COMB_MAX 200
#include "math/comb.hpp"
#include "math/comb.hpp"

static_assert(is_array_v<decltype(fact)> && extent_v<decltype(fact)> == 201);

int main() {
    init_comb();
    mint row[201]{};
    row[0] = 1;
    rep(n, 201) {
        rep(k, n + 1) assert(comb(n, k) == row[k]);
        assert(fact[n] * finv[n] == mint(1));
        assert(comb(n, -1) == mint(0) && comb(n, n + 1) == mint(0));
        per(k, 1, min(n + 2, 201)) row[k] += row[k - 1];
    }
    assert(comb(-1, 0) == mint(0));
}
