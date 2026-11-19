#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=2903"
#include "flow/project_selection.hpp"
#include "components/fast_io.hpp"
#include <array>
#include <string>

int main() {
	FastInput input;
	FastOutput output;
	int h = 0, w = 0;
	input.read(h), input.read(w);
	std::vector<std::string> grid(h);
	for (auto& row : grid) input.read(row);
	std::vector<std::array<int, 2>> horizontal, vertical;
	std::vector<std::vector<int>> row_edges(h * w), column_edges(h * w);
	for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) if (grid[r][c] == '#') {
		int v = r * w + c;
		if (c + 1 < w && grid[r][c + 1] == '#') horizontal.push_back({v, v + 1});
		if (r + 1 < h && grid[r + 1][c] == '#') vertical.push_back({v, v + w});
	}
	int count = int(horizontal.size() + vertical.size());
	std::vector<bool> flipped(count);
	for (int id = int(horizontal.size()); id < count; ++id) flipped[id] = true;
	ProjectSelection<> problem(count, flipped);
	for (int id = 0; id < int(horizontal.size()); ++id)
		for (int v : horizontal[id]) row_edges[v].push_back(id);
	for (int id = 0; id < int(vertical.size()); ++id)
		for (int v : vertical[id]) column_edges[v].push_back(int(horizontal.size()) + id);
	for (int id = 0; id < count; ++id) problem.add_cost(id, -1);
	for (int v = 0; v < h * w; ++v) if (grid[v / w][v % w] == '#') {
		problem.add_constant(1);
		for (int a : row_edges[v]) for (int b : column_edges[v]) problem.forbid(a, b, true, true);
	}
	output.println(problem.solve()->cost);
}
