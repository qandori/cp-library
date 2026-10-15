#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"
#include "../data_structure/li_chao_tree.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

void print_integer(__int128_t value) {
	__uint128_t x = value;
	if (value < 0) cout << '-', x = -x;
	string digits;
	do {
		digits.push_back('0' + x % 10);
		x /= 10;
	} while (x);
	reverse(digits.begin(), digits.end());
	cout << digits << '\n';
}

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	struct Segment {
		int64_t l, r, a, b;
	};
	vector<Segment> initial(n);
	for (auto &[l, r, a, b] : initial) cin >> l >> r >> a >> b;
	struct Query {
		int type, index;
		Segment f;
	};
	vector<Query> queries(q);
	vector<int64_t> xs;
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
			if (value) print_integer(*value);
			else cout << "INFINITY\n";
		}
	}
}
