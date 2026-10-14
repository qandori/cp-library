#include "data_structure/weighted_union_find.hpp"
#include "data_structure/weighted_union_find.hpp"
#include "math/mint.hpp"

int main() {
	WeightedUnionFind<> uf(6);
	assert(uf.merge(0, 1, 3));
	assert(uf.merge(1, 2, -5));
	assert(uf.diff(0, 2) == -2 && uf.diff(2, 0) == 2);
	assert(uf.merge(0, 2, -2));
	assert(!uf.merge(0, 2, 0));
	assert(uf.diff(0, 2) == -2 && uf.size(1) == 3);
	assert(uf.merge(3, 4, 7));
	assert(uf.merge(3, 0, -9));
	assert(uf.diff(3, 2) == -11 && uf.diff(4, 0) == -16);
	assert(uf.size(4) == 5 && !uf.same(0, 5));
	assert(uf.merge(5, 5, 0) && !uf.merge(5, 5, 1));
	WeightedUnionFind<mint> modular(3);
	assert(modular.merge(0, 1, mint(-1)));
	assert(modular.merge(1, 2, mint(2)));
	assert(modular.diff(0, 2) == mint(1));
	assert(modular.merge(2, 0, mint(-1)));
	assert(!modular.merge(2, 0, mint(1)));
	WeightedUnionFind<__int128_t> wide(3);
	__int128_t large = __int128_t(1) << 100;
	assert(wide.merge(0, 1, large) && wide.merge(1, 2, large));
	assert(wide.diff(0, 2) == large * 2);
	mt19937 random(123);
	rep(200) {
		int n = 1 + random() % 25;
		WeightedUnionFind<> current(n);
		vec<vec<pair<int, int64_t>>> g(n);
		rep(300) {
			int x = random() % n, y = random() % n;
			int64_t w = int(random() % 2001) - 1000;
			vec<int> seen(n), todo{x};
			vec<int64_t> distance(n);
			seen[x] = 1;
			rep(i, sz(todo)) {
				int v = todo[i];
				for (auto [to, cost] : g[v]) if (!seen[to]) {
					seen[to] = 1;
					distance[to] = distance[v] + cost;
					todo.push_back(to);
				}
			}
			rep(v, n) {
				assert(current.same(x, v) == bool(seen[v]));
				if (seen[v]) assert(current.diff(x, v) == distance[v]);
			}
			assert(current.size(x) == sz(todo));
			bool valid = !seen[y] || distance[y] == w;
			assert(current.merge(x, y, w) == valid);
			if (valid && !seen[y]) {
				g[x].emplace_back(y, w);
				g[y].emplace_back(x, -w);
			}
		}
	}
	WeightedUnionFind<> chain(100000);
	rep(i, 99999) assert(chain.merge(i, i + 1, 1));
	assert(chain.diff(0, 99999) == 99999 && chain.size(99999) == 100000);
}
