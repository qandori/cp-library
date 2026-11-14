#line 1 "test/general_matching.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/general_matching"
#line 1 "graph/general_matching.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

class GeneralMatching {
	int n, cardinality = 0, tail = 0;
	std::uint32_t stamp = 0, search_stamp = 0;
	std::vector<std::pair<int, int>> input;
	std::vector<int> start, adj, base, parent, queue;
	std::vector<std::uint32_t> seen, state;
	int find(int v) {
		while (base[v] != v) v = base[v] = base[base[v]];
		return v;
	}
	int lca(int u, int v) {
		if (!++stamp) std::fill(seen.begin(), seen.end(), 0), ++stamp;
		for (;; std::swap(u, v)) if (u != -1) {
			u = find(u);
			if (seen[u] == stamp) return u;
			seen[u] = stamp;
			u = mate[u] == -1 ? -1 : parent[mate[u]];
		}
	}
	void contract(int u, int v, int b) {
		while (find(u) != b) {
			parent[u] = v;
			v = mate[u];
			if (state[v] == (search_stamp | 1)) state[v] = search_stamp, queue[tail++] = v;
			if (base[u] == u) base[u] = b;
			if (base[v] == v) base[v] = b;
			u = parent[v];
		}
	}
	void flip(int v) {
		while (v != -1) {
			int u = parent[v], next = mate[u];
			mate[u] = v;
			mate[v] = u;
			v = next;
		}
	}
	bool augment(int root) {
		search_stamp += 2;
		if (!search_stamp) std::fill(state.begin(), state.end(), 0), search_stamp = 2;
		tail = 1;
		queue[0] = root;
		state[root] = search_stamp;
		base[root] = root;
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			for (int e = start[u]; e < start[u + 1]; ++e) {
				int v = adj[e];
				if ((state[v] & ~std::uint32_t(1)) != search_stamp) {
					parent[v] = u;
					state[v] = search_stamp | 1;
					base[v] = v;
					if (mate[v] == -1) { flip(v); return true; }
					state[mate[v]] = search_stamp;
					base[mate[v]] = mate[v];
					queue[tail++] = mate[v];
				} else if (state[v] == search_stamp && find(u) != find(v)) {
					int b = lca(u, v);
					contract(u, v, b);
					contract(v, u, b);
				}
			}
		}
		return false;
	}
	void short_paths() {
		std::vector<int> first(n, -1), second(n, -1);
		auto save = [&](int u, int v) {
			if (first[u] == -1) first[u] = v;
			else if (first[u] != v) second[u] = v;
		};
		for (auto [u, v] : input) {
			if (mate[u] == -1) save(v, u);
			if (mate[v] == -1) save(u, v);
		}
		for (int u = 0; u < n; ++u) if (u < mate[u]) {
			int v = mate[u];
			for (int a : {first[u], second[u]}) if (a != -1 && mate[a] == -1) {
				int b = first[v];
				if (b == -1 || b == a || mate[b] != -1) b = second[v];
				if (b == -1 || b == a || mate[b] != -1) continue;
				mate[u] = a;
				mate[a] = u;
				mate[v] = b;
				mate[b] = v;
				++cardinality;
				break;
			}
		}
	}
	int component_bound(std::vector<int> &roots) {
		std::fill(parent.begin(), parent.end(), -1);
		int bound = 0;
		for (int root = 0; root < n; ++root) if (parent[root] == -1) {
			int end = 1, count[2] = {1, 0};
			bool bipartite = true;
			queue[0] = root;
			parent[root] = 0;
			for (int head = 0; head < end; ++head) {
				int u = queue[head];
				for (int e = start[u]; e < start[u + 1]; ++e) {
					int v = adj[e];
					if (parent[v] == -1) {
						parent[v] = parent[u] ^ 1;
						++count[parent[v]];
						queue[end++] = v;
					} else if (parent[v] == parent[u]) bipartite = false;
				}
			}
			bound += bipartite ? std::min(count[0], count[1]) : end / 2;
			int side = count[0] > count[1];
			for (int i = 0; i < end; ++i) {
				int u = queue[i];
				if (mate[u] == -1 && (!bipartite || parent[u] == side)) roots.push_back(u);
			}
		}
		return bound;
	}
public:
	std::vector<int> mate;
	explicit GeneralMatching(int n) : n(n), mate(n, -1) { assert(n >= 0); }
	int size() const { return n; }
	void reserve(std::size_t m) { input.reserve(m); }
	void add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		if (u == v || cardinality == n / 2) return;
		input.emplace_back(u, v);
		if (mate[u] == -1 && mate[v] == -1) {
			mate[u] = v;
			mate[v] = u;
			++cardinality;
		}
	}
	int build() {
		if (input.empty() || cardinality == n / 2) return cardinality;
		short_paths();
		if (cardinality == n / 2) return cardinality;
		start.assign(n + 1, 0);
		for (auto [u, v] : input) ++start[u + 1], ++start[v + 1];
		std::partial_sum(start.begin(), start.end(), start.begin());
		adj.resize(input.size() * 2);
		parent = start;
		for (auto [u, v] : input) adj[parent[u]++] = v, adj[parent[v]++] = u;
		parent.resize(n);
		queue.resize(n);
		std::vector<int> roots;
		int limit = component_bound(roots);
		if (cardinality == limit) return cardinality;
		base.resize(n);
		seen.resize(n);
		state.resize(n);
		for (int u : roots) if (mate[u] == -1 && augment(u)) {
			if (++cardinality == limit) break;
		}
		return cardinality;
	}
	int matching_size() { return build(); }
	std::vector<std::pair<int, int>> maximum_matching() { build(); return edges(); }
	std::vector<std::pair<int, int>> edges() const {
		std::vector<std::pair<int, int>> result;
		result.reserve(cardinality);
		for (int u = 0; u < n; ++u) if (u < mate[u]) result.emplace_back(u, mate[u]);
		return result;
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


#line 4 "test/general_matching.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n);
	in.read(m);
	GeneralMatching g(n);
	g.reserve(m);
	for (int i = 0, u = 0, v = 0; i < m; ++i) {
		in.read(u);
		in.read(v);
		g.add_edge(u, v);
	}
	out.write(g.build());
	out.write('\n');
	for (auto [u, v] : g.edges()) {
		out.write(u);
		out.write(' ');
		out.write(v);
		out.write('\n');
	}
}
