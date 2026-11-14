#line 1 "test/static_rectangle_add_rectangle_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum"
#line 1 "data_structure/static_rectangle_add_rectangle_sum.hpp"



#line 1 "components/fenwick_sum.hpp"



#include <cassert>
#include <bit>
#include <limits>
#include <vector>

template <class T>
class FenwickSum {
	std::vector<T> data;

  public:
	FenwickSum() = default;
	explicit FenwickSum(int n) { assert(n >= 0); data.resize(n); }
	explicit FenwickSum(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= static_cast<unsigned>(std::numeric_limits<int>::max()));
		data = a;
		constexpr unsigned step = sizeof(T) <= 8 ? 2 : 1;
		for (unsigned i = step; i <= data.size(); i += step) {
			if constexpr (step == 2) data[i - 1] += data[i - 2];
			unsigned p = i + (i & -i);
			if (p <= data.size()) data[p - 1] += data[i - 1];
		}
	}
	int size() const { return static_cast<int>(data.size()); }
	void add(int p, const T &x) {
		assert(0 <= p && p < size());
		for (unsigned i = unsigned(p) + 1; i <= data.size(); i += i & -i) data[i - 1] += x;
	}
	void add_pair(int p, const T &x, int q, const T &y) {
		assert(0 <= p && p < size() && 0 <= q && q < size());
		unsigned a = unsigned(p) + 1, b = unsigned(q) + 1;
		while (a != b && (a <= data.size() || b <= data.size())) {
			if (a < b) data[a - 1] += x, a += a & -a;
			else data[b - 1] += y, b += b & -b;
		}
		T both = x;
		both += y;
		for (; a <= data.size(); a += a & -a) data[a - 1] += both;
	}
	T prefix(int r) const {
		assert(0 <= r && r <= size());
		T result{};
		for (; r; r &= r - 1) result += data[r - 1];
		return result;
	}
	template <class Accum>
	Accum prod_as(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		Accum result{};
		if (l == r) return result;
		unsigned shared = unsigned(l) & (std::numeric_limits<unsigned>::max() << std::bit_width(unsigned(l ^ r)));
		for (; unsigned(l) != shared; l &= l - 1) result -= data[l - 1];
		for (; unsigned(r) != shared; r &= r - 1) result += data[r - 1];
		return result;
	}
	T prod(int l, int r) const { return prod_as<T>(l, r); }
};


#line 1 "components/radix_sort.hpp"



#include <algorithm>
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


#line 1 "components/rectangle_sum_polynomial.hpp"



namespace rectangle_sum_detail {

template <class T>
struct Polynomial {
	T weight{}, x{}, y{}, xy{};
	static Polynomial corner(const T &a, const T &b, const T &value) {
		T ax = value * a, by = value * b;
		return {value, ax, by, ax * b};
	}
	Polynomial &operator+=(const Polynomial &other) {
		weight += other.weight; x += other.x; y += other.y; xy += other.xy;
		return *this;
	}
	Polynomial &operator-=(const Polynomial &other) {
		weight -= other.weight; x -= other.x; y -= other.y; xy -= other.xy;
		return *this;
	}
	T evaluate(const T &a, const T &b) const { return (weight * a - x) * b - y * a + xy; }
};

}


#line 13 "data_structure/static_rectangle_add_rectangle_sum.hpp"

template <class T = std::int64_t, class Coordinate = std::int64_t>
class StaticRectangleAddRectangleSum {
	static_assert(std::is_integral_v<Coordinate> && !std::is_same_v<Coordinate, bool> && sizeof(Coordinate) <= 8);
	using Node = rectangle_sum_detail::Polynomial<T>;
	struct Rectangle { Coordinate left, down, right, up; };
	struct Update { Rectangle rectangle; T value; };
	std::vector<Update> updates_;
	std::vector<Rectangle> queries_;
	void check_count() const { assert(updates_.size() + queries_.size() < std::size_t(std::numeric_limits<int>::max() / 2)); }

  public:
	void add_rectangle(Coordinate left, Coordinate down, Coordinate right, Coordinate up, const T &value) {
		assert(left <= right && down <= up);
		if (left == right || down == up) return;
		check_count();
		updates_.push_back({{left, down, right, up}, value});
	}
	int add_query(Coordinate left, Coordinate down, Coordinate right, Coordinate up) {
		assert(left <= right && down <= up);
		check_count();
		queries_.push_back({left, down, right, up});
		return static_cast<int>(queries_.size()) - 1;
	}
	std::vector<T> solve() const {
		std::vector<T> answers(queries_.size());
		if (updates_.empty() || queries_.empty()) return answers;
		if (std::min(updates_.size(), queries_.size()) <= 4 || updates_.size() <= 4096 / queries_.size()) {
			for (std::size_t i = 0; i < queries_.size(); ++i)
				for (const auto &update : updates_) {
					const auto &a = queries_[i], &b = update.rectangle;
					Coordinate left = std::max(a.left, b.left), right = std::min(a.right, b.right);
					Coordinate down = std::max(a.down, b.down), up = std::min(a.up, b.up);
					if (left < right && down < up) answers[i] += update.value * (T(right) - T(left)) * (T(up) - T(down));
				}
			return answers;
		}
		int n = static_cast<int>(updates_.size()), count = n + static_cast<int>(queries_.size());
		auto rectangle = [&](int id) -> const Rectangle & { return id < n ? updates_[id].rectangle : queries_[id - n]; };
		struct Endpoint { Coordinate y; int id; };
		struct Event { Coordinate x; T down, up, weight; int low, high, id; };
		std::vector<int> ranks(2 * std::size_t(count));
		std::vector<Event> events;
		int size = 0;
		{
			std::vector<Endpoint> endpoints;
			endpoints.reserve(2 * std::size_t(count));
			for (int i = 0; i < count; ++i) {
				const auto &r = rectangle(i);
				endpoints.push_back({r.down, 2 * i}); endpoints.push_back({r.up, 2 * i + 1});
			}
			radix_sort_detail::sort_by_key(endpoints, [](const Endpoint &e) { return e.y; });
			for (std::size_t first = 0; first < endpoints.size();) {
				std::size_t last = first;
				bool update = false;
				while (last < endpoints.size() && endpoints[last].y == endpoints[first].y) {
					ranks[endpoints[last].id] = size;
					update = update || endpoints[last].id < 2 * n;
					++last;
				}
				if (update) ++size;
				first = last;
			}
		}
		events.reserve(2 * std::size_t(count));
		for (int i = 0; i < count; ++i) {
			const auto &r = rectangle(i);
			if (r.left == r.right || r.down == r.up) continue;
			T weight = i < n ? updates_[i].value : T{};
			int id = i < n ? 0 : i - n + 1;
			events.push_back({r.left, T(r.down), T(r.up), weight, ranks[2 * i], ranks[2 * i + 1], -id});
			events.push_back({r.right, T(r.down), T(r.up), T{} - weight, ranks[2 * i], ranks[2 * i + 1], id});
		}
		radix_sort_detail::sort_by_key(events, [](const Event &e) { return e.x; });
		FenwickSum<Node> tree(size);
		for (const auto &event : events) {
			T x(event.x);
			if (!event.id) {
				tree.add_pair(event.low, Node::corner(x, event.down, event.weight), event.high, Node::corner(x, event.up, T{} - event.weight));
			} else {
				T value = tree.prefix(event.high).evaluate(x, event.up) - tree.prefix(event.low).evaluate(x, event.down);
				if (event.id < 0) answers[-event.id - 1] -= value;
				else answers[event.id - 1] += value;
			}
		}
		return answers;
	}
};


#line 1 "math/mint.hpp"


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


#line 5 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 8 "math/mint.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct [[gnu::warn_unused]] Mint {
	using mint = Mint;
	unsigned v = 0;
	[[nodiscard]] static constexpr unsigned mod() { return 998244353; }
	[[nodiscard, gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[nodiscard, gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[nodiscard, gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[nodiscard, gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[nodiscard, gnu::always_inline]] Mint operator+() const { return *this; }
	[[nodiscard, gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[nodiscard, gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[nodiscard, gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[nodiscard, gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	[[nodiscard]] Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[nodiscard, gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[nodiscard, gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	[[nodiscard]] Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[nodiscard, gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if CP_GCC_X86
#pragma GCC pop_options
#endif


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


#line 5 "test/static_rectangle_add_rectangle_sum.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	StaticRectangleAddRectangleSum<mint, int> rectangles;
	for (int i = 0; i < n; ++i) {
		int l = 0, d = 0, r = 0, u = 0, w = 0;
		input.read(l); input.read(d); input.read(r); input.read(u); input.read(w);
		rectangles.add_rectangle(l, d, r, u, mint(w));
	}
	for (int i = 0; i < q; ++i) {
		int l = 0, d = 0, r = 0, u = 0;
		input.read(l); input.read(d); input.read(r); input.read(u);
		rectangles.add_query(l, d, r, u);
	}
	for (auto answer : rectangles.solve()) output.println(answer.val());
}
