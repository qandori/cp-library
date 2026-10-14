#include "template.hpp"
#include "template.hpp"

int main() {
    int sum = 0;
    rep(4) ++sum;
    rep(i, 4) sum += i;
    rep(i, 2, 5) sum += i;
    assert(sum == 19);
    sum = 0;
    per(4) ++sum;
    per(i, 4) sum += i;
    per(i, 2, 5) sum += i;
    assert(sum == 19);
    int x = 10;
    assert(chmin(x, 3) && x == 3);
    assert(!chmin(x, 4));
    assert(chmax(x, 7) && x == 7);
    vec<int> v{3, 1, 1};
    UNIQUE(v);
    assert((v == vec<int>{1, 3}));
    ++v; --v; v++; v--;
    assert((v == vec<int>{1, 3}));
    rep(x, 2) rep(y, 2) {
        vec<bool> bits(130, true);
        bits[64] = x;
        bits[64] ^= bool(y);
        assert(bits[64] == bool(x ^ y));
        bits[64] = x;
        bits[64] &= bool(y);
        assert(bits[64] == bool(x & y));
        bits[64] = x;
        bits[64] |= bool(y);
        assert(bits[64] == bool(x | y));
        rep(i, sz(bits)) if (i != 64) assert(bits[i]);
    }
    vec<bool> bits{false, true};
    (bits[0] ^= bits[1]) ^= true;
    assert(!bits[0] && bits[1]);
    auto bit = bits[0];
    bit |= true;
    bit ^= bit;
    assert(!bits[0]);
    int index = 0;
    bits[index++] ^= true;
    assert(index == 1 && bits[0]);
    __int128_t minimum = -(__int128_t(1) << 126);
    minimum += minimum;
    ostringstream out;
    out << minimum;
    assert(out.str() == "-170141183460469231731687303715884105728");
}
