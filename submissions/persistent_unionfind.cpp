#line 1 "test/rollback_union_find_persistent.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#line 1 "data_structure/rollback_union_find.hpp"



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


#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 10 "components/fast_io.hpp"
#include <cstdint>
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


#line 6 "test/rollback_union_find_persistent.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	struct Node { int a = 0, b = 0, parent = 0, first = -1, next = -1, query = -1, next_query = -1, type = -1; };
	std::vector<Node> nodes(q + 1);
	std::vector<int> version(q + 1), answer(q + 1);
	for (int i = 1; i <= q; ++i) {
		auto &node = nodes[i];
		int k = 0;
		input.read(node.type), input.read(k), input.read(node.a), input.read(node.b);
		node.parent = version[k + 1];
		if (node.type == 0) version[i] = i;
		else {
			version[i] = node.parent;
			if (node.a == node.b) answer[i] = 1;
			else if (node.parent) {
				node.next_query = nodes[node.parent].query;
				nodes[node.parent].query = i;
			}
		}
	}
	for (int i = q; i; --i) {
		auto &node = nodes[i];
		if (node.type == 0 && (node.first != -1 || node.query != -1)) {
			node.next = nodes[node.parent].first;
			nodes[node.parent].first = i;
		}
	}
	RollbackUnionFind uf(n);
	uf.reserve(q);
	struct Frame { int vertex, next; std::size_t state; };
	std::vector<Frame> stack{{0, nodes[0].first, 0}};
	while (!stack.empty()) {
		auto &frame = stack.back();
		if (frame.next == -1) {
			uf.rollback(frame.state);
			stack.pop_back();
			continue;
		}
		int v = frame.next;
		frame.next = nodes[v].next;
		auto state = uf.snapshot();
		uf.merge(nodes[v].a, nodes[v].b);
		for (int i = nodes[v].query; i != -1; i = nodes[i].next_query) answer[i] = uf.same(nodes[i].a, nodes[i].b);
		stack.push_back({v, nodes[v].first, state});
	}
	for (int i = 1; i <= q; ++i) if (nodes[i].type == 1) output.println(answer[i]);
}
