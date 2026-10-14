#define PROBLEM "https://judge.yosupo.jp/problem/log_of_formal_power_series"
#include "../template.hpp"
#include "../math/fps.hpp"

int main() {
    cin.tie(nullptr)->sync_with_stdio(false);
    int n;
    cin >> n;
    fps f(n);
    for (auto &v : f) cin >> v;
    cout << f.log(n) << '\n';
}
