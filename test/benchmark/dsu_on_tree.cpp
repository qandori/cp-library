#include "tree/dsu_on_tree.hpp"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sys/resource.h>

template<class Add, class Remove, class Answer>
void stack_run(const DSUOnTree& tree, Add& add, Remove& remove, Answer& answer) {
	struct Frame { int v; bool keep, ready; };
	std::vector<Frame> stack;
	stack.reserve(tree.size());
	stack.push_back({tree.root(), false, false});
	while (!stack.empty()) {
		auto [v, keep, ready] = stack.back();
		stack.pop_back();
		int h = tree.heavy_child(v), begin = h == -1 ? tree.subtree(v).first + 1 : tree.subtree(h).second;
		int end = tree.subtree(v).second;
		if (!ready) {
			stack.push_back({v, keep, true});
			if (h != -1) stack.push_back({h, true, false});
			for (int i = begin; i < end;) {
				int to = tree.vertex(i);
				stack.push_back({to, false, false});
				i = tree.subtree(to).second;
			}
		} else {
			for (int i = begin; i < end; ++i) add(tree.vertex(i));
			add(v);
			answer(v);
			if (!keep) for (int i = tree.subtree(v).first; i < end; ++i) remove(tree.vertex(i));
		}
	}
}

int main(int argc, char** argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 200000;
	std::string shape = argc > 2 ? argv[2] : "random";
	std::string method = argc > 3 ? argv[3] : "linear";
	std::uint64_t state = 20261021;
	auto random = [&] { state ^= state << 7; state ^= state >> 9; return state; };
	DSUOnTree tree(n);
	for (int v = 1; v < n; ++v) {
		int p = shape == "path" ? v - 1 : shape == "star" ? 0 : shape == "balanced" ? (v - 1) / 2 :
			shape == "broom" ? (v < n / 2 ? v - 1 : n / 2 - 1) : int(random() % v);
		tree.add_edge(p, v);
	}
	std::vector<int> color(n), count(n), frequencies(n + 1), touched;
	for (auto& c : color) c = int(random() % n);
	int maximum = 0, answers = 0;
	std::uint64_t checksum = 0, additions = 0, removals = 0;
	using Clock = std::chrono::steady_clock;
	auto start_wall = Clock::now();
	auto start_cpu = std::clock();
	tree.build();
	auto built_cpu = std::clock();
	auto built_wall = Clock::now();
	std::uint64_t expected_additions = 0;
	for (int v = 0; v < n; ++v) {
		int h = tree.heavy_child(v);
		expected_additions += tree.subtree_size(v) - (h == -1 ? 0 : tree.subtree_size(h));
	}
	auto add = [&](int v) {
		++additions;
		int old = count[color[v]]++;
		if (old) --frequencies[old];
		++frequencies[old + 1];
		maximum = std::max(maximum, old + 1);
	};
	auto remove = [&](int v) {
		++removals;
		int old = count[color[v]]--;
		assert(old > 0);
		--frequencies[old];
		if (old > 1) ++frequencies[old - 1];
		if (!frequencies[maximum]) --maximum;
	};
	auto answer = [&](int v) {
		++answers;
		checksum += std::uint64_t(maximum ^ (v + 1)) * (frequencies[maximum] ^ (v + 1));
	};
	auto run_cpu = std::clock();
	auto run_wall = Clock::now();
	if (method == "stack") stack_run(tree, add, remove, answer);
	else if (method == "clear") tree.run_clear([&](int v) {
		if (!count[color[v]]) touched.push_back(color[v]);
		add(v);
	}, answer, [&] {
		for (int c : touched) count[c] = 0;
		touched.clear();
		std::fill(frequencies.begin() + 1, frequencies.begin() + maximum + 1, 0);
		maximum = 0;
	});
	else { assert(method == "linear"); tree.run(add, remove, answer); }
	auto done_cpu = std::clock();
	auto done_wall = Clock::now();
	assert(maximum == 0 && answers == n && additions == expected_additions);
	assert(method == "clear" || additions == removals);
	for (int x : count) { assert(x == 0); (void)x; }
	struct rusage usage {};
	getrusage(RUSAGE_SELF, &usage);
	std::printf("{\"n\":%d,\"shape\":\"%s\",\"method\":\"%s\",\"build_cpu\":%.9f,\"build_wall\":%.9f,\"run_cpu\":%.9f,\"run_wall\":%.9f,\"checksum\":%llu,\"additions\":%llu,\"removals\":%llu,\"answers\":%d,\"rss_kib\":%ld}\n",
		n, shape.c_str(), method.c_str(), double(built_cpu - start_cpu) / CLOCKS_PER_SEC,
		std::chrono::duration<double>(built_wall - start_wall).count(), double(done_cpu - run_cpu) / CLOCKS_PER_SEC,
		std::chrono::duration<double>(done_wall - run_wall).count(), (unsigned long long)checksum,
		(unsigned long long)additions, (unsigned long long)removals, answers, usage.ru_maxrss);
}
