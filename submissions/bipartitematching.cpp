#line 1 "test/bipartite_matching.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/bipartitematching"
#line 1 "graph/bipartite_matching.hpp"



#line 1 "components/bipartite_matching_core.hpp"



#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

namespace bipartite_matching_detail {
struct CSR {
	std::vector<int> start, adj;
	CSR(int n, std::span<const std::pair<int, int>> edges, bool transpose) : start(n + 1), adj(edges.size()) {
		for (auto [u, v] : edges) ++start[(transpose ? v : u) + 1];
		std::partial_sum(start.begin(), start.end(), start.begin());
		auto next = start;
		for (auto [u, v] : edges) adj[next[transpose ? v : u]++] = transpose ? u : v;
	}
	CSR(int n, const CSR& graph) : start(n + 1), adj(graph.adj.size()) {
		for (int v : graph.adj) ++start[v + 1];
		std::partial_sum(start.begin(), start.end(), start.begin());
		auto next = start;
		for (int u = 0; u + 1 < int(graph.start.size()); ++u)
			for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) adj[next[graph.adj[e]]++] = u;
	}
};

struct Result { int added; bool maximum; };

inline Result hopcroft_karp(const CSR& graph, std::vector<int>& left, std::vector<int>& right,
	int phases = std::numeric_limits<int>::max(), int max_distance = std::numeric_limits<int>::max()) {
	int n = int(left.size()), added = 0;
	std::vector<int> distance(n), queue(n), next(n), path(n);
	for (int phase = 0; phase < phases; ++phase) {
		int tail = 0, limit = n + 1;
		for (int u = 0; u < n; ++u) {
			distance[u] = left[u] < 0 ? 0 : -1;
			if (left[u] < 0) queue[tail++] = u;
		}
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			if (distance[u] >= limit) break;
			for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) {
				int w = right[graph.adj[e]];
				if (w < 0) limit = distance[u] + 1;
				else if (distance[w] < 0) distance[w] = distance[u] + 1, queue[tail++] = w;
			}
		}
		if (limit == n + 1) return {added, true};
		if (limit > max_distance) return {added, false};
		std::copy_n(graph.start.begin(), n, next.begin());
		for (int root = 0; root < n; ++root) if (left[root] < 0 && distance[root] == 0) {
			int depth = 1;
			path[0] = root;
			while (depth) {
				int u = path[depth - 1];
				bool descend = false, found = false;
				while (next[u] < graph.start[u + 1]) {
					int v = graph.adj[next[u]++], w = right[v];
					if (w < 0) {
						if (distance[u] + 1 != limit) continue;
						while (depth) {
							int x = path[--depth], previous = left[x];
							left[x] = v, right[v] = x;
							distance[x] = -1;
							v = previous;
						}
						++added;
						found = true;
						break;
					}
					if (distance[w] == distance[u] + 1 && distance[w] < limit) {
						path[depth++] = w;
						descend = true;
						break;
					}
				}
				if (found) break;
				if (!descend) distance[u] = -1, --depth;
			}
		}
	}
	return {added, false};
}

inline int degree_greedy(const CSR& graph, const CSR& reverse, std::vector<int>& left, std::vector<int>& right,
	std::vector<int>& dl, std::vector<int>& dr, std::vector<int>& queue) {
	int l = int(left.size()), r = int(right.size()), added = 0;
	int tail = 0;
	for (int u = 0; u < l; ++u) {
		dl[u] = graph.start[u + 1] - graph.start[u];
		if (dl[u] == 1) queue[tail++] = u;
	}
	for (int v = 0; v < r; ++v) {
		dr[v] = reverse.start[v + 1] - reverse.start[v];
		if (dr[v] == 1) queue[tail++] = l + v;
	}
	int head = 0, cursor = 0;
	for (;;) {
		int u = -1, v = -1;
		while (head < tail) {
			int x = queue[head++];
			if (x < l) {
				if (!dl[x]) continue;
				u = x;
				break;
			} else {
				x -= l;
				if (!dr[x]) continue;
				v = x;
				break;
			}
		}
		if (u < 0 && v < 0) {
			while (cursor < l && !dl[cursor]) ++cursor;
			if (cursor == l) break;
			u = cursor;
			v = left[u];
		}
		if (v < 0) {
			for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) if (dr[graph.adj[e]]) {
				v = graph.adj[e];
				break;
			}
		}
		if (u < 0) {
			for (int e = reverse.start[v]; e < reverse.start[v + 1]; ++e) if (dl[reverse.adj[e]]) {
				u = reverse.adj[e];
				break;
			}
		}
		if (left[u] != v) {
			int previous = right[v];
			assert(left[u] < 0 || previous < 0);
			if (left[u] >= 0) right[left[u]] = -1;
			else if (previous >= 0) left[previous] = -1;
			else ++added;
			left[u] = v, right[v] = u;
		}
		dl[u] = dr[v] = 0;
		for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) {
			int x = graph.adj[e];
			if (dr[x] && --dr[x] == 1) queue[tail++] = l + x;
		}
		for (int e = reverse.start[v]; e < reverse.start[v + 1]; ++e) {
			int x = reverse.adj[e];
			if (dl[x] && --dl[x] == 1) queue[tail++] = x;
		}
	}
	return added;
}

inline int warm_start(const CSR& graph, std::vector<int>& left, std::vector<int>& right, int multiplier) {
	assert(multiplier >= 0);
	int l = int(left.size()), r = int(right.size()), n = l + r, added = 0;
	int deficit = int(std::count(left.begin(), left.end(), -1));
	if (!deficit || !multiplier) return 0;
	CSR reverse(r, graph);
	std::vector<int> dl(l), dr(r), queue(n);
	added = degree_greedy(graph, reverse, left, right, dl, dr, queue);
	if (added == deficit) return added;
	std::span<int> bfs(queue.data(), l), active(queue.data() + l, r);
	std::uint64_t work = 0, budget = std::uint64_t(graph.adj.size() + n) * multiplier;
	int head = 0, tail = 0, count = 0, steps = 0;
	bool first = true;
	for (;;) {
		if (!steps) {
			work += n;
			if (work > budget) return added;
			int end = 0;
			std::fill(dr.begin(), dr.end(), n);
			for (int u = 0; u < l; ++u) {
				dl[u] = left[u] < 0 ? 0 : n;
				if (left[u] < 0) bfs[end++] = u;
			}
			for (int begin = 0; begin < end; ++begin) {
				int u = bfs[begin];
				work += graph.start[u + 1] - graph.start[u];
				if (work > budget) return added;
				for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) {
					int v = graph.adj[e];
					if (dr[v] <= dl[u] + 1) continue;
					dr[v] = dl[u] + 1;
					int w = right[v];
					if (w >= 0 && dl[w] == n) dl[w] = dr[v] + 1, bfs[end++] = w;
				}
			}
			if (first) {
				for (int v = 0; v < r; ++v) if (right[v] < 0 && dr[v] < n) active[count++] = v;
				tail = count == r ? 0 : count;
				first = false;
			}
		}
		if (!count) return added;
		int v = active[head++], best = -1, minimum = n;
		if (head == r) head = 0;
		--count;
		work += reverse.start[v + 1] - reverse.start[v] + 1;
		if (work > budget) return added;
		for (int e = reverse.start[v]; e < reverse.start[v + 1]; ++e) {
			int u = reverse.adj[e];
			if (dl[u] < minimum) minimum = dl[u], best = u;
		}
		if (best >= 0) {
			int previous = left[best];
			if (previous < 0) ++added;
			else {
				right[previous] = -1;
				active[tail++] = previous;
				if (tail == r) tail = 0;
				++count;
			}
			left[best] = v, right[v] = best;
			dl[best] += 2;
			if (added == deficit) return added;
		}
		if (++steps == n) steps = 0;
	}
}

inline int augment(const CSR& graph, std::vector<int>& left, std::vector<int>& right,
	int multiplier = 16, int root_divisor = 32, int easy_phases = 8) {
	int roots = 0, n = int(left.size());
	for (int u = 0; u < n; ++u) roots += left[u] < 0 && graph.start[u] != graph.start[u + 1];
	Result initial{0, false};
	if (root_divisor > 0 && roots <= std::max(64, n / root_divisor)) initial = hopcroft_karp(graph, left, right, easy_phases);
	if (initial.maximum || std::find(left.begin(), left.end(), -1) == left.end()) return initial.added;
	int added = initial.added + warm_start(graph, left, right, multiplier);
	if (std::find(left.begin(), left.end(), -1) == left.end()) return added;
	return added + hopcroft_karp(graph, left, right).added;
}

struct Analysis {
	int left_size;
	std::vector<int> start, adj, reverse_start, reverse_adj, component, matched_edge;
	std::vector<unsigned char> from_left, to_right;
	Analysis(int l, int r, std::span<const std::pair<int, int>> edges,
		const std::vector<int>& left, const std::vector<int>& right)
		: left_size(l), start(l + r + 1), adj(edges.size()), reverse_start(l + r + 1),
		  reverse_adj(edges.size()), matched_edge(l, -1), from_left(l + r), to_right(l + r) {
		for (int i = 0; i < int(edges.size()); ++i) {
			auto [u, v] = edges[i];
			if (left[u] == v && matched_edge[u] < 0) matched_edge[u] = i;
			v += l;
			if (matched_edge[u] == i) std::swap(u, v);
			++start[u + 1], ++reverse_start[v + 1];
		}
		std::partial_sum(start.begin(), start.end(), start.begin());
		std::partial_sum(reverse_start.begin(), reverse_start.end(), reverse_start.begin());
		auto next = start, reverse_next = reverse_start;
		for (int i = 0; i < int(edges.size()); ++i) {
			auto [u, v] = edges[i];
			v += l;
			if (matched_edge[u] == i) std::swap(u, v);
			adj[next[u]++] = v, reverse_adj[reverse_next[v]++] = u;
		}
		std::vector<int> queue(l + r);
		int tail = 0;
		for (int u = 0; u < l; ++u) if (left[u] < 0) queue[tail++] = u, from_left[u] = 1;
		reach(queue, tail, start, adj, from_left);
		tail = 0;
		for (int v = 0; v < r; ++v) if (right[v] < 0) queue[tail++] = l + v, to_right[l + v] = 1;
		reach(queue, tail, reverse_start, reverse_adj, to_right);
	}
	static void reach(std::vector<int>& queue, int tail, const std::vector<int>& start,
		const std::vector<int>& adj, std::vector<unsigned char>& seen) {
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			for (int e = start[u]; e < start[u + 1]; ++e) if (!seen[adj[e]]) {
				int v = adj[e];
				seen[v] = 1, queue[tail++] = v;
			}
		}
	}
	void components() {
		int n = int(from_left.size());
		if (!component.empty() || !n) return;
		std::vector<unsigned char> seen(n);
		std::vector<int> order, stack, next = start;
		order.reserve(n), stack.reserve(n);
		for (int root = 0; root < n; ++root) if (!seen[root]) {
			seen[root] = 1;
			stack.push_back(root);
			while (!stack.empty()) {
				int u = stack.back();
				if (next[u] == start[u + 1]) order.push_back(u), stack.pop_back();
				else {
					int v = adj[next[u]++];
					if (!seen[v]) seen[v] = 1, stack.push_back(v);
				}
			}
		}
		component.assign(n, -1);
		int count = 0;
		for (auto it = order.rbegin(); it != order.rend(); ++it) if (component[*it] < 0) {
			component[*it] = count;
			stack.push_back(*it);
			while (!stack.empty()) {
				int u = stack.back();
				stack.pop_back();
				for (int e = reverse_start[u]; e < reverse_start[u + 1]; ++e) {
					int v = reverse_adj[e];
					if (component[v] < 0) component[v] = count, stack.push_back(v);
				}
			}
			++count;
		}
	}
};
}


#line 5 "graph/bipartite_matching.hpp"
#include <cstddef>
#line 8 "graph/bipartite_matching.hpp"
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


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/bipartite_matching.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int l = 0, r = 0, m = 0;
	in.read(l), in.read(r), in.read(m);
	BipartiteMatching graph(l, r);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		in.read(u), in.read(v);
		graph.add_edge(u, v);
	}
	out.println(graph.build());
	for (auto [u, v] : graph.edges()) out.write(u), out.write(' '), out.println(v);
}
