#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "../graph/dijkstra.hpp"

int main() {
    cin.tie(nullptr)->sync_with_stdio(false);
    int n, m, s, t;
    cin >> n >> m >> s >> t;
    vec<vec<pair<int, int64_t>>> g(n);
    rep(m) {
        int u, v;
        int64_t c;
        cin >> u >> v >> c;
        g[u].emplace_back(v, c);
    }
    auto d = dijkstra(g, s);
    if (d[t] == -1) {
        cout << -1 << '\n';
        return 0;
    }
    vec<int> parent(n, -1), todo{s};
    parent[s] = s;
    rep(i, sz(todo)) {
        int v = todo[i];
        for (auto [to, c] : g[v])
            if (parent[to] == -1 && d[to] == d[v] + c) {
                parent[to] = v;
                todo.push_back(to);
            }
    }
    vec<pair<int, int>> path;
    for (int v = t; v != s; v = parent[v]) path.emplace_back(parent[v], v);
    reverse(all(path));
    cout << d[t] << ' ' << sz(path) << '\n';
    for (auto [u, v] : path) cout << u << ' ' << v << '\n';
}
