#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../template.hpp"
#include "../math/fps.hpp"

int main() {
    cin.tie(nullptr)->sync_with_stdio(false);
    int n, m;
    cin >> n >> m;
    fps a(n), b(m);
    for (auto &v : a) cin >> v;
    for (auto &v : b) cin >> v;
    cout << a * b << '\n';
}
