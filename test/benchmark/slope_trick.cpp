#include "data_structure/slope_trick.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>

using I = std::int64_t;
template <class T, class C = std::less<T>, std::size_t = 2>
class StandardHeap : public std::priority_queue<T, std::vector<T>, C> {
  public:
	const std::vector<T> &values() const { return this->c; }
	T replace_top(T value) {
		T old = this->top();
		this->pop();
		this->push(value);
		return old;
	}
	T pop_value() {
		T old = this->top();
		this->pop();
		return old;
	}
	void clear() { this->c.clear(); }
};
template <class T, class C, std::size_t D, bool floyd>
class TrialHeap {
	std::vector<T> data;
	C compare{};
	void down(std::size_t i, T value) {
		std::size_t n = data.size(), start = i;
		while (i < n / D + (n % D > 1)) {
			std::size_t best = i * D + 1;
			if constexpr (floyd) {
				for (std::size_t j = best + 1, end = std::min(best + D, n); j < end; ++j)
					if (compare(data[best], data[j])) best = j;
			} else if constexpr (D == 2) {
				if (best + 1 < n && compare(data[best], data[best + 1])) ++best;
			} else if (best + D <= n) {
				for (std::size_t j = best + 1, end = best + D; j < end; ++j)
					if (compare(data[best], data[j])) best = j;
			} else {
				for (std::size_t j = best + 1; j < n; ++j)
					if (compare(data[best], data[j])) best = j;
			}
			if constexpr (!floyd)
				if (!compare(value, data[best])) break;
			data[i] = data[best];
			i = best;
		}
		if constexpr (floyd)
			while (i > start) {
				std::size_t p = (i - 1) / D;
				if (!compare(data[p], value)) break;
				data[i] = data[p];
				i = p;
			}
		data[i] = value;
	}

  public:
	bool empty() const { return data.empty(); }
	std::size_t size() const { return data.size(); }
	const std::vector<T> &values() const { return data; }
	T top() const { return data.front(); }
	void clear() { data.clear(); }
	void push(T value) {
		data.push_back(value);
		std::size_t i = data.size() - 1;
		while (i) {
			std::size_t p = (i - 1) / D;
			if (!compare(data[p], value)) break;
			data[i] = data[p];
			i = p;
		}
		data[i] = value;
	}
	T replace_top(T value) {
		T result = data.front();
		down(0, value);
		return result;
	}
};
template <class T, class C, std::size_t D>
using DirectHeap = TrialHeap<T, C, D, false>;
template <class T, class C, std::size_t D>
using FloydHeap = TrialHeap<T, C, D, true>;

template <template <class, class, std::size_t> class Heap, std::size_t D, bool fused = true, bool linear = true, int batch = 0, bool release = false>
struct Candidate {
	Heap<I, std::less<I>, D> left;
	Heap<I, std::greater<I>, D> right;
	I l = 0, r = 0, value = 0;
	std::size_t size() const { return left.size() + right.size(); }
	I min() const { return value; }
	void add_all(I a) { value += a; }
	void add_a_minus_x(I a) {
		if (!right.empty() && right.top() + r < a) {
			I x = right.replace_top(a - r) + r;
			value += a - x;
			left.push(x - l);
		} else left.push(a - l);
	}
	void add_x_minus_a(I a) {
		if (!left.empty() && a < left.top() + l) {
			I x = left.replace_top(a - l) + l;
			value += x - a;
			right.push(x - r);
		} else right.push(a - r);
	}
	void add_abs(I a) {
		if constexpr (!fused) {
			add_a_minus_x(a);
			add_x_minus_a(a);
		} else if (!left.empty() && a < left.top() + l) {
			I x = left.replace_top(a - l) + l;
			value += x - a;
			left.push(a - l);
			right.push(x - r);
		} else if (!right.empty() && right.top() + r < a) {
			I x = right.replace_top(a - r) + r;
			value += a - x;
			right.push(a - r);
			left.push(x - l);
		} else left.push(a - l), right.push(a - r);
	}
	void clear_right() {
		right.clear();
		r = 0;
	}
	void shift(I a, I b) {
		l += a;
		r += b;
	}
	void merge(Candidate &other) {
		if (size() < other.size()) std::swap(*this, other);
		if constexpr (batch > 0) {
			if (size() + other.size() >= batch && other.size() >= size() / 4) {
				std::vector<I> values;
				values.reserve(size() + other.size());
				I delta = 0;
				for (I x : left.values()) values.push_back(x + l), delta += x + l;
				for (I x : other.left.values()) values.push_back(x + other.l), delta += x + other.l;
				std::size_t middle = values.size();
				for (I x : right.values()) values.push_back(x + r);
				for (I x : other.right.values()) values.push_back(x + other.r);
				std::nth_element(values.begin(), values.begin() + middle, values.end());
				for (std::size_t i = 0; i < middle; ++i) delta -= values[i];
				left = Heap<I, std::less<I>, D>(std::vector<I>(values.begin(), values.begin() + middle));
				right = Heap<I, std::greater<I>, D>(std::vector<I>(values.begin() + middle, values.end()));
				l = r = 0;
				value += other.value + delta;
				other.left.clear();
				other.right.clear();
				other.l = other.r = other.value = 0;
				return;
			}
		}
		value += other.value;
		if constexpr (linear) {
			for (I x : other.left.values()) add_a_minus_x(x + other.l);
			for (I x : other.right.values()) add_x_minus_a(x + other.r);
			other.left.clear();
			other.right.clear();
		} else {
			while (!other.left.empty()) add_a_minus_x(other.left.pop_value() + other.l);
			while (!other.right.empty()) add_x_minus_a(other.right.pop_value() + other.r);
		}
		other.l = other.r = other.value = 0;
		if constexpr (release) {
			other.left = {};
			other.right = {};
		}
	}
};
template <class S>
void run(int n, const std::string &mode, int seed) {
	std::mt19937_64 rng(seed);
	std::vector<I> a(n);
	for (int i = 0; i < n; ++i) {
		if (mode == "increasing") a[i] = i;
		else if (mode == "decreasing") a[i] = -I(i);
		else if (mode == "duplicates") a[i] = rng() % 16;
		else a[i] = I(rng() % 2000000001) - 1000000000;
	}
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	if (mode == "merge_balanced") {
		std::vector<S> trees(n);
		for (int i = 0; i < n; ++i) trees[i].add_abs(a[i]);
		for (int width = 1; width < n; width *= 2)
			for (int i = 0; i + width < n; i += 2 * width) trees[i].merge(trees[i + width]);
		checksum = trees[0].min();
	} else if (mode == "merge_small") {
		S tree;
		for (int i = 0; i < n; ++i) {
			S other;
			other.add_abs(a[i]);
			other.shift(-1, 2);
			tree.merge(other);
		}
		checksum = tree.min();
	} else {
		S tree;
		for (int i = 0; i < n; ++i) {
			if (mode == "prefix") tree.clear_right();
			if (mode == "sliding") tree.shift(-100, 100);
			if (mode == "hinges") {
				if (i & 1) tree.add_a_minus_x(a[i]);
				else tree.add_x_minus_a(a[i]);
			} else tree.add_abs(a[i]);
			checksum += std::uint64_t(tree.min());
		}
	}
	double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
	std::cout << std::fixed << std::setprecision(4) << elapsed << ' ' << checksum << '\n';
}
int main(int argc, char **argv) {
	std::string variant = argc > 1 ? argv[1] : "library", mode = argc > 2 ? argv[2] : "random";
	int n = argc > 3 ? std::atoi(argv[3]) : 500000, seed = argc > 4 ? std::atoi(argv[4]) : 713;
	if (variant == "direct_binary") run<Candidate<DirectHeap, 2>>(n, mode, seed);
	else if (variant == "direct_four") run<Candidate<DirectHeap, 4>>(n, mode, seed);
	else if (variant == "floyd_binary") run<Candidate<FloydHeap, 2>>(n, mode, seed);
	else if (variant == "floyd_four") run<Candidate<FloydHeap, 4>>(n, mode, seed);
	else if (variant == "release_binary") run<Candidate<DaryHeap, 2, true, true, 0, true>>(n, mode, seed);
	else if (variant == "release_four") run<Candidate<DaryHeap, 4, true, true, 0, true>>(n, mode, seed);
	else if (variant == "batch64") run<Candidate<DaryHeap, 2, true, true, 64>>(n, mode, seed);
	else if (variant == "batch256") run<Candidate<DaryHeap, 2, true, true, 256>>(n, mode, seed);
	else if (variant == "batch1024") run<Candidate<DaryHeap, 2, true, true, 1024>>(n, mode, seed);
	else if (variant == "batch256_four") run<Candidate<DaryHeap, 4, true, true, 256>>(n, mode, seed);
	else if (variant == "standard") run<Candidate<StandardHeap, 2, false, false>>(n, mode, seed);
	else if (variant == "standard_fused") run<Candidate<StandardHeap, 2>>(n, mode, seed);
	else if (variant == "binary") run<Candidate<DaryHeap, 2>>(n, mode, seed);
	else if (variant == "quaternary") run<Candidate<DaryHeap, 4>>(n, mode, seed);
	else if (variant == "octary") run<Candidate<DaryHeap, 8>>(n, mode, seed);
	else if (variant == "hexadecary") run<Candidate<DaryHeap, 16>>(n, mode, seed);
	else if (variant == "library") run<SlopeTrick<>>(n, mode, seed);
	else return 1;
}
