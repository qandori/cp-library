#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"
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
	vector<pair<int64_t, int64_t>> initial(n);
	for (auto &[a, b] : initial) cin >> a >> b;
	struct Query {
		int type, index;
		int64_t a, b;
	};
	vector<Query> queries(q);
	vector<int64_t> xs;
	for (auto &query : queries) {
		cin >> query.type >> query.a;
		if (query.type == 0) cin >> query.b;
		else xs.push_back(query.a);
	}
	LiChaoTree<> tree(std::move(xs));
	for (auto &query : queries)
		if (query.type == 1) query.index = tree.index(query.a);
	for (auto [a, b] : initial) tree.add_line(a, b);
	for (auto query : queries) {
		if (query.type == 0) tree.add_line(query.a, query.b);
		else print_integer(*tree.query_index(query.index));
	}
}
