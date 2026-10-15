#include "data_structure/dynamic_segtree.hpp"
#include "string/rolling_hash.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <string_view>
#include <vector>

using RH = RollingHash;
using S = RH::Value;

int main() {
	std::mt19937_64 random(713);
	assert(RH::e() == S{} && RH::e() == RH().value());
	assert(RH::single(-1).hash == 0 && RH::single(-1) != RH::e());
	assert(RH::op(RH::e(), RH::single(-1)) == RH::single(-1));
	assert(RH::op(RH::single(-1), RH::e()) == RH::single(-1));
	RH::reserve(0);
	for (int pass = 0; pass < 200; ++pass) {
		std::vector<std::int64_t> a(random() % 100);
		for (auto &x : a) x = std::int64_t(random());
		if (!a.empty()) a[0] = std::numeric_limits<std::int64_t>::min();
		RH hash(a);
		S whole = RH::e();
		for (auto x : a) whole = RH::op(whole, RH::single(x));
		assert(whole == hash.value());
		assert(whole.hash == hash.get() && whole.length == hash.size());
		for (int query = 0; query < 200; ++query) {
			int l = random() % (a.size() + 1), m = random() % (a.size() + 1), r = random() % (a.size() + 1);
			if (l > m) std::swap(l, m);
			if (m > r) std::swap(m, r);
			if (l > m) std::swap(l, m);
			S x = hash.value(0, l), y = hash.value(l, m), z = hash.value(m, r);
			assert(RH::op(x, RH::op(y, z)) == RH::op(RH::op(x, y), z));
			assert(RH::op(hash.value(l, m), hash.value(m, r)) == hash.value(l, r));
		}
	}
	for (int n : {0, 1, 2, 3, 7, 16, 63, 257, 1000}) {
		std::string s(n, '\0');
		for (char &c : s) c = char(random() % 256);
		RH::reserve(n);
		DynamicSegtree<S, RH::op, RH::e> seg(std::max(n, 1));
		for (int i = 0; i < n; ++i) seg.set(i, RH::single(s[i]));
		for (int query = 0; query < 2000; ++query) {
			if (n && query % 3 == 0) {
				int p = random() % n;
				s[p] = char(random() % 256);
				seg.set(p, RH::single(s[p]));
			}
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			assert(seg.prod(l, r) == RH(std::string_view(s).substr(l, r - l)).value());
			assert(seg.all_prod() == RH(s).value());
		}
	}
	DynamicSegtree<S, RH::op, RH::e> sparse(-1000000000000LL, 1000000000000LL);
	std::map<std::int64_t, std::string> values;
	for (int query = 0; query < 5000; ++query) {
		std::int64_t p = std::int64_t(random() % 1001) * 1000000 - 500000000;
		auto &value = values[p];
		value.resize(random() % 8);
		for (char &c : value) c = char(random() % 256);
		sparse.set(p, RH(value).value());
		std::int64_t l = std::int64_t(random() % 2000000001) - 1000000000, r = std::int64_t(random() % 2000000001) - 1000000000;
		if (l > r) std::swap(l, r);
		std::string joined;
		for (auto it = values.lower_bound(l); it != values.end() && it->first < r; ++it) joined += it->second;
		assert(sparse.prod(l, r) == RH(joined).value());
	}
	S before = RH("abcdef").value();
	RH::reserve(1000000);
	assert(RH::op(before, RH("ghij").value()) == RH("abcdefghij").value());
	assert(RH::op(RH("a").value(), RH("bc").value()) != RH::op(RH("bc").value(), RH("a").value()));
}
