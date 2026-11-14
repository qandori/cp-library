#line 1 "test/offline_dynamic_connectivity.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum"
#line 1 "graph/offline_dynamic_connectivity.hpp"



#line 1 "components/offline_time_segments.hpp"



#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class OfflineTimeSegments {
	int n_;
	std::size_t base_;
	std::vector<std::size_t> offset_;
	std::vector<int> items_;

  public:
	struct Interval { int left, right, item; };
	OfflineTimeSegments(int n, const std::vector<Interval> &intervals) : n_(n), base_(1) {
		assert(n >= 0);
		if (!n || intervals.empty()) return;
		base_ = std::bit_ceil(static_cast<std::size_t>(n));
		offset_.resize(base_ * 2 + 1);
		auto cover = [&](const Interval &interval, auto &&visit) {
			assert(0 <= interval.left && interval.left <= interval.right && interval.right <= n);
			for (std::size_t l = base_ + interval.left, r = base_ + interval.right; l < r; l >>= 1, r >>= 1) {
				if (l & 1) visit(l++);
				if (r & 1) visit(--r);
			}
		};
		for (const auto &interval : intervals) cover(interval, [&](std::size_t v) { ++offset_[v]; });
		for (std::size_t v = 1; v < offset_.size(); ++v) offset_[v] += offset_[v - 1];
		items_.resize(offset_.back());
		for (const auto &interval : intervals)
			cover(interval, [&](std::size_t v) { items_[--offset_[v]] = interval.item; });
	}
	int size() const { return n_; }
	std::size_t entry_count() const { return items_.size(); }
	template <class State, class Apply, class Visit>
	void run(State &state, Apply &&apply, Visit &&visit) const {
		if (!n_) return;
		if (items_.empty()) {
			for (int t = 0; t < n_; ++t) visit(t, std::as_const(state));
			return;
		}
		auto dfs = [&](auto &&self, std::size_t v, std::size_t l, std::size_t r) -> void {
			auto saved = state.snapshot();
			for (std::size_t i = offset_[v]; i < offset_[v + 1]; ++i) apply(state, items_[i]);
			if (r - l == 1) visit(static_cast<int>(l), std::as_const(state));
			else {
				std::size_t m = (l + r) / 2;
				self(self, v * 2, l, m);
				if (m < static_cast<std::size_t>(n_)) self(self, v * 2 + 1, m, r);
			}
			state.rollback(saved);
		};
		dfs(dfs, 1, 0, base_);
	}
};


#line 1 "components/radix_sort.hpp"



#include <algorithm>
#include <array>
#line 7 "components/radix_sort.hpp"
#include <cstdint>
#include <type_traits>
#line 11 "components/radix_sort.hpp"

template <int Bits, class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key &&key) {
	static_assert(1 <= Bits && Bits <= 16);
	constexpr unsigned buckets = 1U << Bits, mask = buckets - 1;
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	if constexpr (Bits == 11 && sizeof(K) <= 4) {
		if (values.size() <= UINT32_MAX && (varying & mask) && ((varying >> 11) & mask) && (varying >> 22)) {
			std::array<std::array<std::uint32_t, buckets>, 3> counts{};
			for (const auto &value : values) {
				std::uint32_t k = key(value);
				++counts[0][k & mask];
				++counts[1][(k >> 11) & mask];
				++counts[2][k >> 22];
			}
			for (int pass = 0; pass < 3; ++pass) {
				auto &count = counts[pass];
				std::uint32_t sum = 0;
				for (auto &x : count) {
					auto n = x;
					x = sum, sum += n;
				}
				for (auto &&value : values) scratch[count[(std::uint32_t(key(value)) >> (11 * pass)) & mask]++] = std::move(value);
				values.swap(scratch);
			}
			return;
		}
	}
	using Counts = std::conditional_t<(Bits <= 11), std::array<std::size_t, buckets>, std::vector<std::size_t>>;
	Counts count = [] {
		if constexpr (Bits <= 11) return Counts{};
		else return Counts(buckets);
	}();
	for (int shift = 0; shift < 64; shift += Bits) {
		if (!((varying >> shift) & mask)) continue;
		std::fill(count.begin(), count.end(), 0);
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask]++] = std::move(value);
		values.swap(scratch);
	}
}

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	stable_radix_sort<8>(values, std::move(key));
}

namespace radix_sort_detail {
template <int Bits = 8, class T, class Key>
void sort_by_key(std::vector<T> &values, Key key) {
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	if constexpr (std::is_integral_v<K> && !std::is_same_v<K, bool> && sizeof(K) <= 8 && std::is_default_constructible_v<T>) {
		using U = std::make_unsigned_t<K>;
		constexpr U sign = std::is_signed_v<K> ? U(1) << (sizeof(K) * 8 - 1) : U(0);
		stable_radix_sort<Bits>(values, [&](const T &value) { return U(U(key(value)) ^ sign); });
	} else std::sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
}
}


#line 1 "data_structure/rollback_union_find.hpp"



#line 8 "data_structure/rollback_union_find.hpp"

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


#line 11 "graph/offline_dynamic_connectivity.hpp"
#include <limits>
#line 14 "graph/offline_dynamic_connectivity.hpp"

class OfflineDynamicConnectivity {
	struct Event { std::uint64_t key; int time; std::uint32_t order; };
	struct Query { int type, u, v; };
	int n_;
	std::vector<Event> events_;
	std::vector<Query> queries_;
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	void update_(int u, int v, bool remove) {
		check_(u), check_(v);
		if (u == v) return;
		assert(events_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
		if (u > v) std::swap(u, v);
		std::uint64_t key = (std::uint64_t(static_cast<unsigned>(u)) << 32) | static_cast<unsigned>(v);
		std::uint32_t order = static_cast<std::uint32_t>(events_.size()) | (std::uint32_t(remove) << 31);
		events_.push_back({key, query_count(), order});
	}
	int query_(int type, int u, int v) {
		assert(queries_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
		int id = query_count();
		queries_.push_back({type, u, v});
		return id;
	}
	template <bool Hooks, class Link, class Cut, class Visit>
	void run_(Link &link, Cut &cut, Visit &visit) const {
		if (queries_.empty()) return;
		auto events = events_;
		stable_radix_sort(events, [](const Event &event) { return event.key; });
		std::vector<OfflineTimeSegments::Interval> intervals;
		std::vector<std::pair<int, int>> edges;
		intervals.reserve(events.size() / 2), edges.reserve(events.size() / 2);
		for (std::size_t i = 0; i < events.size();) {
			std::uint64_t key = events[i].key;
			int count = 0, start = 0;
			auto append = [&](int end) {
				if (start == end) return;
				intervals.push_back({start, end, static_cast<int>(edges.size())});
				edges.emplace_back(static_cast<int>(key >> 32), static_cast<int>(key & 0xffffffffU));
			};
			do {
				const auto &event = events[i++];
				if (!(event.order >> 31)) {
					if (count++ == 0) start = event.time;
				} else if (count && --count == 0) append(event.time);
			} while (i < events.size() && events[i].key == key);
			if (count) append(query_count());
		}
		OfflineTimeSegments timeline(query_count(), intervals);
		struct State {
			RollbackUnionFind uf;
			std::vector<std::pair<int, int>> history;
			Link &link;
			Cut &cut;
			std::size_t snapshot() const { return uf.snapshot(); }
			void merge(int u, int v) {
				int a = uf.leader(u), b = uf.leader(v);
				if (a == b) return;
				if (uf.size(a) < uf.size(b)) std::swap(a, b);
				uf.merge(a, b);
				if constexpr (Hooks) {
					history.emplace_back(a, b);
					link(a, b);
				}
			}
			void rollback(std::size_t saved) {
				if constexpr (Hooks) {
					while (uf.snapshot() > saved) {
						auto [a, b] = history.back();
						cut(a, b);
						history.pop_back(), uf.undo();
					}
				} else uf.rollback(saved);
			}
		};
		State state{RollbackUnionFind(n_), {}, link, cut};
		std::size_t capacity = std::min<std::size_t>(n_ ? n_ - 1 : 0, edges.size());
		state.uf.reserve(capacity);
		if constexpr (Hooks) state.history.reserve(capacity);
		timeline.run(state, [&](State &s, int id) { s.merge(edges[id].first, edges[id].second); },
			[&](int id, const State &s) { visit(id, s.uf); });
	}

  public:
	explicit OfflineDynamicConnectivity(int n = 0) : n_(n) { assert(n >= 0); }
	void reset(int n) { assert(n >= 0); n_ = n; events_.clear(), queries_.clear(); }
	void reserve(int updates, int queries = 0) {
		assert(updates >= 0 && queries >= 0);
		events_.reserve(updates), queries_.reserve(queries);
	}
	int vertex_count() const { return n_; }
	int query_count() const { return static_cast<int>(queries_.size()); }
	void link(int u, int v) { update_(u, v, false); }
	void cut(int u, int v) { update_(u, v, true); }
	int query_same(int u, int v) { check_(u), check_(v); return query_(0, u, v); }
	int query_size(int v) { check_(v); return query_(1, v, 0); }
	int query_component_count() { return query_(2, 0, 0); }
	int query() { return query_(3, 0, 0); }
	template <class Visit>
	void run(Visit &&visit) const {
		auto noop = [](int, int) {};
		run_<false>(noop, noop, visit);
	}
	template <class Link, class Cut, class Visit>
	void run(Link &&link, Cut &&cut, Visit &&visit) const { run_<true>(link, cut, visit); }
	std::vector<int> solve() const {
		std::vector<int> answers(queries_.size());
		run([&](int id, const RollbackUnionFind &uf) {
			const auto &q = queries_[id];
			if (q.type == 0) answers[id] = uf.same(q.u, q.v);
			else if (q.type == 1) answers[id] = uf.size(q.u);
			else { assert(q.type == 2); answers[id] = uf.component_count(); }
		});
		return answers;
	}
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
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


#line 6 "test/offline_dynamic_connectivity.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<std::int64_t> sum(n);
	for (auto &x : sum) input.read(x);
	OfflineDynamicConnectivity graph(n);
	graph.reserve(q, q);
	struct Query { int type, vertex; std::int64_t delta; };
	std::vector<Query> queries;
	queries.reserve(q);
	for (int i = 0; i < q; ++i) {
		int type, u;
		input.read(type), input.read(u);
		if (type < 2) {
			int v;
			input.read(v);
			if (type == 0) graph.link(u, v);
			else graph.cut(u, v);
		} else {
			std::int64_t delta = 0;
			if (type == 2) input.read(delta);
			graph.query(), queries.push_back({type, u, delta});
		}
	}
	std::vector<int> parent(n, -1);
	graph.run([&](int root, int child) {
		parent[child] = root, sum[root] += sum[child];
	}, [&](int root, int child) {
		sum[root] -= sum[child], parent[child] = -1;
	}, [&](int id, const RollbackUnionFind &uf) {
		auto [type, vertex, delta] = queries[id];
		if (type == 2) {
			for (int v = vertex; v != -1; v = parent[v]) sum[v] += delta;
		} else output.println(sum[uf.leader(vertex)]);
	});
}
