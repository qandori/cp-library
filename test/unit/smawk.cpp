#include "math/smawk.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>

int main() {
	std::mt19937 rng(11);
	assert(smawk(0, 0, [](int, int, int) { return false; }).empty());
	for (int rows = 1; rows < 100; ++rows)
		for (int cols = 1; cols < 100; ++cols) {
			std::vector<std::int64_t> offset(cols);
			for (auto &x : offset) x = int(rng() % 200) - 100;
			auto value = [&](int i, int j) { return std::int64_t(i - j) * (i - j) + offset[j]; };
			int calls = 0;
			auto best = smawk(rows, cols, [&](int i, int x, int y) { ++calls; return value(i, y) < value(i, x); });
			assert(calls < 10 * (rows + cols));
			for (int i = 0; i < rows; ++i) {
				int expected = 0;
				for (int j = 1; j < cols; ++j)
					if (value(i, j) < value(i, expected)) expected = j;
				assert(best[i] == expected);
			}
			std::vector<int> columns;
			for (int j = cols - 1; j >= 0; j -= 2) columns.push_back(j);
			auto reverse_value = [&](int i, int j) { return value(i, cols - 1 - j); };
			best = smawk(rows, columns, [&](int i, int x, int y) { return reverse_value(i, y) < reverse_value(i, x); });
			for (int i = 0; i < rows; ++i) {
				int expected = columns[0];
				for (int j : columns)
					if (reverse_value(i, j) < reverse_value(i, expected)) expected = j;
				assert(best[i] == expected);
			}
		}
	auto ties = smawk(32, 17, [](int, int, int) { return false; });
	for (int j : ties) assert(j == 0);
}
