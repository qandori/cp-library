#include "kernel.hpp"
#include "tree/rerooting.hpp"

Matrix multiply_matrices(const Matrix& a, const Matrix& b) {
	Matrix result{};
	for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) {
		std::uint64_t sum = 0;
		for (int k = 0; k < 4; ++k) sum += std::uint64_t(a[4 * i + k]) * std::uint64_t(b[4 * k + j]);
		result[4 * i + j] = sum % 1000000007;
	}
	return result;
}
Result run_pair(const Input& input) {
	Result result(new Rerooting<PairData>(input.n), [](void *p) { delete static_cast<Rerooting<PairData> *>(p); });
	auto &tree = *static_cast<Rerooting<PairData> *>(result.get());
	for (auto [u, v] : input.edges) tree.add_edge(u, v);
	tree.build(PairData{0, 0}, [](PairData a, PairData b) { return PairData{a.sum + b.sum, a.count + b.count}; },
		[](PairData x, int) { return PairData{x.sum, x.count + 1}; },
		[](PairData x, int, int, int) { return PairData{x.sum + x.count, x.count}; }, input.root);
	return result;
}
Result run_matrix(const Input& input) {
	Result result(new Rerooting<Matrix>(input.n), [](void *p) { delete static_cast<Rerooting<Matrix> *>(p); });
	auto &tree = *static_cast<Rerooting<Matrix> *>(result.get());
	for (auto [u, v] : input.edges) tree.add_edge(u, v);
	Matrix unit{};
	for (int i = 0; i < 4; ++i) unit[5 * i] = 1;
	tree.build(unit, multiply_matrices,
		[&](const Matrix& x, int v) { return multiply_matrices(input.matrices[v], x); },
		[](const Matrix& x, int, int, int) { return x; }, input.root);
	return result;
}
std::vector<PairData> snapshot_pair(const Result& result) {
	const auto &tree = *static_cast<const Rerooting<PairData> *>(result.get());
	auto values = tree.values();
	values.reserve(tree.size() + 2 * tree.edge_count());
	for (int id = 0; id < tree.edge_count(); ++id) {
		values.push_back(tree.edge_dp(id));
		values.push_back(tree.edge_dp(id, true));
	}
	return values;
}
std::vector<Matrix> snapshot_matrix(const Result& result) {
	const auto &tree = *static_cast<const Rerooting<Matrix> *>(result.get());
	auto values = tree.values();
	values.reserve(tree.size() + 2 * tree.edge_count());
	for (int id = 0; id < tree.edge_count(); ++id) {
		values.push_back(tree.edge_dp(id));
		values.push_back(tree.edge_dp(id, true));
	}
	return values;
}
