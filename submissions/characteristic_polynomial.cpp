#line 1 "test/matrix_characteristic_polynomial.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/characteristic_polynomial"
#line 1 "test/fixtures/matrix_common.hpp"



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



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
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


#line 5 "components/fast_io_output.hpp"
#include <limits>
#include <utility>

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


#line 1 "math/matrix.hpp"



#line 1 "components/matrix_arithmetic.hpp"



#line 1 "components/simd_capabilities.hpp"


#line 4 "components/simd_capabilities.hpp"

#if CP_GCC_X86
_Pragma("GCC push_options")
_Pragma("GCC target(\"no-avx512f,avx512f\")")
#ifdef __AVX512F__
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif
_Pragma("GCC pop_options")
#elif CP_X86 && defined(__clang__)
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif


#line 1 "components/matrix_base_arithmetic.hpp"



#line 1 "components/field_base.hpp"



#include <cmath>
#line 6 "components/field_base.hpp"

namespace field_detail {
template <class T>
struct Field {
	using V = T;
	static T add(T a, T b) { return a + b; }
	static T mul(T a, T b) { return a * b; }
	static T neg(T a) { return -a; }
	static T inv(T a) { return T(1) / a; }
	static bool zero(T a, long double eps) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) <= eps;
		else { (void)eps; return a == T(0); }
	}
	static bool better(T a, T b) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) > std::abs(b);
		else { (void)a; (void)b; return false; }
	}
	static T dot(const T *a, const T *b, int n) {
		T result = 0;
		for (int i = 0; i < n; ++i) result += a[i] * b[i];
		return result;
	}
	static void scale(T *a, int n, T x) { for (int i = 0; i < n; ++i) a[i] *= x; }
	static void axpy(T *a, const T *b, int n, T x) { for (int i = 0; i < n; ++i) a[i] += x * b[i]; }
};

}


#line 12 "components/matrix_base_arithmetic.hpp"
#include <vector>

namespace matrix_detail {
template <class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;

using field_detail::Field;

template <class Ops>
struct Elimination {
	std::vector<int> pivots;
	typename Ops::V determinant = 1;
};

template <class Ops, bool exact = false>
Elimination<Ops> eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
	using V = typename Ops::V;
	Elimination<Ops> result;
	result.pivots.reserve(std::min(h, columns));
	int rank = 0;
	for (int col = 0; col < columns && rank < h; ++col) {
		int p = -1;
		for (int i = rank; i < h; ++i) {
			V x = a[std::size_t(i) * stride + col];
			if (!Ops::zero(x, eps) && (p < 0 || Ops::better(x, a[std::size_t(p) * stride + col]))) p = i;
		}
		if (p < 0) continue;
		V *row = a.data() + std::size_t(rank) * stride;
		if (p != rank) {
			std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
			result.determinant = Ops::neg(result.determinant);
		}
		result.determinant = Ops::mul(result.determinant, row[col]);
		Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
		row[col] = V(1);
		for (int i = rank + 1; i < h; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps)) Ops::axpy(dst + col + 1, row + col + 1, stride - col - 1, Ops::neg(x));
			dst[col] = V(0);
		}
		result.pivots.push_back(col);
		++rank;
	}
	bool full = exact && rank == columns;
	if (reduced) for (int k = rank - 1; k >= 0; --k) {
		int col = result.pivots[k], start = full ? columns : col + 1;
		const V *row = a.data() + std::size_t(k) * stride;
		for (int i = 0; i < k; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps) && (!full || start < stride)) Ops::axpy(dst + start, row + start, stride - start, Ops::neg(x));
			dst[col] = V(0);
		}
	}
	return result;
}

template <class T>
[[gnu::noinline]] T integer_determinant(const std::vector<T> &a, int n) {
	static_assert(std::numeric_limits<T>::is_signed);
	using W = __int128_t;
	std::vector<W> work(a.begin(), a.end());
	W previous = 1, sign = 1;
	if (!n) return T(1);
	for (int k = 0; k + 1 < n; ++k) {
		int pivot = k;
		while (pivot < n && work[std::size_t(pivot) * n + k] == 0) ++pivot;
		if (pivot == n) return T(0);
		if (pivot != k) { for (int j = 0; j < n; ++j) std::swap(work[std::size_t(k) * n + j], work[std::size_t(pivot) * n + j]); sign = -sign; }
		W p = work[std::size_t(k) * n + k];
		for (int i = k + 1; i < n; ++i) {
			for (int j = k + 1; j < n; ++j) {
				W value = work[std::size_t(i) * n + j] * p - work[std::size_t(i) * n + k] * work[std::size_t(k) * n + j];
				assert(value % previous == 0);
				work[std::size_t(i) * n + j] = value / previous;
			}
			work[std::size_t(i) * n + k] = 0;
		}
		previous = p;
	}
	W result = sign * work.back();
	assert(result >= std::numeric_limits<T>::lowest() && result <= std::numeric_limits<T>::max());
	return T(result);
}

template <class T>
std::vector<T> basic_multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
		const T &x = a[std::size_t(i) * inner + k];
		for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
	}
	return c;
}
}


#line 1 "components/field_arithmetic.hpp"



#line 10 "components/field_arithmetic.hpp"
#if CP_GNU_X86
#include <immintrin.h>
#endif

namespace field_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<T>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 31))
		&& std::is_convertible_v<decltype(std::declval<T>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

template <class T>
inline constexpr bool small_modint = SmallModint<T>::value;

inline bool avx2() {
#if CP_GNU_X86 && !defined(FIELD_DISABLE_AVX2) && !defined(MATRIX_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

template <class T>
struct ModularField {
	using V = std::uint32_t;
	static constexpr V mod = T::mod();
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V mul(V a, V b) { return std::uint64_t(a) * b % mod; }
	static V neg(V a) { return a ? mod - a : 0; }
	static V dot(const V *a, const V *b, int n) {
		constexpr int step = mod <= (V(1) << 30) ? 16 : 4;
		std::uint64_t sum = 0;
		for (int i = 0; i < n; i += step) {
			for (int j = i; j < std::min(n, i + step); ++j) sum += std::uint64_t(a[j]) * b[j];
			sum %= mod;
		}
		return sum;
	}
	static V inv(V a) { return (T(1) / T::raw(a)).val(); }
	static bool zero(V a, long double) { return !a; }
	static bool better(V, V) { return false; }
	static V product(V a, V x, V q) {
		V r = a * x - V((std::uint64_t(a) * q) >> 32) * mod;
		return r < mod ? r : r - mod;
	}
#if CP_GNU_X86
	__attribute__((target("avx2"))) static __m256i product8(__m256i a, __m256i x, __m256i q) {
		__m256i p = _mm256_set1_epi32(mod);
		__m256i even = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		__m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		__m256i high = _mm256_or_si256(even, _mm256_slli_epi64(_mm256_srli_epi64(odd, 32), 32));
		__m256i r = _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(high, p));
		return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
	}
#if CP_LIBRARY_AVX512
	__attribute__((target("avx512f"))) static __m512i product16(__m512i a, __m512i x, __m512i q) {
		const auto p = _mm512_set1_epi32(mod);
		auto even = _mm512_srli_epi64(_mm512_mul_epu32(a, q), 32);
		auto odd = _mm512_mul_epu32(_mm512_srli_epi64(a, 32), q);
		auto high = _mm512_mask_blend_epi32(0xaaaa, even, odd);
		auto r = _mm512_sub_epi32(_mm512_mullo_epi32(a, x), _mm512_mullo_epi32(high, p));
		return _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
	}
	template <bool add_to>
	__attribute__((target("avx512f"))) static void apply16(V *a, const V *b, int n, V x, V q) {
		const auto vx = _mm512_set1_epi32(x), vq = _mm512_set1_epi32(q), p = _mm512_set1_epi32(mod);
		int i = 0;
		for (; i + 16 <= n; i += 16) {
			auto r = product16(_mm512_loadu_si512(b + i), vx, vq);
			if constexpr (add_to) {
				r = _mm512_add_epi32(r, _mm512_loadu_si512(a + i));
				r = _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
			}
			_mm512_storeu_si512(a + i, r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	__attribute__((target("avx2"))) static void apply8(V *a, const V *b, int n, V x, V q) {
		__m256i vx = _mm256_set1_epi32(x), vq = _mm256_set1_epi32(q), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			__m256i r = product8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), vx, vq);
			if constexpr (add_to) {
				r = _mm256_add_epi32(r, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)));
				r = _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	static void apply(V *a, const V *b, int n, V x) {
		V q = (std::uint64_t(x) << 32) / mod;
#if CP_GNU_X86
#if CP_LIBRARY_AVX512
		if (n >= 64 && avx2() && __builtin_cpu_supports("avx512f")) { apply16<add_to>(a, b, n, x, q); return; }
#endif
		if (n >= 16 && avx2()) { apply8<add_to>(a, b, n, x, q); return; }
#endif
		for (int i = 0; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	static void scale(V *a, int n, V x) { apply<false>(a, a, n, x); }
	static void axpy(V *a, const V *b, int n, V x) { apply<true>(a, b, n, x); }
};

}


#line 7 "components/matrix_arithmetic.hpp"

#line 16 "components/matrix_arithmetic.hpp"
#if CP_X86
#include <immintrin.h>
#endif

namespace matrix_detail {
using field_detail::SmallModint, field_detail::small_modint, field_detail::avx2, field_detail::ModularField;

template <class T, class F>
auto field(std::vector<T> &a, F &&f) {
	static_assert(!integer<T>);
	if constexpr (small_modint<T>) {
		std::vector<std::uint32_t> work(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) work[i] = a[i].val();
		auto result = f.template operator()<ModularField<T>>(work);
		for (std::size_t i = 0; i < a.size(); ++i) a[i] = T::raw(work[i]);
		return result;
	} else return f.template operator()<Field<T>>(a);
}

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <std::uint32_t mod>
void product_scalar(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; j += 32) {
		int width = std::min(32, m - j);
		for (int k = 0; k < inner; k += step) {
			std::uint64_t sums[32]{};
			for (int t = k; t < std::min(inner, k + step); ++t) {
				std::uint64_t x = a[std::size_t(i) * inner + t];
				const auto *row = b + std::size_t(t) * m + j;
				for (int s = 0; s < width; ++s) sums[s] += x * row[s];
			}
			for (int s = 0; s < width; ++s) c[std::size_t(i) * m + j + s] = (sums[s] + c[std::size_t(i) * m + j + s]) % mod;
		}
	}
}

#if CP_GNU_X86
inline bool avx512_product() {
#if CP_LIBRARY_AVX512
	static const bool supported = __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512dq");
	return avx2() && supported;
#else
	return false;
#endif
}

#if CP_LIBRARY_AVX512
template <std::uint32_t mod>
__attribute__((target("avx512f,avx512dq"))) __m512i reduce_product_avx512(__m512i sum) {
	const auto p = _mm512_set1_epi64(mod);
	const auto q = _mm512_cvt_roundpd_epi64(
		_mm512_mul_pd(_mm512_cvtepi64_pd(sum), _mm512_set1_pd(1.0 / mod)),
		_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
	auto r = _mm512_sub_epi64(sum, _mm512_mullo_epi64(q, p));
	r = _mm512_mask_add_epi64(r, _mm512_cmp_epi64_mask(r, _mm512_setzero_si512(), _MM_CMPINT_LT), r, p);
	return _mm512_mask_sub_epi64(r, _mm512_cmp_epi64_mask(r, p, _MM_CMPINT_NLT), r, p);
}

template <std::uint32_t mod, int rows = 8>
__attribute__((target("avx512f,avx512dq"))) void product_packed_avx512(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr std::int64_t residue = (std::uint64_t(1) << 32) % mod;
	constexpr std::int64_t folded_bound = (std::int64_t(1) << 31) * residue + 0xffffffffu;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - folded_bound) / (radius * radius)));
	int i = 0;
	for (; i + rows <= n; i += rows) {
		for (int j = 0; j < m; j += 16) {
			const int width = std::min(16, m - j);
			const __mmask16 active = (1u << width) - 1;
			__m512i sums[rows][2];
			for (int r = 0; r < rows; ++r) {
				auto previous = _mm512_maskz_loadu_epi32(active, c + std::size_t(i + r) * m + j);
				sums[r][0] = _mm512_and_si512(previous, _mm512_set1_epi64(0xffffffffu));
				sums[r][1] = _mm512_srli_epi64(previous, 32);
			}
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) {
					auto x = _mm512_maskz_loadu_epi32(active, b + std::size_t(j) * inner + std::size_t(t) * width);
					auto y = _mm512_srli_epi64(x, 32);
					for (int r = 0; r < rows; ++r) {
						auto z = _mm512_set1_epi32(a[std::size_t(i + r) * inner + t]);
						sums[r][0] = _mm512_add_epi64(sums[r][0], _mm512_mul_epi32(x, z));
						sums[r][1] = _mm512_add_epi64(sums[r][1], _mm512_mul_epi32(y, z));
					}
				}
				for (int r = 0; r < rows; ++r) for (int q = 0; q < 2; ++q) {
					auto x = sums[r][q];
					sums[r][q] = _mm512_add_epi64(_mm512_mul_epi32(_mm512_srli_epi64(x, 32), _mm512_set1_epi32(residue)), _mm512_and_si512(x, _mm512_set1_epi64(0xffffffffu)));
				}
			}
			for (int r = 0; r < rows; ++r) {
				auto even = reduce_product_avx512<mod>(sums[r][0]), odd = reduce_product_avx512<mod>(sums[r][1]);
				_mm512_mask_storeu_epi32(c + std::size_t(i + r) * m + j, active, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
			}
		}
	}
	if constexpr (rows > 1) {
		if (i < n) product_packed_avx512<mod, rows / 2>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	}
}
#endif

template <std::uint32_t mod, int rows = 4>
__attribute__((target("avx2"))) void product_packed_avx2(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr std::int64_t residue = (std::uint64_t(1) << 32) % mod;
	constexpr std::int64_t folded_bound = (std::int64_t(1) << 31) * residue + 0xffffffffu;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - folded_bound) / (radius * radius)));
	auto reduce = [](std::int64_t sum) {
		std::int64_t r = sum % std::int64_t(mod);
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	int i = 0;
	for (; i + rows <= n; i += rows) {
		for (int j = 0; j < m; j += 8) {
			int width = std::min(8, m - j);
			auto active = _mm256_cmpgt_epi32(_mm256_set1_epi32(width), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
			__m256i sums[rows][2];
			for (int r = 0; r < rows; ++r) {
				auto previous = _mm256_maskload_epi32(reinterpret_cast<const int *>(c + std::size_t(i + r) * m + j), active);
				sums[r][0] = _mm256_and_si256(previous, _mm256_set1_epi64x(0xffffffffu));
				sums[r][1] = _mm256_srli_epi64(previous, 32);
			}
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) {
					auto x = _mm256_maskload_epi32(b + std::size_t(j) * inner + std::size_t(t) * width, active);
					auto y = _mm256_srli_epi64(x, 32);
					for (int r = 0; r < rows; ++r) {
						auto z = _mm256_set1_epi32(a[std::size_t(i + r) * inner + t]);
						sums[r][0] = _mm256_add_epi64(sums[r][0], _mm256_mul_epi32(x, z));
						sums[r][1] = _mm256_add_epi64(sums[r][1], _mm256_mul_epi32(y, z));
					}
				}
				for (int r = 0; r < rows; ++r) for (int q = 0; q < 2; ++q) {
					auto x = sums[r][q];
					sums[r][q] = _mm256_add_epi64(_mm256_mul_epi32(_mm256_srli_epi64(x, 32), _mm256_set1_epi32(residue)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffu)));
				}
			}
			for (int r = 0; r < rows; ++r) {
				std::int64_t even[4], odd[4];
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), sums[r][0]);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), sums[r][1]);
				std::uint32_t result[8];
				for (int q = 0; q < 4; ++q) result[q * 2] = reduce(even[q]), result[q * 2 + 1] = reduce(odd[q]);
				_mm256_maskstore_epi32(reinterpret_cast<int *>(c + std::size_t(i + r) * m + j), active, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(result)));
			}
		}

	}
	if constexpr (rows == 4) {
		if (i < n) product_packed_avx2<mod, 1>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	}
}

#endif

template <class T>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	if (!n || !inner || !m) return c;
	auto basic = [&] {
		for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
			const T &x = a[std::size_t(i) * inner + k];
			for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
		}
	};
	if constexpr (small_modint<T>) {
		if (std::uint64_t(n) * m <= 128u / unsigned(inner)) { basic(); return c; }
#if CP_GNU_X86
		if (inner >= 16 && m >= 8 && avx2()) {
			const int block = avx512_product() ? 16 : 8;
			std::vector<std::int32_t> x(a.size()), y(b.size());
			std::vector<std::uint32_t> z(c.size());
			auto center = [](std::uint32_t value) {
				std::int32_t v = value;
				return v <= std::int32_t((T::mod() - 1) / 2) ? v : v - std::int32_t(T::mod());
			};
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = center(a[i].val());
			for (int j = 0; j < m; j += block) {
				int width = std::min(block, m - j);
				for (int k = 0; k < inner; ++k) for (int s = 0; s < width; ++s)
					y[std::size_t(j) * inner + std::size_t(k) * width + s] = center(b[std::size_t(k) * m + j + s].val());
			}
#if CP_LIBRARY_AVX512
			if (block == 16) product_packed_avx512<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			else
#endif
			product_packed_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
			return c;
		}
#endif
		std::vector<std::uint32_t> x(a.size()), y(b.size()), z(c.size());
		for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
		for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
		product_scalar<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
	} else basic();
	return c;
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif
}


#line 1 "components/matrix_power.hpp"



#line 1 "components/matrix_elimination_modular.hpp"



#line 7 "components/matrix_elimination_modular.hpp"

namespace matrix_detail {
template <class Ops>
struct ModularElimination {
	using V = typename Ops::V;
	static void hessenberg(std::vector<V> &b, int n, int k, V inverse, std::vector<V> &factors) {
		bool any = false;
		for (int i = k + 1; i < n; ++i) {
			V factor = factors[i] = Ops::mul(b[std::size_t(i) * n + k - 1], inverse);
			if (!factor) continue;
			any = true;
			Ops::axpy(b.data() + std::size_t(i) * n + k, b.data() + std::size_t(k) * n + k, n - k, Ops::neg(factor));
			b[std::size_t(i) * n + k - 1] = 0;
		}
		if (!any) return;
		for (int j = 0; j < n; ++j) {
			auto *row = b.data() + std::size_t(j) * n;
			row[k] = Ops::add(row[k], Ops::dot(row + k + 1, factors.data() + k + 1, n - k - 1));
		}
	}

	static constexpr int batch = int(std::min<std::uint64_t>(8, (std::numeric_limits<std::int64_t>::max() - (Ops::mod - 1)) / (std::uint64_t(Ops::mod - 1) * (Ops::mod - 1))));
#if CP_GNU_X86
#if CP_LIBRARY_AVX512
	__attribute__((target("avx512f,avx512dq"))) static __m512i reduce512(__m512i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm512_add_epi64(sum, _mm512_mul_epu32(_mm512_mul_epu32(sum, _mm512_set1_epi32(negative_inverse)), _mm512_set1_epi32(Ops::mod)));
		sum = _mm512_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm512_min_epu32(sum, _mm512_sub_epi32(sum, _mm512_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx512f,avx512dq"))) static void update512(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 16 <= length; j += 16) {
			auto x = _mm512_loadu_si512(dst + j);
			auto even = _mm512_setzero_si512(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm512_loadu_si512(src[k] + first + j), b = _mm512_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm512_add_epi64(even, _mm512_mul_epu32(a, b));
				odd = _mm512_add_epi64(odd, _mm512_mul_epu32(_mm512_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) even = reduce512(even), odd = reduce512(odd);
			else even = reduce_product_avx512<Ops::mod>(even), odd = reduce_product_avx512<Ops::mod>(odd);
			x = _mm512_add_epi32(x, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
			_mm512_storeu_si512(dst + j, _mm512_min_epu32(x, _mm512_sub_epi32(x, _mm512_set1_epi32(Ops::mod))));
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
#endif
	__attribute__((target("avx2"))) static __m256i reduce256(__m256i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm256_add_epi64(sum, _mm256_mul_epu32(_mm256_mul_epu32(sum, _mm256_set1_epi32(negative_inverse)), _mm256_set1_epi32(Ops::mod)));
		sum = _mm256_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx2"))) static void update256(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 8 <= length; j += 8) {
			auto even = _mm256_setzero_si256(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src[k] + first + j));
				auto b = _mm256_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(a, b));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) {
				even = reduce256(even), odd = reduce256(odd);
				auto value = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(dst + j)), _mm256_or_si256(even, _mm256_slli_epi64(odd, 32)));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(dst + j), _mm256_min_epu32(value, _mm256_sub_epi32(value, _mm256_set1_epi32(Ops::mod))));
			} else {
				std::uint64_t x[4], y[4];
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(x), even);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(y), odd);
				for (int k = 0; k < 4; ++k) {
					dst[j + 2 * k] = (x[k] + dst[j + 2 * k]) % Ops::mod;
					dst[j + 2 * k + 1] = (y[k] + dst[j + 2 * k + 1]) % Ops::mod;
				}
			}
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
#endif
	static void update(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		if (!count || !length || std::none_of(coeff, coeff + count, [](V x) { return x != 0; })) return;
		if (count == 1) { if (coeff[0]) Ops::axpy(dst, src[0] + first, length, coeff[0]); return; }
#if CP_GNU_X86
#if CP_LIBRARY_AVX512
		if (length >= 32 && avx512_product()) { update512(dst, src, coeff, count, first, length); return; }
#endif
		if (length >= 16 && avx2()) { update256(dst, src, coeff, count, first, length); return; }
#endif
		for (int j = 0; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
	static Elimination<Ops> run(std::vector<V> &a, int h, int stride, int columns, bool reduced) {
		Elimination<Ops> result;
		result.pivots.reserve(std::min(h, columns));
		std::vector<std::array<V, batch>> factors(h);
		std::array<const V *, batch> source;
		int rank = 0;
		for (int first = 0; first < columns && rank < h; first += batch) {
			int end = std::min(columns, first + batch), start = rank;
			for (int i = rank; i < h; ++i) factors[i].fill(0);
			for (int col = first; col < end && rank < h; ++col) {
				int p = rank;
				while (p < h && !a[std::size_t(p) * stride + col]) ++p;
				if (p == h) continue;
				V *row = a.data() + std::size_t(rank) * stride;
				if (p != rank) {
					std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
					std::swap(factors[p], factors[rank]);
					result.determinant = Ops::neg(result.determinant);
				}
				update(row + end, source.data(), factors[rank].data(), rank - start, end, stride - end);
				result.determinant = Ops::mul(result.determinant, row[col]);
				Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
				row[col] = 1;
				source[rank - start] = row;
				for (int i = rank + 1; i < h; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					V x = factors[i][rank - start] = Ops::neg(dst[col]);
					if (x) Ops::axpy(dst + col + 1, row + col + 1, end - col - 1, x);
					dst[col] = 0;
				}
				result.pivots.push_back(col);
				++rank;
			}
			for (int i = rank; i < h; ++i) update(a.data() + std::size_t(i) * stride + end, source.data(), factors[i].data(), rank - start, end, stride - end);
		}
		bool full = rank == columns;
		if (reduced) for (int end = rank; end > 0;) {
			int start = std::max(0, end - batch);
			for (int k = end - 1; k >= start; --k) {
				int col = result.pivots[k], first = full ? columns : col + 1;
				const V *row = a.data() + std::size_t(k) * stride;
				for (int i = start; i < k; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					if (V x = dst[col]) Ops::axpy(dst + first, row + first, stride - first, Ops::neg(x));
					dst[col] = 0;
				}
			}
			int first = full ? columns : result.pivots[start] + 1;
			for (int k = start; k < end; ++k) source[k - start] = a.data() + std::size_t(k) * stride;
			for (int i = 0; i < start; ++i) {
				V *dst = a.data() + std::size_t(i) * stride;
				for (int k = start; k < end; ++k) factors[i][k - start] = Ops::neg(dst[result.pivots[k]]);
				update(dst + first, source.data(), factors[i].data(), end - start, first, stride - first);
				for (int k = start; k < end; ++k) dst[result.pivots[k]] = 0;
			}
			end = start;
		}
		return result;
	}
};
}


#line 7 "components/matrix_power.hpp"
#include <optional>

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace matrix_detail {
constexpr bool prime_modulus(std::uint32_t p) {
	if (p < 2 || p % 2 == 0) return p == 2;
	if (p % 3 == 0) return p == 3;
	for (std::uint32_t d = 5; std::uint64_t(d) * d <= p; d += 6)
		if (p % d == 0 || p % (d + 2) == 0) return false;
	return true;
}

template <class T>
struct FrobeniusPower {
	using Ops = ModularField<T>;
	using V = std::uint32_t;
	using Row = std::vector<V>;
	int n, work = 0;
	std::vector<Row> basis, reduced;
	std::vector<int> pivots;
	Row transpose, upper, inverses;
	std::vector<std::int32_t> packed;
	int block = 0;
	std::uint64_t random = 0x243f6a8885a308d3ULL;
	explicit FrobeniusPower(const std::vector<T> &a, int size) : n(size), transpose(a.size()), upper(a.size()), inverses(size) {
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) transpose[std::size_t(j) * n + i] = a[std::size_t(i) * n + j].val();
	}
	static std::int32_t center(V x) {
		return x <= (Ops::mod - 1) / 2 ? std::int32_t(x) : std::int32_t(std::int64_t(x) - Ops::mod);
	}
	Row seed() {
		Row x(n);
		for (auto &v : x) {
			random ^= random << 7;
			random ^= random >> 9;
			v = random % Ops::mod;
		}
		return x;
	}
	Row apply(const Row &x) const {
		Row result(n);
#if CP_GNU_X86
		if (block) {
			std::vector<std::int32_t> centered(n);
			for (int i = 0; i < n; ++i) centered[i] = center(x[i]);
#if CP_LIBRARY_AVX512
			if (block == 16) product_packed_avx512<Ops::mod, 1>(centered.data(), packed.data(), result.data(), 1, n, n);
			else
#endif
			product_packed_avx2<Ops::mod, 1>(centered.data(), packed.data(), result.data(), 1, n, n);
			return result;
		}
#endif
		constexpr int count = ModularElimination<Ops>::batch;
		std::array<const V *, count> source;
		for (int i = 0; i < n; i += count) {
			int used = std::min(count, n - i);
			for (int j = 0; j < used; ++j) source[j] = transpose.data() + std::size_t(i + j) * n;
			ModularElimination<Ops>::update(result.data(), source.data(), x.data() + i, used, 0, n);
		}
		return result;
	}
	std::optional<Row> chain(Row x, int first = 0) {
		while (++work <= 4 * n) {
			const int m = int(basis.size());
			Row y = x, factors(m);
			constexpr int count = ModularElimination<Ops>::batch;
			std::array<const V *, count> source;
			std::array<V, count> coeff;
			for (int j = 0; j < m; j += count) {
				int used = std::min(count, m - j);
				for (int u = 0; u < used; ++u) {
					int p = pivots[j + u];
					std::uint64_t value = y[p];
					for (int k = 0; k < u; ++k) value += std::uint64_t(source[k][p]) * coeff[k];
					V factor = factors[j + u] = value % Ops::mod;
					coeff[u] = Ops::neg(factor);
					source[u] = reduced[j + u].data();
				}
				ModularElimination<Ops>::update(y.data(), source.data(), coeff.data(), used, 0, n);
			}
			int p = 0;
			while (p < n && !y[p]) ++p;
			if (p == n) {
				for (int i = m - 1; i >= first; --i) {
					V sum = Ops::dot(upper.data() + std::size_t(i) * n + i + 1, factors.data() + i + 1, m - i - 1);
					factors[i] = Ops::mul(Ops::add(factors[i], Ops::neg(sum)), inverses[i]);
				}
				for (auto &value : factors) value = Ops::neg(value);
				factors.push_back(1);
				return factors;
			}
			for (int j = 0; j < m; ++j) upper[std::size_t(j) * n + m] = factors[j];
			upper[std::size_t(m) * n + m] = y[p];
			inverses[m] = Ops::inv(y[p]);
			Ops::scale(y.data() + p, n - p, inverses[m]);
			pivots.push_back(p);
			reduced.push_back(std::move(y));
			basis.push_back(std::move(x));
			x = apply(basis.back());
		}
		return std::nullopt;
	}
	static Row power(const Row &relation, std::uint64_t k) {
		const int d = int(relation.size());
		Row p{1};
		for (int bit = int(std::bit_width(k)) - 1; bit >= 0; --bit) {
			const int m = int(p.size());
			Row reversed(p.rbegin(), p.rend()), square(2 * m - 1);
			for (int i = 0; i < int(square.size()); ++i) {
				int first = std::max(0, i - m + 1), last = std::min(m, i + 1);
				square[i] = Ops::dot(p.data() + first, reversed.data() + m - 1 - i + first, last - first);
			}
			for (int i = int(square.size()) - 1; i >= d; --i)
				if (square[i]) Ops::axpy(square.data() + i - d, relation.data(), d, square[i]);
			square.resize(std::min<int>(d, square.size()));
			p = std::move(square);
			if ((k >> bit) & 1) {
				p.insert(p.begin(), 0);
				if (int(p.size()) > d) {
					V lead = p.back();
					p.pop_back();
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
			}
		}
		p.resize(d);
		return p;
	}
	std::optional<std::vector<Row>> decompose(bool diagonal = true) {
#if CP_GNU_X86
		if (n >= 32 && avx2()) {
			block = avx512_product() ? 16 : 8;
			packed.resize(transpose.size());
			for (int j = 0; j < n; j += block) {
				int width = std::min(block, n - j);
				for (int i = 0; i < n; ++i) for (int q = 0; q < width; ++q) packed[std::size_t(j) * n + std::size_t(i) * width + q] = center(transpose[std::size_t(i) * n + j + q]);
			}
		}
#endif
		for (int attempt = 0; attempt < 3; ++attempt) {
			basis.clear(), reduced.clear(), pivots.clear();
			work = 0;
			std::vector<Row> relations;
			bool valid = true;
			while (int(basis.size()) < n) {
				const int start = int(basis.size());
				Row x = seed();
				if (!diagonal) for (int p : pivots) x[p] = 0;
				auto rec = chain(std::move(x), diagonal ? 0 : start);
				if (!rec) { valid = false; break; }
				if (int(basis.size()) == start) {
					Row x(n);
					int p = 0;
					while (std::find(pivots.begin(), pivots.end(), p) != pivots.end()) ++p;
					x[p] = 1;
					rec = chain(std::move(x), diagonal ? 0 : start);
					if (!rec) { valid = false; break; }
				}
				const int d = int(basis.size()) - start;
				if (!diagonal && !start && d <= 2) return std::nullopt;
				if (diagonal && std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
					Row remainder = *rec, quotient(start + 1);
					for (int i = int(remainder.size()) - 1; i >= d; --i) {
						V factor = remainder[i];
						quotient[i - d] = factor;
						Ops::axpy(remainder.data() + i - d, rec->data() + start, d + 1, Ops::neg(factor));
					}
					Row x = basis[start];
					for (int j = 0; j < start; ++j) if (quotient[j]) Ops::axpy(x.data(), basis[j].data(), n, quotient[j]);
					basis.resize(start), reduced.resize(start), pivots.resize(start);
					rec = chain(std::move(x));
					if (!rec || int(basis.size()) != start + d || std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
						valid = false;
						break;
					}
				}
				Row relation(d);
				for (int i = 0; i < d; ++i) relation[i] = Ops::neg((*rec)[start + i]);
				relations.push_back(std::move(relation));
			}
			if (!valid) continue;
			return relations;
		}
		return std::nullopt;
	}
	std::optional<std::vector<T>> characteristic() {
		bool upper_triangular = true, lower_triangular = true;
		for (int i = 0; i < n && (upper_triangular || lower_triangular); ++i) for (int j = 0; j < i; ++j) {
			upper_triangular &= transpose[std::size_t(j) * n + i] == 0;
			lower_triangular &= transpose[std::size_t(i) * n + j] == 0;
		}
		if (upper_triangular || lower_triangular) {
			Row p{1};
			for (int i = 0; i < n; ++i) {
				Row next(p.size() + 1);
				std::copy(p.begin(), p.end(), next.begin() + 1);
				Ops::axpy(next.data(), p.data(), int(p.size()), Ops::neg(transpose[std::size_t(i) * n + i]));
				p = std::move(next);
			}
			std::vector<T> result(p.size());
			for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
			return result;
		}
		auto relations = decompose(false);
		if (!relations) return std::nullopt;
		Row p{1};
		for (const auto &relation : *relations) {
			const int d = int(relation.size());
			Row next(p.size() + d);
			for (int i = 0; i < int(p.size()); ++i) {
				next[i + d] = Ops::add(next[i + d], p[i]);
				if (p[i]) Ops::axpy(next.data() + i, relation.data(), d, Ops::neg(p[i]));
			}
			p = std::move(next);
		}
		std::vector<T> result(p.size());
		for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
		return result;
	}
	std::optional<std::vector<T>> run(std::uint64_t exponent) {
		auto relation_blocks = decompose();
		if (!relation_blocks) return std::nullopt;
		const auto &relations = *relation_blocks;
			std::vector<T> b(std::size_t(n) * n), companion(b.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				b[std::size_t(j) * n + i] = T::raw(basis[i][j]);
			int start = 0;
			for (const auto &relation : relations) {
				const int d = int(relation.size());
				Row p = power(relation, exponent);
				for (int j = 0; j < d; ++j) {
					for (int i = 0; i < d; ++i) companion[std::size_t(start + i) * n + start + j] = T::raw(p[i]);
					V lead = p.back();
					std::move_backward(p.begin(), p.end() - 1, p.end());
					p[0] = 0;
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
				start += d;
			}
			auto result = multiply(b, companion, n, n, n);
			Row columns(result.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				columns[std::size_t(j) * n + i] = result[std::size_t(i) * n + j].val();
			for (int j = 0; j < n; ++j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = 0; i < j; ++i) if (upper[std::size_t(i) * n + j])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(upper[std::size_t(i) * n + j]));
				Ops::scale(column, n, inverses[j]);
			}
			for (int j = n - 1; j >= 0; --j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = j + 1; i < n; ++i) if (reduced[j][pivots[i]])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(reduced[j][pivots[i]]));
			}
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				result[std::size_t(i) * n + pivots[j]] = T::raw(columns[std::size_t(j) * n + i]);
		return result;
	}
};
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 7 "math/matrix.hpp"

namespace matrix_detail {
template <class T>
std::optional<std::pair<T, std::vector<T>>> block_inverse(const std::vector<T> &a, int n, bool adjugate);

template <class T>
struct FastMatrixArithmetic {
	static constexpr bool modular = small_modint<T>;
	template <class Ops>
	static auto eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
		if constexpr (modular) if (h >= 32 && columns >= 32) return ModularElimination<Ops>::run(a, h, stride, columns, reduced);
		return matrix_detail::eliminate<Ops, modular>(a, h, stride, columns, reduced, eps);
	}
	template <class Ops>
	static bool hessenberg(std::vector<typename Ops::V> &a, int n, int k, typename Ops::V inverse, std::vector<typename Ops::V> &factors) {
		if constexpr (modular) if (n >= 16) {
			factors.resize(n);
			ModularElimination<Ops>::hessenberg(a, n, k, inverse, factors);
			return true;
		}
		return false;
	}

	template <class F>
	static auto field(std::vector<T> &a, F &&f) { return matrix_detail::field(a, std::forward<F>(f)); }
	static std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
		return matrix_detail::multiply(a, b, n, inner, m);
	}
	static std::optional<std::pair<T, std::vector<T>>> inverse(const std::vector<T> &a, int n, bool adjugate) {
		if constexpr (modular) return block_inverse(a, n, adjugate);
		return std::nullopt;
	}
	static std::optional<std::vector<T>> characteristic(const std::vector<T> &a, int n) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 32) return FrobeniusPower<T>(a, n).characteristic();
		}
		return std::nullopt;
	}
	static std::optional<std::vector<T>> power(const std::vector<T> &a, int n, std::uint64_t exponent) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 16 && exponent >= 256) return FrobeniusPower<T>(a, n).run(exponent);
		}
		return std::nullopt;
	}
};
}

#line 1 "components/matrix_core.hpp"



#line 9 "components/matrix_core.hpp"
#include <initializer_list>
#line 16 "components/matrix_core.hpp"

template <class T>
struct LinearSolution {
	std::vector<T> particular;
	std::vector<std::vector<T>> basis;
};

namespace matrix_detail {
template <class T> struct FastMatrixArithmetic;
template <class T> struct PlainMatrixArithmetic;
#ifdef CP_MATRIX_LIGHT_API
template <class T> using DefaultMatrixArithmetic = PlainMatrixArithmetic<T>;
#else
template <class T> using DefaultMatrixArithmetic = FastMatrixArithmetic<T>;
#endif
}

template <class T, class Arithmetic = matrix_detail::DefaultMatrixArithmetic<T>>
class [[nodiscard, gnu::warn_unused]] Matrix {
	static_assert(!std::is_same_v<T, bool>);
	int h = 0, w = 0;
	std::vector<T> a;
	std::pair<T, std::vector<int>> reduce(int columns, bool reduced, long double eps) {
		assert(eps >= 0);
		return Arithmetic::field(a, [&]<class Ops>(auto &work) {
			auto res = [&] {
				if constexpr (requires { Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps); })
					return Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps);
				else return matrix_detail::eliminate<Ops, Arithmetic::modular>(work, h, w, columns, reduced, eps);
			}();
			return std::pair{T(res.determinant), std::move(res.pivots)};
		});
	}
	static std::vector<std::vector<T>> nullspace(const Matrix &r, const std::vector<int> &piv, int vars) {
		std::vector<bool> pivot(vars);
		for (int p : piv) pivot[p] = true;
		std::vector<std::vector<T>> basis;
		basis.reserve(vars - piv.size());
		for (int col = 0; col < vars; ++col) if (!pivot[col]) {
			std::vector<T> v(vars, T(0));
			v[col] = T{1};
			for (int i = 0; i < int(piv.size()); ++i) v[piv[i]] = -r(i, col);
			basis.push_back(std::move(v));
		}
		return basis;
	}
	Matrix augment() const {
		static_assert(!matrix_detail::integer<T>);
		assert(h == w && w <= std::numeric_limits<int>::max() / 2);
		Matrix work(h, w * 2);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w + i) = T{1};
		}
		return work;
	}

	std::pair<Matrix, std::vector<int>> echelon(bool reduced, long double eps) const {
		Matrix work = *this;
		auto piv = work.reduce(w, reduced, eps).second;
		return {std::move(work), std::move(piv)};
	}

  public:
	using value_type = T;
	using Solution = LinearSolution<T>;
	static constexpr long double default_epsilon = std::is_floating_point_v<T> ? 1e-12L : 0;
	Matrix() = default;
	explicit Matrix(int n) : Matrix(n, n) {}
	Matrix(int rows, int columns, T value = T(0)) : h(rows), w(columns) {
		assert(rows >= 0 && columns >= 0);
		a.assign(std::size_t(h) * w, value);
	}
	Matrix(std::initializer_list<std::initializer_list<T>> values) : Matrix(int(values.size()), values.size() ? int(values.begin()->size()) : 0) {
		int i = 0;
		for (const auto &row : values) {
			assert(int(row.size()) == w);
			std::copy(row.begin(), row.end(), (*this)[i++].begin());
		}
	}
	[[nodiscard]] explicit Matrix(const std::vector<std::vector<T>> &values) : Matrix(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h; ++i) {
			assert(int(values[i].size()) == w);
			std::copy(values[i].begin(), values[i].end(), (*this)[i].begin());
		}
	}
	Matrix(const Matrix &) = default;
	Matrix &operator=(const Matrix &) = default;
	Matrix(Matrix &&other) noexcept { swap(other); }
	Matrix &operator=(Matrix &&other) noexcept {
		if (this != &other) { Matrix next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(Matrix &other) noexcept { std::swap(h, other.h); std::swap(w, other.w); a.swap(other.a); }
	[[nodiscard]] int rows() const { return h; }
	[[nodiscard]] int cols() const { return w; }
	[[nodiscard]] bool empty() const { return !h || !w; }
	[[nodiscard]] T *data() { return a.data(); }
	[[nodiscard]] const T *data() const { return a.data(); }
	[[nodiscard]] std::span<T> operator[](int i) { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	[[nodiscard]] std::span<const T> operator[](int i) const { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	[[nodiscard]] T &operator()(int i, int j) { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	[[nodiscard]] const T &operator()(int i, int j) const { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	[[nodiscard]] bool operator==(const Matrix &) const = default;
	static Matrix identity(int n) {
		Matrix res(n);
		for (int i = 0; i < n; ++i) res(i, i) = T{1};
		return res;
	}
	void fill(T value) { std::fill(a.begin(), a.end(), value); }
	Matrix transpose() const {
		Matrix res(w, h);
		for (int i = 0; i < h; i += 32) for (int j = 0; j < w; j += 32)
			for (int x = i; x < std::min(h, i + 32); ++x) for (int y = j; y < std::min(w, j + 32); ++y) res(y, x) = (*this)(x, y);
		return res;
	}
	Matrix submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h && 0 <= col_l && col_l <= col_r && col_r <= w);
		Matrix res(row_r - row_l, col_r - col_l);
		for (int i = row_l; i < row_r; ++i) std::copy((*this)[i].begin() + col_l, (*this)[i].begin() + col_r, res[i - row_l].begin());
		return res;
	}
	Matrix operator-() const { Matrix res = *this; for (auto &x : res.a) x = -x; return res; }
	Matrix &operator+=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] += b.a[i]; return *this; }
	Matrix &operator-=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] -= b.a[i]; return *this; }
	Matrix &operator*=(const Matrix &b) { return *this = *this * b; }
	Matrix &operator*=(T x) { for (auto &v : a) v *= x; return *this; }
	Matrix &operator/=(T x) { static_assert(!matrix_detail::integer<T>); assert(x != T(0)); return *this *= T{1} / x; }
	friend Matrix operator+(Matrix a, const Matrix &b) { a += b; return a; }
	friend Matrix operator-(Matrix a, const Matrix &b) { a -= b; return a; }
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		assert(a.w == b.h);
		Matrix res;
		res.h = a.h; res.w = b.w;
		res.a = Arithmetic::multiply(a.a, b.a, a.h, a.w, b.w);
		return res;
	}
	friend Matrix operator*(Matrix a, T x) { a *= x; return a; }
	friend Matrix operator*(T x, Matrix a) { a *= x; return a; }
	friend Matrix operator/(Matrix a, T x) { a /= x; return a; }
	[[nodiscard]] std::vector<T> operator*(const std::vector<T> &v) const {
		assert(int(v.size()) == w);
		return Arithmetic::multiply(a, v, h, w, 1);
	}
	Matrix pow(std::uint64_t exponent) const {
		assert(h == w);
		if (auto power = Arithmetic::power(a, h, exponent)) {
			Matrix res(h);
			res.a = std::move(*power);
			return res;
		}
		return pow(exponent, [](Matrix &) {});
	}
	template <class Normalize>
	Matrix pow(std::uint64_t exponent, Normalize normalize) const {
		assert(h == w);
		Matrix res = identity(h), x = *this;
		normalize(res);
		normalize(x);
		while (exponent) {
			if (exponent & 1) {
				res *= x;
				normalize(res);
			}
			exponent >>= 1;
			if (exponent) {
				x *= x;
				normalize(x);
			}
		}
		return res;
	}
	[[nodiscard]] T trace() const { assert(h == w); T res = T(0); for (int i = 0; i < h; ++i) res += (*this)(i, i); return res; }
	[[nodiscard]] T det(long double eps = default_epsilon) const {
		assert(h == w);
		if constexpr (matrix_detail::integer<T>) {
			return matrix_detail::integer_determinant(a, h);
		} else {
			Matrix work = *this;
			auto [value, piv] = work.reduce(w, false, eps);
			return int(piv.size()) == h ? value : T(0);
		}
	}
	[[nodiscard]] int rank(long double eps = default_epsilon) const { Matrix work = *this; return int(work.reduce(w, false, eps).second.size()); }
	[[nodiscard]] std::pair<Matrix, std::vector<int>> row_echelon(long double eps = default_epsilon) const { return echelon(false, eps); }
	[[nodiscard]] std::pair<Matrix, std::vector<int>> rref(long double eps = default_epsilon) const { return echelon(true, eps); }

	[[nodiscard]] std::optional<Matrix> inverse(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::inverse(a, h, false); }) if (auto fast = Arithmetic::inverse(a, h, false)) {
			if (fast->second.empty()) return std::nullopt;
			Matrix res(h);
			res.a = std::move(fast->second);
			return res;
		}
		auto work = augment();
		auto piv = work.reduce(w, true, eps).second;
		if (int(piv.size()) < h) return std::nullopt;
		return work.submatrix(0, h, w, w * 2);
	}
	[[nodiscard]] std::vector<std::vector<T>> kernel(long double eps = default_epsilon) const {
		auto [work, piv] = rref(eps);
		return nullspace(work, piv, w);
	}
	[[nodiscard]] std::optional<LinearSolution<T>> solve(const std::vector<T> &b, long double eps = default_epsilon) const {
		assert(int(b.size()) == h && w < std::numeric_limits<int>::max());
		Matrix work(h, w + 1);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w) = b[i];
		}
		auto piv = work.reduce(w, true, eps).second;
		for (int i = int(piv.size()); i < h; ++i) if (!matrix_detail::Field<T>::zero(work(i, w), eps)) return std::nullopt;
		LinearSolution<T> res{std::vector<T>(w, T(0)), nullspace(work, piv, w)};
		for (int i = 0; i < int(piv.size()); ++i) res.particular[piv[i]] = work(i, w);
		return res;
	}
	[[nodiscard]] std::vector<T> characteristic_polynomial(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::characteristic(a, h); })
			if (auto res = Arithmetic::characteristic(a, h)) return std::move(*res);
		auto work = a;
		return Arithmetic::field(work, [&]<class Ops>(auto &b) {
			using V = typename Ops::V;
			auto at = [&](int i, int j) -> V& { return b[std::size_t(i) * w + j]; };
			std::vector<V> factors;
			for (int k = 1; k < h; ++k) {
				int pivot = -1;
				for (int i = k; i < h; ++i) if (!Ops::zero(at(i, k - 1), eps)
					&& (pivot < 0 || Ops::better(at(i, k - 1), at(pivot, k - 1)))) pivot = i;
				if (pivot < 0) continue;
				if (pivot != k) {
					for (int j = 0; j < w; ++j) std::swap(at(k, j), at(pivot, j));
					for (int i = 0; i < h; ++i) std::swap(at(i, k), at(i, pivot));
				}
				V inverse = Ops::inv(at(k, k - 1));
				if constexpr (requires { Arithmetic::template hessenberg<Ops>(b, h, k, inverse, factors); })
					if (Arithmetic::template hessenberg<Ops>(b, h, k, inverse, factors)) continue;

				for (int i = k + 1; i < h; ++i) {
					V factor = Ops::mul(at(i, k - 1), inverse);
					if (Ops::zero(factor, eps)) continue;
					Ops::axpy(b.data() + std::size_t(i) * w + k, b.data() + std::size_t(k) * w + k, w - k, Ops::neg(factor));
					at(i, k - 1) = V(0);
					for (int j = 0; j < h; ++j) at(j, k) = Ops::add(at(j, k), Ops::mul(factor, at(j, i)));
				}
			}
			std::vector<std::vector<V>> p(h + 1);
			p[0] = {V(1)};
			for (int n = 1; n <= h; ++n) {
				p[n].assign(n + 1, V(0));
				std::copy(p[n - 1].begin(), p[n - 1].end(), p[n].begin() + 1);
				Ops::axpy(p[n].data(), p[n - 1].data(), n, Ops::neg(at(n - 1, n - 1)));
				V product = 1;
				for (int i = n - 2; i >= 0; --i) {
					product = Ops::mul(product, at(i + 1, i));
					V factor = Ops::mul(product, at(i, n - 1));
					Ops::axpy(p[n].data(), p[i].data(), i + 1, Ops::neg(factor));
				}
			}
			std::vector<T> res(h + 1);
			for (int i = 0; i <= h; ++i) res[i] = T(p[h][i]);
			return res;
		});
	}
	Matrix adjugate(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::inverse(a, h, true); }) if (auto fast = Arithmetic::inverse(a, h, true)) {
			Matrix res(h);
			res.a = std::move(fast->second);
			return res;
		}
		auto work = augment();
		auto [det, piv] = work.reduce(w, true, eps);
		if (int(piv.size()) == h) return work.submatrix(0, h, w, w * 2) * det;
		Matrix res(h);
		if (int(piv.size()) + 1 < h) return res;
		auto right = nullspace(work, piv, w)[0];
		int col = 0;
		while (std::find(piv.begin(), piv.end(), col) != piv.end()) ++col;
		if ((h - 1 - col) & 1) det = -det;
		for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) res(i, j) = det * right[i] * work(h - 1, w + j);
		return res;
	}
};


#line 1 "components/matrix_block_inverse.hpp"



#line 7 "components/matrix_block_inverse.hpp"

namespace matrix_detail {
template <class T>
struct GaussianMatrixArithmetic {
	static constexpr bool modular = small_modint<T>;
	template <class Ops>
	static auto eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double) {
		return ModularElimination<Ops>::run(a, h, stride, columns, reduced);
	}
	template <class F>
	static auto field(std::vector<T> &a, F &&f) { return matrix_detail::field(a, std::forward<F>(f)); }
};

template <class T>
std::optional<std::pair<T, std::vector<T>>> block_inverse(const std::vector<T> &a, int n, bool adjugate) {
	if (n < 112) return std::nullopt;
	int h = n / 2, k = n - h;
	for (int i = 0; i < h; ++i) {
		bool row = false, column = false;
		for (int j = 0; j < h; ++j) {
			row |= a[std::size_t(i) * n + j] != T(0);
			column |= a[std::size_t(j) * n + i] != T(0);
		}
		if (!row || !column) return std::nullopt;
	}
	auto extract = [&](int r, int c, int rows, int cols) {
		std::vector<T> block(std::size_t(rows) * cols);
		for (int i = 0; i < rows; ++i) std::copy_n(a.data() + std::size_t(r + i) * n + c, cols, block.data() + std::size_t(i) * cols);
		return block;
	};
	auto solve = [](const std::vector<T> &block, int size, bool adjugate) {
		if (auto fast = block_inverse(block, size, adjugate)) return std::move(*fast);
		Matrix<T, GaussianMatrixArithmetic<T>> matrix(size);
		std::copy(block.begin(), block.end(), matrix.data());
		if (adjugate) {
			auto result = matrix.adjugate();
			return std::pair{matrix.det(), std::vector<T>(result.data(), result.data() + block.size())};
		}
		auto result = matrix.inverse();
		if (!result) return std::pair{T(0), std::vector<T>{}};
		return std::pair{matrix.det(), std::vector<T>(result->data(), result->data() + block.size())};
	};
	auto [da, ai] = solve(extract(0, 0, h, h), h, false);
	if (ai.empty()) return std::nullopt;
	auto b = extract(0, h, h, k), c = extract(h, 0, k, h), d = extract(h, h, k, k);
	auto v = multiply(c, ai, k, h, h), vb = multiply(v, b, k, h, k);
	for (std::size_t i = 0; i < d.size(); ++i) d[i] -= vb[i];
	auto [ds, si] = solve(d, k, adjugate);
	if (si.empty()) return std::pair{T(0), std::vector<T>{}};
	auto u = multiply(ai, b, h, h, k);
	auto r = multiply(u, si, h, k, k), t = multiply(si, v, k, k, h);
	auto rv = multiply(r, v, h, k, h);
	for (std::size_t i = 0; i < ai.size(); ++i) ai[i] = (adjugate ? ai[i] * ds : ai[i]) + rv[i];
	std::vector<T> result(a.size());
	for (int i = 0; i < h; ++i) {
		std::copy_n(ai.data() + std::size_t(i) * h, h, result.data() + std::size_t(i) * n);
		for (int j = 0; j < k; ++j) result[std::size_t(i) * n + h + j] = -r[std::size_t(i) * k + j];
	}
	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < h; ++j) result[std::size_t(h + i) * n + j] = -t[std::size_t(i) * h + j];
		std::copy_n(si.data() + std::size_t(i) * k, k, result.data() + std::size_t(h + i) * n + h);
	}
	if (adjugate) for (auto &x : result) x *= da;
	return std::pair{da * ds, std::move(result)};
}

}


#line 56 "math/matrix.hpp"


#line 1 "math/mint.hpp"


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


#line 7 "test/fixtures/matrix_common.hpp"

inline Matrix<mint> read_matrix(FastInput &in, int n, int m) {
	Matrix<mint> a(n, m);
	auto *next = a.data();
	read_unsigned_blocks(in, std::size_t(n) * m, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) next[i] = mint::raw(values[i]);
		next += count;
	});
	return a;
}

inline void write_matrix(FastOutput &out, const Matrix<mint> &a) {
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) { out.write(a(i, j).val()); out.write(j + 1 == a.cols() ? '\n' : ' '); }
}


#line 3 "test/matrix_characteristic_polynomial.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	auto p = read_matrix(in, n, n).characteristic_polynomial();
	for (int i = 0; i <= n; ++i) { out.write(p[i].val()); out.write(i == n ? '\n' : ' '); }
}
