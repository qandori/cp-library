#line 1 "test/lowest_common_ancestor.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#line 1 "tree/lca.hpp"



#line 1 "components/static_range_min.hpp"



#line 1 "components/default_init_allocator.hpp"



#include <memory>
#include <new>

template <class T>
struct DefaultInitAllocator : std::allocator<T> {
	template <class U>
	struct rebind {
		using other = DefaultInitAllocator<U>;
	};
	DefaultInitAllocator() = default;
	template <class U>
	DefaultInitAllocator(const DefaultInitAllocator<U> &) noexcept {}
	template <class U>
	void construct(U *p) { ::new (static_cast<void *>(p)) U; }
};


#line 1 "data_structure/sparse_table.hpp"



#line 5 "data_structure/sparse_table.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e = nullptr>
class SparseTable {
	using Value = std::conditional_t<std::is_same_v<S, bool>, std::uint8_t, S>;
	using Reference = std::conditional_t<std::is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	std::vector<std::vector<Value, DefaultInitAllocator<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (std::is_same_v<S, bool> || !std::is_invocable_v<decltype(op), const S &, const S &>) return S(op(S(a), S(b)));
		else return op(a, b);
	}

  public:
	SparseTable() = default;
	SparseTable(const SparseTable &) = default;
	SparseTable &operator=(const SparseTable &) = default;
	SparseTable(SparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	SparseTable &operator=(SparseTable &&other) noexcept {
		if (this != &other) {
			n = std::exchange(other.n, 0);
			data = std::move(other.data);
		}
		return *this;
	}
	explicit SparseTable(const std::vector<S> &a) { build(a); }
	void clear() {
		data.clear();
		n = 0;
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		build(static_cast<int>(a.size()), [&](int i) -> S { return a[i]; });
	}
	template <class F>
	void build(int size, F value) {
		assert(size >= 0);
		n = size;
		data.resize(std::bit_width(unsigned(n)));
		if (!n) return;
		if constexpr (std::is_default_constructible_v<Value>) {
			data[0].resize(n);
			for (int i = 0; i < n; ++i) data[0][i] = value(i);
		} else {
			data[0].clear();
			data[0].reserve(n);
			for (int i = 0; i < n; ++i) data[0].push_back(value(i));
		}
		for (int k = 1; k < std::bit_width(unsigned(n)); ++k) {
			int length = n - (1U << k) + 1, half = 1U << (k - 1);
			if constexpr (std::is_default_constructible_v<Value>) {
				data[k].resize(length);
				for (int i = 0; i < length; ++i) data[k][i] = combine(data[k - 1][i], data[k - 1][i + half]);
			} else {
				data[k].clear();
				data[k].reserve(length);
				for (int i = 0; i < length; ++i) data[k].push_back(combine(data[k - 1][i], data[k - 1][i + half]));
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	Reference operator[](int i) const {
		assert(0 <= i && i < n);
		return data[0][i];
	}
	[[gnu::always_inline]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if constexpr (std::is_same_v<decltype(e), std::nullptr_t>) assert(l < r);
		else if (l == r) return e();
		return prod_unchecked(l, r);
	}
	[[gnu::always_inline]] S prod_unchecked(int l, int r) const {
		int k = std::bit_width(unsigned(r - l)) - 1;
		return combine(data[k][l], data[k][r - (1U << k)]);
	}
	S all_prod() const { return prod(0, n); }
};


#line 7 "components/static_range_min.hpp"
#include <array>
#line 12 "components/static_range_min.hpp"
#include <span>
#line 16 "components/static_range_min.hpp"

template <class T = int, bool FastBoundaries = false> class StaticRangeMin {
	static T minimum(T a, T b) { return b < a ? b : a; }
	std::vector<T> values_;
	std::vector<std::uint32_t, DefaultInitAllocator<std::uint32_t>> masks_;
	SparseTable<T, minimum> blocks_;
	struct Boundaries { std::vector<T> prefix, suffix; };
	struct Empty {};
	[[no_unique_address]] std::conditional_t<FastBoundaries, Boundaries, Empty> boundaries_;
	T within(int l, int r) const {
		auto mask = masks_[r - 1] & (~std::uint32_t(0) << (l & 31));
		return values_[(l & ~31) + std::countr_zero(mask)];
	}

  public:
	StaticRangeMin() = default;
	explicit StaticRangeMin(std::vector<T> values) { build(std::move(values)); }
	void build(std::vector<T> values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		values_ = std::move(values);
		int n = size();
		masks_.resize(n);
		for (int first = 0; first < n;) {
			int last = first + std::min(32, n - first);
			std::uint32_t mask = 0;
			for (int i = first; i < last; ++i) {
				while (mask && !(values_[first + std::bit_width(mask) - 1] < values_[i]))
					mask &= ~(std::uint32_t(1) << (std::bit_width(mask) - 1));
				mask |= std::uint32_t(1) << (i - first);
				masks_[i] = mask;
			}
			first = last;
		}
		if constexpr (FastBoundaries) {
			boundaries_.prefix = values_, boundaries_.suffix = values_;
			for (int first = 0; first < n;) {
				int last = first + std::min(32, n - first);
				for (int i = first + 1; i < last; ++i) boundaries_.prefix[i] = minimum(boundaries_.prefix[i - 1], values_[i]);
				for (int i = last - 1; i > first; --i) boundaries_.suffix[i - 1] = minimum(values_[i - 1], boundaries_.suffix[i]);
				first = last;
			}
		}
		blocks_.build(n ? (n - 1) / 32 + 1 : 0, [&](int i) {
			int last = i * 32 + std::min(32, n - i * 32);
			if constexpr (FastBoundaries) return T(boundaries_.prefix[last - 1]);
			else return within(i * 32, last);
		});
	}
	int size() const { return static_cast<int>(values_.size()); }
	bool empty() const { return values_.empty(); }
	std::conditional_t<std::is_same_v<T, bool>, bool, const T &> operator[](int i) const {
		assert(0 <= i && i < size());
		return values_[i];
	}
	T prod(int l, int r) const {
		assert(0 <= l && l < r && r <= size());
		int a = l / 32, b = (r - 1) / 32;
		if (a == b) return within(l, r);
		T value = [&] {
			if constexpr (FastBoundaries) return minimum(boundaries_.suffix[l], boundaries_.prefix[r - 1]);
			else return minimum(within(l, (a + 1) * 32), within(b * 32, r));
		}();
		if (a + 1 < b) value = minimum(value, blocks_.prod_unchecked(a + 1, b));
		return value;
	}
	std::vector<T> prod_many(std::span<const std::pair<int, int>> ranges) const {
		std::vector<T> result;
		result.reserve(ranges.size());
		if constexpr (FastBoundaries) {
			for (auto [l, r] : ranges) result.emplace_back(prod(l, r));
		} else {
			std::array<int, 32> left, right;
			for (std::size_t first = 0; first < ranges.size(); first += 32) {
				int count = int(std::min<std::size_t>(32, ranges.size() - first));
				for (int i = 0; i < count; ++i) {
					auto [l, r] = ranges[first + i];
					assert(0 <= l && l < r && r <= size());
					int a = l & ~31, b = (r - 1) & ~31;
					auto mask = masks_[a == b ? r - 1 : a + 31] & (~std::uint32_t(0) << (l & 31));
					left[i] = a + std::countr_zero(mask);
					right[i] = a == b ? left[i] : b + std::countr_zero(masks_[r - 1]);
#if defined(__GNUC__) || defined(__clang__)
					if constexpr (!std::is_same_v<T, bool>) {
						__builtin_prefetch(values_.data() + left[i]);
						__builtin_prefetch(values_.data() + right[i]);
					}
#endif
				}
				for (int i = 0; i < count; ++i) {
					auto [l, r] = ranges[first + i];
					T value = minimum(values_[left[i]], values_[right[i]]);
					int a = l / 32 + 1, b = (r - 1) / 32;
					if (a < b) value = minimum(value, blocks_.prod_unchecked(a, b));
					result.emplace_back(std::move(value));
				}
			}
		}
		return result;
	}

	T all_prod() const { return prod(0, size()); }
};


#line 11 "tree/lca.hpp"

class LCA {
	std::vector<int> in, out, order;
	bool ordered_ = false;
	StaticRangeMin<int, true> table;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && static_cast<int>(in.size()) == n); }

	template<bool Ordered = false> void finish(int root) {
		in[root] = n;
		ordered_ = root == 0;
		for (int i = n - 2; i >= 0; --i) {
			int v = Ordered ? n - 1 - i : order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			in[v] = in[p];
			in[p] -= out[v];
		}
		std::vector<int> values(n);
		for (int v = 0; v < n; ++v) {
			--in[v];
			out[v] += in[v];
			order[in[v]] = v;
			if constexpr (Ordered) values[in[v]] = v ? parent[v] : 0;
			else ordered_ &= parent[v] < v;
		}
		if constexpr (!Ordered)
			for (int i = 1; i < n; ++i) values[i] = ordered_ ? parent[order[i]] : in[parent[order[i]]];
		table.build(std::move(values));
		parent[root] = -1;
	}

  public:
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth;
	explicit LCA(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return static_cast<int>(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		in.resize(n);
		out.resize(n);
		order.resize(n);
		parent.assign(n, -1);
		depth.assign(n, 0);
		table = StaticRangeMin<int, true>();
		if (!n) return;
		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		degree[root] += 2;
		std::fill(out.begin(), out.end(), 1);
		int count = 0;
		for (int v = 0; v < n; ++v) if (degree[v] == 1) order[count++] = v;
		for (int i = 0; i < count; ++i) {
			int v = order[i], p = neighbors[v];
			parent[v] = p;
			out[p] += out[v];
			neighbors[p] ^= v;
			if (--degree[p] == 1) order[count++] = p;
		}
		assert(count == n - 1);
		finish(root);
	}

	void build_from_parents(std::vector<int> parents) {
		assert(parents.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		n = static_cast<int>(parents.size());
		parent = std::move(parents);
		in.resize(n);
		out.assign(n, 1);
		order.resize(n);
		depth.assign(n, 0);
		table = StaticRangeMin<int, true>();
		edges.clear();
		edges.reserve(std::max(0, n - 1));
		if (!n) return;
		int root = -1;
		bool ordered = true;
		for (int v = 0; v < n; ++v) {
			int p = parent[v];
			assert(-1 <= p && p < n && p != v);
			if (p == -1) {
				assert(root == -1);
				root = v;
			} else {
				edges.emplace_back(p, v);
				ordered &= p < v;
			}
		}
		assert(root != -1);
		if (ordered) {
			for (int v = n - 1; v > 0; --v) {
				out[parent[v]] += out[v];
			}
			finish<true>(root);
			return;
		} else {
			std::fill(in.begin(), in.end(), 0);
			for (int v = 0; v < n; ++v) if (v != root) ++in[parent[v]];
			int count = 0;
			for (int v = 0; v < n; ++v) if (!in[v] && v != root) order[count++] = v;
			for (int i = 0; i < count; ++i) {
				int v = order[i], p = parent[v];
				out[p] += out[v];
				if (!--in[p] && p != root) order[count++] = p;
			}
			assert(count == n - 1);
		}
		finish(root);
	}

	int lca(int u, int v) const {
		check(u);
		check(v);
		int l = in[u], r = in[v];
		if (l == r) return u;
		if (l > r) std::swap(l, r);
		++l;
		int w = table.prod(l, r + 1);
		return ordered_ ? w : order[w];
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	int distance(int u, int v) const { return dist(u, v); }
	bool is_ancestor(int u, int v) const {
		check(u);
		check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	std::pair<int, int> subtree(int v) const {
		check(v);
		return {in[v], out[v]};
	}
};


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 5 "components/fast_io_bulk.hpp"

#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#line 14 "components/fast_io.hpp"
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


#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 10 "components/fast_io_bulk.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	template <bool Signed = false>
	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid, unsigned *signs = nullptr) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		unsigned n0 = 0, n1 = 0;
		if constexpr (Signed) {
			if (a < 1 || a > 11 || b <= a + 1 || b - a - 1 > 11) [[unlikely]] {
				invalid = _mm256_set1_epi32(-1);
				p += std::min(b + 1, 32u);
				*signs = 0;
				return _mm256_setzero_si256();
			}
			n0 = p[0] == '-', n1 = p[a + 1] == '-';
			*signs = n0 | (n1 << 1);
		}
		if (a < n0 + 1 || a - n0 > 10 || b <= a + n1 + 1 || b - a - n1 - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		auto window = bytes;
		if constexpr (Signed) window = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1 + n1)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + n0)));
		else window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - n1 - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a - n0].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	template <bool Signed = false>
	static std::uint32_t one(const char *&p, bool &bad) {
		bool negative = false;
		if constexpr (Signed) negative = *p == '-', p += negative;
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		if constexpr (Signed) bad |= value > 2147483647ULL + negative;
		else bad |= value > 0xffffffffULL;
		p += n + 1;
		return negative ? 0u - std::uint32_t(value) : std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	template <bool Signed = false>
	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				unsigned signs0 = 0, signs1 = 0, signs2 = 0, signs3 = 0;
				const auto g0 = two<Signed>(p[0], invalid, &signs0), g1 = two<Signed>(p[1], invalid, &signs1), g2 = two<Signed>(p[2], invalid, &signs2), g3 = two<Signed>(p[3], invalid, &signs3);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				auto max_high = _mm256_set1_epi32(42), max_low = _mm256_set1_epi32(94967295);
				auto signs = _mm256_setzero_si256();
				if constexpr (Signed) {
					signs = _mm256_setr_epi32(signs0 & 1, signs1 & 1, signs2 & 1, signs3 & 1, signs0 >> 1, signs1 >> 1, signs2 >> 1, signs3 >> 1);
					max_high = _mm256_set1_epi32(21), max_low = _mm256_add_epi32(_mm256_set1_epi32(47483647), signs);
				}
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, max_high),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, max_high), _mm256_cmpgt_epi32(low, max_low)));
				invalid = _mm256_or_si256(invalid, overflow);
				auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				if constexpr (Signed) {
					const auto mask = _mm256_sub_epi32(_mm256_setzero_si256(), signs);
					value = _mm256_sub_epi32(_mm256_xor_si256(value, mask), mask);
				}
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <bool Signed = false, class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep<Signed>(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one<Signed>(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <bool Signed = false, class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks<Signed>(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (!Signed && done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				std::conditional_t<Signed, std::int32_t, unsigned> value = 0;
				in.read(value);
				emit(reinterpret_cast<unsigned *>(&value), 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, class T, std::size_t... I>
void emit_record(Emit &emit, const T *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template <class Emit>
void read_signed_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks<true>(input, count, [&](const unsigned *values, std::size_t n) {
			emit(reinterpret_cast<const std::int32_t *>(values), n);
		});
		return;
	}
#endif
	std::int32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

template <std::size_t Fields, class Emit>
void read_signed_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::int32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_signed_blocks(input, count * Fields, [&](const std::int32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 4 "test/lowest_common_ancestor.test.cpp"
#include <tuple>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<int> parents(n, -1);
	int vertex = 1;
	read_unsigned_records<1>(input, n - 1, [&](unsigned p) { parents[vertex++] = p; });
	LCA tree;
	tree.build_from_parents(std::move(parents));
	std::vector<unsigned> answers;
	answers.reserve(q);
	read_unsigned_records<2>(input, q, [&](unsigned u, unsigned v) { answers.push_back(tree.lca(u, v)); });
	write_unsigned_records<1>(output, answers.size(), [&](std::size_t i) { return std::array{answers[i]}; });
}
