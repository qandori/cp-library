#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "../components/fast_io.hpp"
#include "../components/predecessor_set.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	PredecessorSet set(in.token());
	while (q--) {
		int type = 0, p = 0;
		in.read(type), in.read(p);
		if (type < 2) set.set(p, type == 0);
		else if (type == 2) out.println(int(set.contains(p)));
		else if (type == 3) { int x = set.next(p); out.println(x == n ? -1 : x); }
		else out.println(set.prev(p));
	}
}
