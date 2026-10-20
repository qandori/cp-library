#ifndef DSU_ON_TREE_HPP
#define DSU_ON_TREE_HPP

#include "../components/undirected_csr.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <span>
#include <utility>
#include <vector>

class DSUOnTree {
	int n_, root_ = -1;
	bool built_ = false;
	std::vector<std::pair<int, int>> edges_;
	std::vector<int> parent_, heavy_, in_, out_, order_;
	void check(int v) const {
		assert(built_ && 0 <= v && v < n_);
		(void)v;
	}
	template<class Add, class Answer, class Clear>
	void execute(Add& add, Answer& answer, Clear& clear, bool keep) const {
		assert(built_);
		for (int i = n_; i-- > 0;) {
			int v = order_[i], h = heavy_[v];
			for (int j = h == -1 ? i + 1 : out_[h]; j < out_[v]; ++j) add(order_[j]);
			add(v);
			answer(v);
			if (v == root_ ? !keep : heavy_[parent_[v]] != v) clear(v);
		}
	}

  public:
	explicit DSUOnTree(int n = 0) : n_(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges_.reserve(std::max(0, n - 1));
	}
	int size() const { return n_; }
	bool empty() const { return n_ == 0; }
	int edge_count() const { return int(edges_.size()); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(std::max(0, n_ - 1)));
		built_ = false;
		edges_.emplace_back(u, v);
		return edge_count() - 1;
	}
	const std::pair<int, int>& edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	void build(int root = 0) {
		assert(edge_count() == std::max(0, n_ - 1));
		assert(!n_ || (0 <= root && root < n_));
		built_ = false;
		root_ = n_ ? root : -1;
		parent_.assign(n_, -2);
		heavy_.assign(n_, -1);
		in_.resize(n_);
		out_.assign(n_, 1);
		order_.clear();
		if (!n_) { built_ = true; return; }
		UndirectedCSR graph(n_, edges_);
		order_.reserve(n_);
		order_.push_back(root);
		parent_[root] = -1;
		for (std::size_t i = 0; i < order_.size(); ++i) {
			int v = order_[i];
			for (auto arc : graph[v]) {
				int to = arc.to;
				if (to == parent_[v]) continue;
				assert(parent_[to] == -2);
				if (parent_[to] != -2) continue;
				parent_[to] = v;
				order_.push_back(to);
			}
		}
		assert(int(order_.size()) == n_);
		for (int i = n_ - 1; i > 0; --i) {
			int v = order_[i], p = parent_[v];
			out_[p] += out_[v];
			if (heavy_[p] == -1 || out_[v] > out_[heavy_[p]]) heavy_[p] = v;
		}
		int timer = 0, top = n_ - 1;
		order_[top] = root;
		while (top < n_) {
			int v = order_[top++];
			in_[v] = timer;
			order_[timer++] = v;
			for (auto arc : graph[v])
				if (parent_[arc.to] == v && arc.to != heavy_[v]) order_[--top] = arc.to;
			if (heavy_[v] != -1) order_[--top] = heavy_[v];
		}
		for (int v = 0; v < n_; ++v) out_[v] += in_[v];
		built_ = true;
	}
	int root() const { assert(built_); return root_; }
	int parent(int v) const { check(v); return parent_[v]; }
	int heavy_child(int v) const { check(v); return heavy_[v]; }
	int subtree_size(int v) const { check(v); return out_[v] - in_[v]; }
	std::pair<int, int> subtree(int v) const { check(v); return {in_[v], out_[v]}; }
	int vertex(int i) const {
		assert(built_ && 0 <= i && i < n_);
		return order_[i];
	}
	std::span<const int> order() const { assert(built_); return order_; }
	std::span<const int> vertices(int v) const {
		check(v);
		return std::span<const int>(order_).subspan(in_[v], out_[v] - in_[v]);
	}
	template<class Add, class Remove, class Answer>
	void run(Add&& add, Remove&& remove, Answer&& answer, bool keep = false) const {
		auto clear = [&](int v) {
			for (int i = in_[v]; i < out_[v]; ++i) remove(order_[i]);
		};
		execute(add, answer, clear, keep);
	}
	template<class Add, class Answer, class Clear>
	void run_clear(Add&& add, Answer&& answer, Clear&& clear, bool keep = false) const {
		auto reset = [&](int) { clear(); };
		execute(add, answer, reset, keep);
	}
};

#endif
