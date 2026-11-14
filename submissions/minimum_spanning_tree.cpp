#line 1 "test/minimum_spanning_tree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/minimum_spanning_tree"
#line 1 "graph/kruskal.hpp"



#line 1 "data_structure/union_find.hpp"



#include <algorithm>
#include <cassert>
#include <vector>

class UnionFind {
	std::vector<int> parent_;
	int count_ = 0;

  public:
	explicit UnionFind(int n = 0) : count_(n) {
		assert(n >= 0);
		parent_.assign(n, -1);
	}
	[[nodiscard]] int size() const { return static_cast<int>(parent_.size()); }
	[[nodiscard]] int count() const { return count_; }
	[[nodiscard]] int vertex_count() const { return size(); }
	[[nodiscard]] int component_count() const { return count(); }
	[[nodiscard]] int leader(int v) {
		assert(0 <= v && v < size());
		while (parent_[v] >= 0) {
			int p = parent_[v];
			if (parent_[p] < 0) return p;
			v = parent_[v] = parent_[p];
		}
		return v;
	}
	int merge(int a, int b) {
		a = leader(a), b = leader(b);
		if (a == b) return a;
		if (parent_[a] > parent_[b]) std::swap(a, b);
		parent_[a] += parent_[b];
		parent_[b] = a;
		--count_;
		return a;
	}
	[[nodiscard]] bool same(int a, int b) { return leader(a) == leader(b); }
	[[nodiscard]] int size(int v) { return -parent_[leader(v)]; }
	[[nodiscard]] std::vector<std::vector<int>> groups() {
		std::vector<int> index(size(), -1);
		std::vector<std::vector<int>> result(count_);
		int next = 0;
		for (int v = 0; v < size(); ++v) if (parent_[v] < 0) {
			index[v] = next;
			result[next++].reserve(-parent_[v]);
		}
		for (int v = 0; v < size(); ++v) result[index[leader(v)]].push_back(v);
		return result;
	}
};


#line 1 "components/radix_sort.hpp"



#line 5 "components/radix_sort.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
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


#line 7 "graph/kruskal.hpp"
#include <limits>
#include <stdexcept>
#line 10 "graph/kruskal.hpp"

template<class Cost = std::int64_t, class Sum = Cost>
class Kruskal {
	static_assert((std::numeric_limits<Cost>::is_integer || std::is_floating_point_v<Cost>) && !std::is_same_v<Cost, bool>);
	static_assert((std::numeric_limits<Sum>::is_integer || std::is_floating_point_v<Sum>) && !std::is_same_v<Sum, bool>);
	static_assert(std::numeric_limits<Cost>::is_integer == std::numeric_limits<Sum>::is_integer);
	static_assert(std::numeric_limits<Cost>::digits <= std::numeric_limits<Sum>::digits);
	static_assert(!std::numeric_limits<Cost>::is_signed || std::numeric_limits<Sum>::is_signed);
	int n_;

  public:
	struct Edge { int from, to; Cost cost; };
	struct Result {
		Sum cost = 0;
		std::vector<int> edges;
		int components = 0;
		bool connected() const { return components <= 1; }
	};

  private:
	std::vector<Edge> edges_;
	static void add_cost(Sum &sum, Cost value) {
		if constexpr (std::numeric_limits<Cost>::is_integer) {
#if defined(__GNUC__) || defined(__clang__)
			Sum next;
			if (__builtin_add_overflow(sum, value, &next)) throw std::overflow_error("Kruskal cost");
			sum = next;
#else
			if ((value > 0 && sum > std::numeric_limits<Sum>::max() - value) ||
				(std::numeric_limits<Cost>::is_signed && value < 0 && sum < std::numeric_limits<Sum>::lowest() - value))
				throw std::overflow_error("Kruskal cost");
			sum += value;
#endif
		} else sum += value;
	}

  public:
	explicit Kruskal(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max()));
		edges_.reserve(m);
	}
	int add_edge(int u, int v, Cost cost) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		if constexpr (std::is_floating_point_v<Cost>) assert(cost == cost);
		int id = edge_count();
		edges_.push_back({u, v, cost});
		return id;
	}
	Edge get_edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	Result solve() const {
		std::vector<int> order;
		order.reserve(edges_.size());
		for (int id = 0; id < edge_count(); ++id) if (edges_[id].from != edges_[id].to) order.push_back(id);
		auto compare = [&](int a, int b) {
			if (edges_[a].cost < edges_[b].cost) return true;
			if (edges_[b].cost < edges_[a].cost) return false;
			return a < b;
		};
		if constexpr (std::is_integral_v<Cost> && sizeof(Cost) <= sizeof(std::uint64_t)) {
			if (order.size() < 256) std::sort(order.begin(), order.end(), compare);
			else if (std::is_sorted(order.begin(), order.end(), [&](int a, int b) { return edges_[b].cost < edges_[a].cost; })) {
				std::reverse(order.begin(), order.end());
				for (std::size_t first = 0; first < order.size();) {
					std::size_t last = first + 1;
					while (last < order.size() && edges_[order[first]].cost == edges_[order[last]].cost) ++last;
					std::reverse(order.begin() + first, order.begin() + last);
					first = last;
				}
			} else {
				using Key = std::make_unsigned_t<Cost>;
				auto sort_key = [&](int id) {
					Key key = static_cast<Key>(edges_[id].cost);
					if constexpr (std::is_signed_v<Cost>) key ^= Key(1) << (std::numeric_limits<Key>::digits - 1);
					return key;
				};
				if (sizeof(Cost) >= 4 && order.size() >= 65536) stable_radix_sort<11>(order, sort_key);
				else stable_radix_sort(order, sort_key);
			}
		} else std::sort(order.begin(), order.end(), compare);
		UnionFind uf(n_);
		Result result;
		result.edges.reserve(std::min(order.size(), std::size_t(n_ ? n_ - 1 : 0)));
		for (int id : order) {
			const auto &e = edges_[id];
			if constexpr (std::is_same_v<Cost, std::int64_t> && std::is_same_v<Sum, Cost>) {
				int components = uf.count();
				uf.merge(e.from, e.to);
				if (uf.count() == components) continue;
				add_cost(result.cost, e.cost);
			} else {
				int u = uf.leader(e.from), v = uf.leader(e.to);
				if (u == v) continue;
				add_cost(result.cost, e.cost);
				uf.merge(u, v);
			}
			result.edges.push_back(id);
			if (uf.count() == 1) break;
		}
		result.components = uf.count();
		return result;
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



#line 6 "components/fast_io.hpp"
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 15 "components/fast_io.hpp"
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


#line 8 "components/fast_io_bulk.hpp"
#include <span>
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


#line 4 "test/minimum_spanning_tree.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	Kruskal<std::uint32_t, std::uint64_t> graph(n);
	graph.reserve(m);
	read_unsigned_records<3>(input, m, [&](unsigned u, unsigned v, unsigned cost) { graph.add_edge(u, v, cost); });
	auto result = graph.solve();
	output.println(result.cost);
	write_unsigned_records<1>(output, result.edges.size(), [&](std::size_t i) { return std::array{unsigned(result.edges[i])}; });
}
