#include "data_structure/wavelet_matrix.hpp"
#include <chrono>
struct Query { int l, r, k, pos; int64_t x; };
uint64_t sink = 0;
int max_length = 0;
template<class F> double timed(F f) {
	auto start = chrono::steady_clock::now();
	f();
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}
template<class T> void run(int n, int q, int mode) {
	mt19937_64 rng(12345678);
	vec<T> a(n);
	for (auto &x : a) x = mode == 0 ? T(rng() % 1000000000) : mode == 1 ? T(rng() % 256) : T(rng());
	if (mode == 3) sort(all(a));
	vec<Query> queries(q);
	for (auto &z : queries) {
		z.l = rng() % n; z.r = rng() % n;
		if (z.l > z.r) swap(z.l,z.r);
		++z.r;
		if (max_length) z.r = min(n, z.l + 1 + int(rng() % max_length));
		z.k = rng() % (z.r-z.l); z.pos = rng() % n;
		z.x = a[rng()%n];
	}
	WaveletMatrix<T> wm;
	auto emit = [&](string_view name, double t) { cout << name << '=' << t << ' '; };
	cout << fixed << setprecision(2);
	emit("build", timed([&]{wm.build(a);}));
	emit("kth", timed([&]{for(auto z:queries) sink += uint64_t(wm.kth_smallest(z.l,z.r,z.k));}));
	emit("freq", timed([&]{for(auto z:queries) sink += wm.range_freq(z.l,z.r,T(z.x));}));
	emit("count", timed([&]{for(auto z:queries) sink += wm.count(z.l,z.r,T(z.x));}));
	emit("access", timed([&]{for(auto z:queries) sink += uint64_t(wm.access(z.pos));}));
	emit("select", timed([&]{rep(i,min(q,20000)) sink += wm.select(T(queries[i].x),0);}));
	emit("prepare", timed([&]{wm.prepare_sum();}));
	emit("small", timed([&]{for(auto z:queries) sink += uint64_t(wm.sum_smallest(z.l,z.r,z.k));}));
	emit("large", timed([&]{for(auto z:queries) sink += uint64_t(wm.sum_largest(z.l,z.r,z.k));}));
	emit("build_sum", timed([&]{WaveletMatrix<T> other(a,true); sink += uint64_t(other.range_sum(0,n));}));
	cout << "checksum=" << sink << '\n';
}
int main(int argc,char **argv) {
	int n=argc>1?atoi(argv[1]):200000,q=argc>2?atoi(argv[2]):200000,mode=argc>3?atoi(argv[3]):0;
	if (argc > 5) max_length = atoi(argv[5]);
	if(argc>4 && string(argv[4])=="32") run<int32_t>(n,q,mode);
	else run<int64_t>(n,q,mode);
}
