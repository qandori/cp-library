#include "template.hpp"
#include "math/fps.hpp"
#include "math/fps.hpp"

int main() {
    mt19937 random(123);
    for (int n : {1, 2, 3, 8, 31, 32, 63, 64, 127, 128, 257}) {
        fps a(n), b(n), expected(2 * n - 1);
        rep(i, n) a[i] = random(), b[i] = random();
        rep(i, n) rep(j, n) expected[i + j] += a[i] * b[j];
        assert(a * b == expected);
        a[0] = 1;
        fps inverse = a.inv(n), product = a * inverse;
        assert(product[0] == mint(1));
        rep(i, 1, n) assert(product[i] == mint(0));
        assert(a.log(n).exp(n) == a);
        vec<mint> xs(n);
        rep(i, n) xs[i] = random();
        auto values = a.multipoint_eval(xs);
        rep(i, n) {
            mint value = 0;
            per(j, n) value = value * xs[i] + a[j];
            assert(values[i] == value);
        }
    }
}
