#define PROBLEM "https://judge.yosupo.jp/problem/multiplication_of_big_integers"
#include "../math/bigint.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int t = 0;
	in.read(t);
	while (t--) {
		BigInt a, b;
		in.read(a);
		in.read(b);
		out.println(a * b);
	}
}
