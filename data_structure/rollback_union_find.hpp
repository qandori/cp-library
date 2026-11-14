#ifndef ROLLBACK_UNION_FIND_HPP
#define ROLLBACK_UNION_FIND_HPP

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class RollbackUnionFind {
	struct Change { int child, size; };
	std::vector<int> parent_;
	std::vector<Change> history_;
	int components_ = 0;

  public:
	explicit RollbackUnionFind(int n = 0) { reset(n); }
	void reset(int n) {
		assert(n >= 0);
		parent_.assign(n, -1), history_.clear();
		components_ = n;
	}
	void reserve(std::size_t updates) { history_.reserve(updates); }
	int vertex_count() const { return static_cast<int>(parent_.size()); }
	int component_count() const { return components_; }
	int size() const { return vertex_count(); }
	int count() const { return component_count(); }
	int leader(int x) const {
		assert(0 <= x && x < vertex_count());
		while (parent_[x] >= 0) x = parent_[x];
		return x;
	}
	bool same(int x, int y) const { return leader(x) == leader(y); }
	int size(int x) const { return -parent_[leader(x)]; }
	bool merge(int x, int y) {
		x = leader(x), y = leader(y);
		if (x == y) {
			history_.push_back({-1, 0});
			return false;
		}
		if (parent_[x] > parent_[y]) std::swap(x, y);
		history_.push_back({y, parent_[y]});
		parent_[x] += parent_[y], parent_[y] = x;
		--components_;
		return true;
	}
	std::size_t snapshot() const { return history_.size(); }
	void undo() {
		assert(!history_.empty());
		auto [child, size] = history_.back();
		history_.pop_back();
		if (child < 0) return;
		parent_[parent_[child]] -= size, parent_[child] = size;
		++components_;
	}
	void rollback(std::size_t state = 0) {
		assert(state <= history_.size());
		while (history_.size() > state) undo();
	}
	std::vector<std::vector<int>> groups() const {
		std::vector<std::vector<int>> result;
		result.reserve(components_);
		std::vector<int> index(vertex_count(), -1);
		for (int v = 0; v < vertex_count(); ++v) {
			int r = leader(v);
			if (index[r] == -1) {
				index[r] = static_cast<int>(result.size());
				result.emplace_back();
				result.back().reserve(-parent_[r]);
			}
			result[index[r]].push_back(v);
		}
		return result;
	}
};

#endif
