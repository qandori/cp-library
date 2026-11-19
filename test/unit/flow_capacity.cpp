#include "flow/max_flow.hpp"
#include "flow/min_cost_flow.hpp"

template<class Cap> void check() {
	MaxFlow<Cap> maximum(3);
	maximum.add_edge(0, 1, 100), maximum.add_edge(1, 2, 70);
	assert(maximum.flow(0, 2) == 70);
	assert(maximum.get_edge(0).cap == 100 && maximum.get_edge(0).flow == 70);
	maximum.reset_flow();
	assert(maximum.flow(0, 2, 17) == 17 && maximum.flow(0, 2) == 53);
	MinCostFlow<Cap, short> minimum(3);
	minimum.add_edge(0, 1, 100, -7), minimum.add_edge(1, 2, 70, 10);
	auto result = minimum.flow(0, 2);
	assert(result.first == 70 && result.second == 210);
	assert(minimum.get_edge(0).cap == 100 && minimum.get_edge(0).flow == 70);
	minimum.reset_flow();
	assert(minimum.slope(0, 2).back() == result);
}

int main() {
	check<signed char>();
	check<unsigned char>();
	check<short>();
	check<unsigned short>();
}
