#define DSU_ON_TREE_MODE_TEST
#include "test/benchmark/dsu_on_tree_mode.cpp"
#include <random>

void check(const std::string &text, unsigned expected) {
	FILE *file = std::tmpfile();
	assert(file);
	{
		FastInput input(text);
		FastOutput output(file);
		solve_dsu_on_tree_mode(input, output);
	}
	std::rewind(file);
	unsigned actual = 0;
	assert(std::fscanf(file, "%u", &actual) == 1 && actual == expected);
	char extra;
	assert(std::fscanf(file, " %c", &extra) == EOF);
	assert(!std::ferror(file));
	assert(std::fclose(file) == 0);
}

int main() {
	check("4 454 4 2\n1 2 2\n1 2 2 3\n", 29);
	check("6 123 2 2\n1\n1 2\n", 101);
	check("15 1 4 5\n1 2 3\n5 3 1 3\n", 1199);
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 120; ++trial) {
		int n = 2 + trial % 79, m = trial % 2 ? n : 2, f = trial % 3 ? n : 1;
		std::uint64_t seed = trial % 2 ? 1 : 0x7fffffff, state = seed;
		std::vector<int> parent(n, -1), color(n);
		std::string text = std::to_string(n) + " " + std::to_string(seed) + " "
			+ std::to_string(m) + " " + std::to_string(f) + "\n";
		for (int v = 1; v < n; ++v) {
			if (v < m) {
				parent[v] = trial % 3 == 0 ? v - 1 : trial % 3 == 1 ? 0 : int(random() % v);
				text += std::to_string(parent[v] + 1) + " ";
			} else {
				parent[v] = state % v;
				state = (state * 1103515245 + 12345) % (1ULL << 31);
			}
		}
		text += "\n";
		for (int v = 0; v < n; ++v) {
			if (v < m) {
				color[v] = trial % 3 == 0 ? 0 : trial % 3 == 1 ? v : int(random() % n);
				text += std::to_string(color[v] + 1) + " ";
			} else {
				color[v] = state % f;
				state = (state * 1103515245 + 12345) % (1ULL << 31);
			}
		}
		text += "\n";
		std::vector<std::vector<int>> frequency(n, std::vector<int>(n));
		for (int v = 0; v < n; ++v)
			for (int p = v; p != -1; p = parent[p]) ++frequency[p][color[v]];
		std::uint64_t expected = 0;
		for (int v = 0; v < n; ++v) {
			int maximum = *std::max_element(frequency[v].begin(), frequency[v].end());
			int kinds = std::count(frequency[v].begin(), frequency[v].end(), maximum);
			expected += std::uint64_t(maximum ^ (v + 1)) * (kinds ^ (v + 1));
		}
		check(text, expected % 998244353);
	}
}
