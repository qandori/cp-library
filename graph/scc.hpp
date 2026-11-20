#ifndef SCC_HPP
#define SCC_HPP

#include "../components/scc_core.hpp"

class SCCGraph {
	int n_;
	std::vector<std::pair<int, int>> edges_;

  public:
	struct Components {
		std::vector<int> id, vertices, offset;
		Components() = default;
		Components(const Components &) = default;
		Components(Components &&other) noexcept { swap(other); }
		Components &operator=(Components other) noexcept { swap(other); return *this; }
		void swap(Components &other) noexcept { id.swap(other.id), vertices.swap(other.vertices), offset.swap(other.offset); }
		int size() const { return offset.empty() ? 0 : static_cast<int>(offset.size() - 1); }
		std::span<const int> operator[](int component) const {
			assert(0 <= component && component < size());
			return {vertices.data() + offset[component], std::size_t(offset[component + 1] - offset[component])};
		}
	};
	struct Condensation {
		std::vector<std::vector<int>> dag;
		std::vector<int> id;
	};
	explicit SCCGraph(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	std::size_t edge_count() const { return edges_.size(); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max()));
		edges_.reserve(m);
	}
	void add_edge(int from, int to) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		edges_.emplace_back(from, to);
	}
	std::pair<int, std::vector<int>> scc_ids() const { return scc_detail::ids(n_, edges_); }
	Condensation scc_dag() const {
		auto [count, id] = scc_ids();
		Condensation result{std::vector<std::vector<int>>(count), std::move(id)};
		if (count <= 1 || edges_.empty()) return result;
		const auto &component = result.id;
		std::vector<int> offset(std::size_t(count) + 1);
		for (auto [u, v] : edges_) if (component[u] != component[v]) ++offset[component[u]];
		for (int c = 0; c < count; ++c) offset[c + 1] += offset[c];
		std::vector<int> targets(offset.back()), seen(count, -1);
		for (auto [u, v] : edges_) {
			int from = component[u], to = component[v];
			if (from != to) targets[--offset[from]] = to;
		}
		for (int from = 0; from < count; ++from) {
			for (int i = offset[from]; i < offset[from + 1]; ++i) {
				int to = targets[i];
				if (seen[to] == from) continue;
				seen[to] = from;
				result.dag[from].push_back(to);
			}
		}
		return result;
	}
	Components decomposition() const {
		auto [count, id] = scc_ids();
		Components result;
		result.id = std::move(id);
		result.offset.resize(std::size_t(count) + 1);
		for (int x : result.id) ++result.offset[x + 1];
		for (int i = 0; i < count; ++i) result.offset[i + 1] += result.offset[i];
		result.vertices.resize(n_);
		auto cursor = result.offset;
		for (int v = 0; v < n_; ++v) result.vertices[cursor[result.id[v]]++] = v;
		return result;
	}
	std::vector<std::vector<int>> scc() const {
		auto [count, id] = scc_ids();
		std::vector<int> sizes(count);
		for (int x : id) ++sizes[x];
		std::vector<std::vector<int>> groups(count);
		for (int i = 0; i < count; ++i) groups[i].reserve(sizes[i]);
		for (int v = 0; v < n_; ++v) groups[id[v]].push_back(v);
		return groups;
	}
};

#endif
