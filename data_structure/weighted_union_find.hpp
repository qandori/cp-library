#ifndef WEIGHTED_UNION_FIND_HPP
#define WEIGHTED_UNION_FIND_HPP

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

template <class T = std::int64_t>
struct WeightedUnionFind {
  private:
	int count_ = 0;
  public:
	std::vector<int> parent;
	std::vector<T> delta;
	explicit WeightedUnionFind(int n) : count_(n) {
		assert(n >= 0);
		parent.assign(n, -1);
		delta.resize(n);
	}
	int size() const { return static_cast<int>(parent.size()); }
	int vertex_count() const { return size(); }
	int count() const { return count_; }
	int component_count() const { return count(); }
	int leader(int x) {
		assert(0 <= x && x < static_cast<int>(parent.size()));
		int p = parent[x];
		if (p < 0) return x;
		if (parent[p] < 0) return p;
		int r = leader(p);
		delta[x] += delta[parent[x]];
		return parent[x] = r;
	}
	T weight(int x) {
		leader(x);
		return delta[x];
	}
	bool same(int x, int y) { return leader(x) == leader(y); }
	int size(int x) { return -parent[leader(x)]; }
	T diff(int x, int y) {
		[[maybe_unused]] int rx = leader(x), ry = leader(y);
		assert(rx == ry);
		return delta[y] - delta[x];
	}
	bool merge(int x, int y, T w) {
		int rx = leader(x), ry = leader(y);
		w += delta[x] - delta[y];
		if (rx == ry) return w == T(0);
		if (parent[rx] > parent[ry]) std::swap(rx, ry), w = -w;
		parent[rx] += parent[ry];
		parent[ry] = rx;
		delta[ry] = w;
		--count_;
		return true;
	}
};

#endif
