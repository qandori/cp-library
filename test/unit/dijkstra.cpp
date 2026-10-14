#include "graph/dijkstra.hpp"
#include "graph/dijkstra.hpp"

int main() {
    using I = int64_t;
    using G = vec<vec<pair<int, I>>>;
    mt19937_64 random(123);
    rep(trial, 500) {
        int n = 1 + random() % 20, edges = random() % (n * n + 1);
        I inf = trial % 2 ? numeric_limits<I>::max() : DIJKSTRA_INF;
        G g(n);
        vec<vec<I>> expected(n, vec<I>(n, inf));
        rep(i, n) expected[i][i] = 0;
        rep(edges) {
            int v = random() % n, to = random() % n;
            I c = trial % 3 == 0 ? random() % 2 : trial % 3 == 1 ? random() % 1000 : random() >> 1;
            g[v].emplace_back(to, c);
            chmin(expected[v][to], c);
        }
        rep(k, n) rep(i, n) rep(j, n) {
            __int128_t sum = __int128_t(expected[i][k]) + expected[k][j];
            if (sum < expected[i][j]) expected[i][j] = I(sum);
        }
        rep(st, n) {
            for (I &v : expected[st]) if (v == inf) v = -1;
            assert(dijkstra(g, st, inf) == expected[st]);
        }
    }
    const vec<vec<pair<int, int>>> g = {{{1, 7}}, {}, {}};
    assert((dijkstra(g, 0) == vec<int>{0, 7, -1}));
    const vec<vec<pair<int, double>>> f = {{{1, 1.5}}, {{2, 2.25}}, {}, {}};
    assert((dijkstra(f, 0, 100.) == vec<double>{0, 1.5, 3.75, -1}));
    I maximum = numeric_limits<I>::max();
    const G huge = {{{1, maximum - 1}, {2, maximum}}, {{2, maximum}}, {}};
    assert((dijkstra(huge, 0, maximum) == vec<I>{0, maximum - 1, -1}));
}
