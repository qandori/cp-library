#include "math/fps.hpp"
#include <cassert>
#include <random>
#include <vector>

using namespace std;

int main() {
	mt19937 random(123);
	for (int n : {1, 2, 3, 8, 31, 32, 63, 64, 127, 128, 257}) {
		fps a(n), b(n), expected(2 * n - 1);
		for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random();
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) expected[i + j] += a[i] * b[j];
		assert(a * b == expected);
		a[0] = 1;
		fps inverse = a.inv(n), product = a * inverse;
		assert(product[0] == mint(1));
		for (int i = 1; i < n; ++i) assert(product[i] == mint(0));
		assert(a.log(n).exp(n) == a);
		vector<mint> xs(n);
		for (int i = 0; i < n; ++i) xs[i] = random();
		auto values = a.multipoint_eval(xs);
		for (int i = 0; i < n; ++i) {
			mint value = 0;
			for (int j = (n)-1; j >= 0; --j) value = value * xs[i] + a[j];
			assert(values[i] == value);
		}
	}
}
