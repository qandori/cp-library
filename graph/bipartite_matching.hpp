#ifndef BIPARTITE_MATCHING_HPP
#define BIPARTITE_MATCHING_HPP

#include "../components/bipartite_matching_core.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

class BipartiteMatching {
public:
	struct VertexSet { std::vector<int> left, right; };
	enum class EdgeStatus : unsigned char { never, optional, forced };

private:
	int l_, r_, size_ = 0;
	bool dirty_ = false;
	std::vector<std::pair<int, int>> input_;
	std::vector<int> left_, right_;
	std::optional<bipartite_matching_detail::Analysis> analysis_;
	auto& analysis() {
		build();
		if (!analysis_) analysis_.emplace(l_, r_, input_, left_, right_);
		return *analysis_;
	}
	template<bool left_selected, bool right_selected>
	VertexSet select_vertices(const std::vector<unsigned char>& mask) const {
		VertexSet result;
		for (int u = 0; u < l_; ++u) if (bool(mask[u]) == left_selected) result.left.push_back(u);
		for (int v = 0; v < r_; ++v) if (bool(mask[l_ + v]) == right_selected) result.right.push_back(v);
		return result;
	}

public:
	explicit BipartiteMatching(int l, int r) : l_(l), r_(r), left_(l, -1), right_(r, -1) {
		assert(l >= 0 && r >= 0 && std::int64_t(l) + r < std::numeric_limits<int>::max());
	}
	int left_size() const { return l_; }
	int right_size() const { return r_; }
	int edge_count() const { return int(input_.size()); }
	void reserve(std::size_t m) { assert(m <= std::size_t(std::numeric_limits<int>::max())); input_.reserve(m); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < l_ && 0 <= v && v < r_ && input_.size() < std::size_t(std::numeric_limits<int>::max()));
		int id = edge_count();
		input_.emplace_back(u, v);
		if (left_[u] < 0 && right_[v] < 0) left_[u] = v, right_[v] = u, ++size_;
		dirty_ = true;
		analysis_.reset();
		return id;
	}
	std::pair<int, int> get_edge(int id) const { assert(0 <= id && id < edge_count()); return input_[id]; }
	int build() {
		if (!dirty_) return size_;
		if (size_ < std::min(l_, r_)) {
			bool transpose = l_ > r_;
			bipartite_matching_detail::CSR graph(transpose ? r_ : l_, input_, transpose);
			size_ += bipartite_matching_detail::augment(graph, transpose ? right_ : left_, transpose ? left_ : right_);
		}
		dirty_ = false;
		return size_;
	}
	int matching_size() { return build(); }
	std::vector<std::pair<int, int>> maximum_matching() { return edges(); }
	const std::vector<int>& left_mates() { build(); return left_; }
	const std::vector<int>& right_mates() { build(); return right_; }
	bool covers_left() { return build() == l_; }
	bool covers_right() { return build() == r_; }
	std::vector<std::pair<int, int>> edges() {
		build();
		std::vector<std::pair<int, int>> result;
		result.reserve(size_);
		for (int u = 0; u < l_; ++u) if (left_[u] >= 0) result.emplace_back(u, left_[u]);
		return result;
	}
	std::vector<int> matching_edge_ids() {
		build();
		std::vector<int> ids(l_, -1), result;
		for (int i = 0; i < edge_count(); ++i) {
			auto [u, v] = input_[i];
			if (left_[u] == v && ids[u] < 0) ids[u] = i;
		}
		result.reserve(size_);
		for (int id : ids) if (id >= 0) result.push_back(id);
		return result;
	}
	VertexSet minimum_vertex_cover() { return select_vertices<false, true>(analysis().from_left); }
	VertexSet maximum_independent_set() { return select_vertices<true, false>(analysis().from_left); }
	std::optional<std::vector<int>> minimum_edge_cover() {
		build();
		std::vector<int> first_left(l_, -1), first_right(r_, -1);
		for (int i = 0; i < edge_count(); ++i) {
			auto [u, v] = input_[i];
			if (first_left[u] < 0) first_left[u] = i;
			if (first_right[v] < 0) first_right[v] = i;
		}
		if (std::find(first_left.begin(), first_left.end(), -1) != first_left.end() ||
			std::find(first_right.begin(), first_right.end(), -1) != first_right.end()) return std::nullopt;
		auto result = matching_edge_ids();
		for (int u = 0; u < l_; ++u) if (left_[u] < 0) result.push_back(first_left[u]);
		for (int v = 0; v < r_; ++v) if (right_[v] < 0) result.push_back(first_right[v]);
		return result;
	}
	std::optional<VertexSet> hall_witness_left() {
		if (covers_left()) return std::nullopt;
		return select_vertices<true, true>(analysis().from_left);
	}
	std::optional<VertexSet> hall_witness_right() {
		if (covers_right()) return std::nullopt;
		return select_vertices<true, true>(analysis().to_right);
	}
	std::vector<EdgeStatus> edge_classification() {
		auto& a = analysis();
		a.components();
		std::vector<EdgeStatus> result(edge_count());
		for (int i = 0; i < edge_count(); ++i) {
			auto [u, v] = input_[i];
			bool flexible = a.from_left[u] || a.to_right[l_ + v] || a.component[u] == a.component[l_ + v];
			result[i] = flexible ? EdgeStatus::optional : a.matched_edge[u] == i ? EdgeStatus::forced : EdgeStatus::never;
		}
		return result;
	}
	void clear() {
		input_.clear();
		std::fill(left_.begin(), left_.end(), -1);
		std::fill(right_.begin(), right_.end(), -1);
		size_ = 0;
		dirty_ = false;
		analysis_.reset();
	}
};

#endif
