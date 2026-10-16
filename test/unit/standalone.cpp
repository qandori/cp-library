#define COMB_MAX 20
#include "data_structure/disjoint_sparse_table.hpp"
#include "components/dynamic_bit_vector.hpp"
#include "data_structure/dynamic_connectivity.hpp"
#include "data_structure/dynamic_segtree.hpp"
#include "data_structure/dynamic_wavelet_matrix.hpp"
#include "data_structure/li_chao_tree.hpp"
#include "data_structure/link_cut_tree.hpp"
#include "data_structure/slope_trick.hpp"
#include "data_structure/wavelet_matrix.hpp"
#include "data_structure/weighted_union_find.hpp"
#include "graph/dijkstra.hpp"
#include "graph/hld.hpp"
#include "graph/lca.hpp"
#include "graph/low_link.hpp"
#include "math/comb.hpp"
#include "math/factorize.hpp"
#include "math/fps.hpp"
#include "math/min_of_mod_of_linear.hpp"
#include "math/monoid_floor_sum.hpp"
#include "math/sqrt_floor.hpp"
#include "string/edit_distance.hpp"
#include "string/lcs.hpp"
#include "string/rolling_hash.hpp"

#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)
#error "Library headers must not load template macros"
#endif

int vector = 0, min = 0;
std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }

int main() {
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
	LowLink low(3);
	low.add_edge(0, 1);
	low.add_edge(1, 2);
	low.build();
	assert(low.is_bridge(0) && low.articulation == std::vector<int>({1}));
	init_comb();
	assert(comb(5, 2).val() == 10);
	assert(is_prime(998244353));
	assert(min_of_mod_of_linear(4, 13, 10, 11) == 2);
	assert(factorize(12) == std::vector<std::uint64_t>({2, 2, 3}));
	fps f{1, 2, 3};
	auto product = f * f.inv(16);
	assert(product[0] == mint(1));
	for (int i = 1; i < 16; ++i) assert(product[i] == mint(0));
	assert((monoid_floor_sum<std::int64_t, add, zero>(3, 5, 2, 1, 1, 10) == 13));
	auto [dx, dy, segments] = sqrt_floor(3, 5, 2, 1);
	std::vector<std::int64_t> actual(3);
	for (auto [x, y, length] : segments)
		for (std::int64_t j = 0; j < length; ++j) actual[x + dx * j] = y + dy * j;
	assert(actual == std::vector<std::int64_t>({1, 3, 0}));
}
