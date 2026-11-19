#include "flow/min_cost_flow.hpp"
#include <cassert>
#include <cstdlib>
#include <new>

namespace {
using Graph = MinCostFlow<int, std::int8_t>;
Graph *observed = nullptr;
bool fail_after_flow = false;
}

void *operator new(std::size_t bytes) {
	if (fail_after_flow && observed && observed->get_edge(0).flow) throw std::bad_alloc();
	if (void *p = std::malloc(bytes ? bytes : 1)) return p;
	throw std::bad_alloc();
}
[[gnu::noinline]] void operator delete(void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }

int main() {
	Graph graph(2);
	graph.add_edge(0, 1, 1, 126);
	observed = &graph;
	fail_after_flow = true;
	bool failed = false;
	try { graph.slope(0, 1, 1); }
	catch (const std::bad_alloc &) { failed = true; }
	fail_after_flow = false;
	assert(failed && graph.get_edge(0).flow == 1);
	graph.add_edge(0, 1, 0, 0);
	const auto& potential = graph.potentials();
	assert(-126 + int(potential[1]) - int(potential[0]) >= 0);
	auto result = graph.flow(1, 0, 1);
	assert(result.first == 1 && result.second == -126);
	observed = nullptr;
}
