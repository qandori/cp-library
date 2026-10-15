#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"
#include "../data_structure/li_chao_tree.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	struct Segment {
		int64_t l, r, a, b;
	};
	vec<Segment> initial(n);
	for (auto &[l, r, a, b] : initial) cin >> l >> r >> a >> b;
	struct Query {
		int type, index;
		Segment f;
	};
	vec<Query> queries(q);
	vec<int64_t> xs;
	for (auto &query : queries) {
		cin >> query.type >> query.f.l;
		if (query.type == 0) cin >> query.f.r >> query.f.a >> query.f.b;
		else xs.push_back(query.f.l);
	}
	LiChaoTree<> tree(std::move(xs));
	for (auto &query : queries)
		if (query.type == 1) query.index = tree.index(query.f.l);
	for (auto [l, r, a, b] : initial) tree.add_segment(l, r, a, b);
	for (auto query : queries) {
		if (query.type == 0) tree.add_segment(query.f.l, query.f.r, query.f.a, query.f.b);
		else {
			auto value = tree.query_index(query.index);
			if (value) cout << *value << '\n';
			else cout << "INFINITY\n";
		}
	}
}
