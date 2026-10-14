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
    __int128_t minimum = -(__int128_t(1) << 126);
    minimum += minimum;
    ostringstream out;
    out << minimum;
    assert(out.str() == "-170141183460469231731687303715884105728");
}
