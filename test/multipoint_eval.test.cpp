#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "../template.hpp"
#include "../math/fps.hpp"

int main() {
    cin.tie(nullptr)->sync_with_stdio(false);
    int n, m;
    cin >> n >> m;
    fps f(n);
    vec<mint> xs(m);
    for (auto &v : f) cin >> v;
    cin >> xs;
    cout << f.multipoint_eval(xs) << '\n';
}
