#include "math/sqrt_floor.hpp"
#include "math/sqrt_floor.hpp"

void check(long n, long m, long a, long b) {
    auto [dx, dy, segments] = sqrt_floor(n, m, a, b);
    if (!n) {
        assert(dx == 0 && dy == 0 && segments.empty());
        return;
    }
    vec<int> seen(n);
    for (auto [x, y, len] : segments) {
        assert(len > 0);
        for (long j = 0; j < len; ++j) {
            long i = x + dx * j;
            assert(0 <= i && i < n && ++seen[i] == 1);
            assert(y + dy * j == (a * i + b) % m);
        }
    }
    for (int count : seen) assert(count == 1);
}

int main() {
    rep(n, 21) rep(m, 1, 20) rep(a, m) rep(b, m) check(n, m, a, b);
    mt19937_64 random(123);
    rep(1000) check(random() % 1000, 1 + random() % 1000000000, random() % 1000000000, random() % 1000000000);
}
