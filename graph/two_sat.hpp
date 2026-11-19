#ifndef TWO_SAT_HPP
#define TWO_SAT_HPP

#include "../components/two_sat_core.hpp"

class TwoSAT {
	int n_;
	std::vector<std::pair<int, int>> edges_;
	std::vector<int> forced_;
	std::vector<bool> answer_;
	bool valid_ = false, solved_ = false;
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), answer_(vertices(n) / 2) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = solved_ = false;
		if (i == j && f != g) return;
		assert(edges_.size() <= std::size_t(std::numeric_limits<int>::max() - 1 - int(i != j)));
		edges_.emplace_back(2 * i + !f, 2 * j + g);
		if (i == j) forced_.push_back(2 * i + f);
		if (i != j) edges_.emplace_back(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		if (solved_) return valid_;
		valid_ = false;
		if (edges_.empty()) std::fill(answer_.begin(), answer_.end(), false), valid_ = true;
		else valid_ = two_sat_detail::solve(n_, edges_, forced_, answer_);
		solved_ = true;
		return valid_;
	}
	std::vector<bool> answer() const { return answer_; }
};

namespace cp_acl {
using two_sat = TwoSAT;
}

#endif
