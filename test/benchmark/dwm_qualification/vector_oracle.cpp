#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Value = __int128_t;
void require(bool condition, const std::string &message) {
	if (!condition) throw std::runtime_error(message);
}
Value value(std::istream &input) {
	std::string s;
	require(bool(input >> s), "missing value");
	std::size_t i = s[0] == '-';
	require(i < s.size() && s.size() <= 21, "invalid value length");
	Value result = 0;
	for (; i < s.size(); ++i) {
		require('0' <= s[i] && s[i] <= '9', "invalid value digit");
		result = result * 10 + s[i] - '0';
	}
	if (s[0] == '-') result = -result;
	require(result >= -(Value(1) << 63) && result < (Value(1) << 64), "value outside supported integral domain");
	return result;
}
std::uint64_t word(std::istream &input) {
	std::array<unsigned char, 8> bytes{};
	require(bool(input.read(reinterpret_cast<char *>(bytes.data()), 8)), "short binary result");
	std::uint64_t result = 0;
	for (int i = 0; i < 8; ++i) result |= std::uint64_t(bytes[i]) << (i * 8);
	return result;
}
int main(int argc, char **argv) {
	try {
		require(argc == 3, "usage: vector_oracle DUMP_PATH KIND");
		std::string path = argv[1], kind = argv[2];
		std::ifstream input(path + ".input"), actual(path, std::ios::binary);
		require(bool(input) && bool(actual), "input or result unavailable");
		int n = 0, q = 0;
		require(bool(input >> n >> q) && n >= 0 && q >= 0, "invalid n or q");
		std::vector<Value> a(n), scratch;
		for (auto &x : a) x = value(input);
		require(word(actual) == std::uint64_t(q) * 2, "output length mismatch");
		auto final_size = word(actual);
		std::array<std::uint64_t, 16> counts{};
		std::uint64_t checks = 0, hits = 0, misses = 0;
		auto check = [&](std::uint64_t expected) {
			auto observed = word(actual);
			if (expected != observed) throw std::runtime_error("result mismatch at word " + std::to_string(checks) + ": expected " + std::to_string(expected) + ", observed " + std::to_string(observed));
			++checks;
		};
		for (int i = 0; i < q; ++i) {
			int type, p, l, r, k;
			require(bool(input >> type >> p >> l >> r >> k), "missing operation fields");
			Value x = value(input), y = value(input), z = 0;
			std::uint64_t tag = 0;
			require(0 <= type && type < 16, "invalid operation type");
			if (kind != "build" && kind != "rebuild") {
				++counts[type];
				int size = int(a.size());
				if (type == 0) require(0 <= p && p <= size, "insert index out of bounds");
				if (type == 1 || type == 2 || type == 6 || type == 12) require(0 <= p && p < size, "point index out of bounds");
				if (type == 3 || type == 4 || type == 5 || type == 8 || type == 9 || type == 10 || type == 11) require(0 <= l && l <= r && r <= size, "range out of bounds");
				switch (type) {
				case 0: a.insert(a.begin() + p, x); break;
				case 1: z = a[p]; a.erase(a.begin() + p); break;
				case 2: a[p] = x; break;
				case 3: case 11: {
					require(0 <= k && k < r - l, "kth index out of bounds");
					scratch.assign(a.begin() + l, a.begin() + r);
					int at = type == 3 ? k : r - l - 1 - k;
					std::nth_element(scratch.begin(), scratch.begin() + at, scratch.end());
					z = scratch[at]; break;
				}
				case 4: for (int j = l; j < r; ++j) z += a[j] < x; break;
				case 5: for (int j = l; j < r; ++j) z += a[j] == x; break;
				case 6: case 12: z = a[p]; break;
				case 7: {
					z = -1;
					if (k >= 0) for (int j = 0, occurrence = 0; j < size; ++j) if (a[j] == x && occurrence++ == k) { z = j; break; }
					if (z < 0) ++misses; else ++hits;
					break;
				}
				case 8: for (int j = l; j < r; ++j) if (a[j] < x && (!tag || a[j] > z)) { z = a[j]; tag = 1; } break;
				case 9: for (int j = l; j < r; ++j) if (a[j] >= x && (!tag || a[j] < z)) { z = a[j]; tag = 1; } break;
				case 10: for (int j = l; j < r; ++j) z += x <= a[j] && a[j] < y; break;
				case 13: require(0 <= r && r <= size, "rank endpoint out of bounds"); for (int j = 0; j < r; ++j) z += a[j] == x; break;
				case 14: a.push_back(x); break;
				case 15: require(!a.empty(), "pop from empty sequence"); z = a.back(); a.pop_back(); break;
				}
			}
			check(std::uint64_t(z)); check(tag);
		}
		input >> std::ws;
		require(input.eof(), "unexpected extra input");
		require(final_size == a.size(), "final state length mismatch");
		for (auto x : a) check(std::uint64_t(x));
		require(actual.peek() == std::char_traits<char>::eof(), "unexpected extra result");
		std::cout << "{\"oracle\":\"full_vector\",\"status\":\"pass\",\"n\":" << n << ",\"q\":" << q << ",\"checked_words\":" << checks << ",\"select_hits\":" << hits << ",\"select_misses\":" << misses << ",\"operation_counts\":[";
		for (int i = 0; i < 16; ++i) std::cout << (i ? "," : "") << counts[i];
		std::cout << "]}\n";
	} catch (const std::exception &error) {
		std::cerr << "vector oracle failed: " << error.what() << '\n';
		return 1;
	}
}
