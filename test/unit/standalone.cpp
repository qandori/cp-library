#define COMB_MAX 20
#include "data_structure/disjoint_sparse_table.hpp"
#include "components/dynamic_bit_vector.hpp"
#include "data_structure/dynamic_connectivity.hpp"
#include "data_structure/dynamic_segtree.hpp"
#include "data_structure/dynamic_wavelet_matrix.hpp"
#include "data_structure/li_chao_tree.hpp"
#include "data_structure/dynamic_li_chao_tree.hpp"
#include "data_structure/interval_set.hpp"
#include "data_structure/link_cut_tree.hpp"
#include "data_structure/slope_trick.hpp"
#include "data_structure/wavelet_matrix.hpp"
#include "data_structure/weighted_union_find.hpp"
#include "graph/dijkstra.hpp"
#include "graph/max_flow.hpp"
#include "graph/min_cost_flow.hpp"
#include "graph/hld.hpp"
#include "graph/static_top_tree.hpp"
#include "graph/centroid_decomposition.hpp"
#include "graph/lca.hpp"
#include "graph/low_link.hpp"
#include "math/comb.hpp"
#include "math/binomial_prefix_sum.hpp"
#include "math/mod_pow.hpp"
#include "math/mod_sqrt.hpp"
#include "math/factorize.hpp"
#include "fps/fps.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#include "fps/polynomial_gcd.hpp"
#include "convolution/online_convolution.hpp"
#include "math/fraction.hpp"
#include "math/stern_brocot_tree.hpp"
#include "math/matrix.hpp"
#include "math/xor_basis.hpp"
#include "math/min_of_mod_of_linear.hpp"
#include "math/prefix_min_of_mod_of_linear.hpp"
#include "math/monoid_floor_sum.hpp"
#include "math/partition_mod_of_linear.hpp"
#include "string/edit_distance.hpp"
#include "string/lcs.hpp"
#include "string/rolling_hash.hpp"

#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)
#error "Library headers must not load template macros"
#endif

int vector = 0, min = 0;
std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }

struct StaticTreeSum {
	using Vertex = int;
	using Point = int;
	using Path = int;
	static int point_identity() { return 0; }
	static int path_identity() { return 0; }
	static int rake(int a, int b) { return a + b; }
	static int compress(int a, int b) { return a + b; }
	static int add_edge(int a) { return a; }
	static int add_vertex(int a, int v) { return a + v; }
};

int main() {
	assert(mod_sqrt(10, 13) == 6 && !mod_sqrt(2, 13));
	SternBrocotTree<> rational_tree(5, 3);
	assert((rational_tree.depth() == 3 && rational_tree.ancestor(2)->value() == std::pair<std::uint64_t, std::uint64_t>(3, 2)));
	OnlineConvolution<> online(3);
	assert(online.push(1, 3) == 3 && online.push(2, 4) == 10 && online.push(0, 0) == 8);
	MaxFlow<> maximum(2);
	maximum.add_edge(0, 1, 3);
	assert(maximum.flow(0, 1) == 3);
	MinCostFlow<> minimum(2);
	minimum.add_edge(0, 1, 3, -2);
	assert(minimum.flow(0, 1).second == -6);
	BinomialPrefixSum<> prefix(50);
	assert(prefix.query(50, 2) == mint(51) && prefix.prod(5, 1, 3) == mint(15));
	assert(binomial_prefix_sum(std::vector<std::pair<int, int>>{{5, 3}, {4, 1}}) == std::vector<mint>({16, 1}));
	ModPow modular(101);
	assert(modular.pow(3, 7) == 66 && modular.inv(3) == 34 && modular.pow_root(modular.log(7)) == 7);
	assert(vector == 0 && min == 0);
	DynamicWaveletMatrix<int> dynamic(std::vector<int>{3, 1, 4});
	dynamic.insert(1, -2);
	dynamic.set(2, 5);
	assert(dynamic.erase(0) == 3 && dynamic.kth_smallest(0, 3, 1) == 4);
	assert(edit_distance("kitten", "sitting") == 3);
	assert(lcs_length("abcde", "ace") == 3 && lcs("abcde", "ace") == "ace");
	RollingHash hash("abracadabra");
	assert(hash.get(0, 4) == hash.get(7, 11) && hash.lcp(0, hash, 7) == 4);
	assert(RollingHash::op(RollingHash::single('a'), RollingHash::single('b')) == RollingHash("ab").value());
	SlopeTrick<> slope;
	slope.add_abs(3);
	assert(slope.eval(5) == 2);
	const std::vector<std::int64_t> values{1, 2, 3};
	DisjointSparseTable<std::int64_t, add, zero> table(values);
	assert(table.prod(0, 3) == 6);
	DynamicSegtree<std::int64_t, add, zero> sparse(1LL << 60);
	sparse.set(42, 7);
	assert(sparse.prod(0, 100) == 7);
	DynamicConnectivity<std::int64_t, add, zero> graph(values);
	graph.link(0, 1);
	assert(graph.prod(1) == 3);
	LinkCutTree<std::int64_t, add, zero, true> tree(values);
	tree.link(0, 1);
	assert(tree.prod(1, 0) == 3);
	LiChaoTree<> lines({0, 1, 2});
	lines.add_line(2, 1);
	assert(lines.query(2) == 5);
	DynamicLiChaoTree<> online_lines(-3, 4);
	online_lines.add_segment(-2, 3, 2, 1);
	assert(online_lines.query(2) == 5 && !online_lines.query(3));
	IntervalSet<> intervals;
	intervals.insert(1, 4); intervals.erase(2);
	assert(intervals.size() == 2 && intervals.mex(1) == 2 && intervals.next(2) == 3);
	WaveletMatrix<std::int64_t> wm(values);
	assert(wm.kth_smallest(0, 3, 1) == 2 && wm.sum_largest(0, 3, 2) == 5);
	WeightedUnionFind<> uf(3);
	uf.merge(0, 1, 4);
	assert(uf.diff(0, 1) == 4);
	std::vector<std::vector<std::pair<int, int>>> adj{{{1, 2}}, {{2, 3}}, {}};
	assert(dijkstra(adj, 0) == std::vector<int>({0, 2, 5}));
	LCA lca(3);
	lca.add_edge(0, 1);
	lca.add_edge(1, 2);
	lca.build();
	assert(lca.lca(1, 2) == 1 && lca.dist(0, 2) == 2);
	HLD hld(3);
	hld.add_edge(0, 1);
	hld.add_edge(1, 2);
	hld.build();
	assert(hld.jump(2, 0, 1) == 1 && hld.subtree(1).second - hld.subtree(1).first == 2);
	StaticTopTree<StaticTreeSum> static_tree(hld, {1, 2, 3});
	static_tree.set(1, 4);
	assert(static_tree.dp() == 8 && static_tree.subtree_prod(1) == 7);
	CentroidDecomposition<> centroid(3);
	centroid.add_edge(0, 1), centroid.add_edge(1, 2);
	centroid.build();
	assert(centroid.root == 1 && centroid.dist(0, 2) == 2 && centroid.ancestors(0).size() == 2);
	LowLink low(3);
	low.add_edge(0, 1);
	low.add_edge(1, 2);
	low.build();
	assert(low.is_bridge(0) && low.articulation == std::vector<int>({1}));
	init_comb();
	assert(comb(5, 2).val() == 10);
	assert(is_prime(998244353));
	Fraction<> rational(2, -6);
	assert(rational + Fraction<>(5, 6) == Fraction<>(1, 2));
	assert(rational < 0 && rational.floor() == -1);
	Matrix<mint> matrix{{1, 2}, {3, 4}};
	assert(matrix.det() == mint(-2));
	assert(matrix * *matrix.inverse() == Matrix<mint>::identity(2));
	XorBasis<> xor_basis{3, 5};
	assert(xor_basis.max_xor() == 6 && xor_basis.kth(2) == 5);
	RollbackXorBasis<> rollback_basis;
	rollback_basis.insert(3); rollback_basis.undo(); assert(rollback_basis.basis().empty());
	PrefixXorBasis<> prefix_basis;
	prefix_basis.push_back(3); prefix_basis.push_back(5); assert(prefix_basis.basis(1).contains(5));
	XorBasisWithWitness<> witnessed_basis;
	witnessed_basis.insert(3); witnessed_basis.insert(5); assert(witnessed_basis.represent(6)->size() == 2);
	assert(min_of_mod_of_linear(4, 13, 10, 11) == 2);
	auto prefix_min = prefix_min_of_mod_of_linear(4, 13, 10, 11);
	assert(prefix_min.size() == 1 && prefix_min[0].count == 4);
	assert(prefix_min[0].y - prefix_min[0].dy * 3 == 2);
	assert(factorize(12) == std::vector<std::uint64_t>({2, 2, 3}));
	fps f{1, 2, 3};
	assert(polynomial_gcd(fps{mint(-1), 0, 1}, fps{mint(-1), 1}) == fps({mint(-1), 1}));
	assert(bostan_mori(fps{1}, fps{1, -1}, 100) == mint(1));
	assert(linear_recurrence(std::vector<mint>{0, 1}, std::vector<mint>{1, 1}, 10) == mint(55));
	assert(compose(f, fps{0, 1}) == f);
	assert(compositional_inverse(fps{0, 1}, 3) == fps({0, 1, 0}));
	assert(multipoint_eval(f, std::vector<mint>{0, 1}) == std::vector<mint>({1, 6}));
	assert(interpolate(std::vector<mint>{0, 1}, std::vector<mint>{1, 3}) == fps({1, 2}));
	assert(taylor_shift(fps{1, 2}, 3) == fps({7, 2}));
	auto product = f * f.inv(16);
	assert(product[0] == mint(1));
	for (int i = 1; i < 16; ++i) assert(product[i] == mint(0));
	assert((monoid_floor_sum<std::int64_t, add, zero>(3, 5, 2, 1, 1, 10) == 13));
	auto segments = partition_mod_of_linear(3, 5, 2, 1);
	std::vector<std::int64_t> actual(3);
	for (auto [x, y, dx, dy, count] : segments)
		for (std::uint64_t j = 0; j < count; ++j) actual[x + dx * j] = __int128_t(y) + __int128_t(dy) * j;
	assert(actual == std::vector<std::int64_t>({1, 3, 0}));
}
