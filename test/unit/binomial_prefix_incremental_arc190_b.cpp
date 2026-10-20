#define BINOMIAL_PREFIX_INCREMENTAL_ARC190_B_TEST
#include "test/benchmark/binomial_prefix_incremental_arc190_b.cpp"

void check(int n, int a, int b, const std::vector<int> &ks, const std::vector<unsigned> &expected) {
	assert(ks.size() == expected.size());
	std::string input = std::to_string(n) + " " + std::to_string(a) + " " + std::to_string(b) + "\n"
		+ std::to_string(ks.size()) + "\n";
	for (int k : ks) input += std::to_string(k) + " ";
	input += "\n";
	FILE *file = std::tmpfile();
	assert(file);
	{
		FastInput in(input);
		FastOutput out(file);
		solve_arc190_b(in, out);
	}
	std::rewind(file);
	for (unsigned value : expected) {
		unsigned actual = 0;
		assert(std::fscanf(file, "%u", &actual) == 1 && actual == value);
	}
	char extra;
	assert(std::fscanf(file, " %c", &extra) == EOF);
	assert(!std::ferror(file));
	assert(std::fclose(file) == 0);
}

std::vector<unsigned> brute(int n, int a, int b) {
	std::vector<unsigned> answer(n);
	for (unsigned mask = 0; mask < (1U << (2 * (n - 1))); ++mask) {
		int u = 1, v = 1, level = 1;
		unsigned choices = mask;
		for (int length = n; length > 1; --length, choices >>= 2) {
			int nu = u + (choices & 1), nv = v + ((choices >> 1) & 1);
			if (u <= a && a < u + length && v <= b && b < v + length
				&& !(nu <= a && a < nu + length - 1 && nv <= b && b < nv + length - 1))
				level = length;
			u = nu, v = nv;
		}
		++answer[level - 1];
	}
	return answer;
}

int main() {
	check(3, 1, 2, {2}, {6});
	check(5, 2, 5, {1, 3, 5}, {4, 32, 128});
	check(100, 50, 50, {1, 10, 50, 100}, {934228871, 758172260, 444239843, 0});
	for (int n = 1; n <= 6; ++n) for (int a = 1; a <= n; ++a) for (int b = 1; b <= n; ++b) {
		auto expected = brute(n, a, b);
		std::vector<int> ks;
		for (int k = 1; k <= n; ++k) ks.push_back(k);
		check(n, a, b, ks, expected);
		std::vector<int> sparse;
		std::vector<unsigned> values;
		for (int k = 1; k <= n; k += 2) sparse.push_back(k), values.push_back(expected[k - 1]);
		check(n, a, b, sparse, values);
	}
	constexpr int n = 200000;
	std::vector<int> ks;
	std::vector<unsigned> expected;
	std::uint64_t power = 1;
	for (int k = 1; k <= n; ++k) {
		ks.push_back(k);
		expected.push_back(k == 1 ? 1 : 3 * power % 998244353);
		if (k > 1) power = 4 * power % 998244353;
	}
	check(n, 1, n, ks, expected);
	std::vector<int> sparse{1, 2, 17, n / 2, n - 1, n};
	std::vector<unsigned> values;
	for (int k : sparse) values.push_back(expected[k - 1]);
	check(n, n, 1, sparse, values);
}
