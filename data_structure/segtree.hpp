#ifndef SEGTREE_HPP
#define SEGTREE_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template<class S, auto op, auto e> class Segtree {
	int n_ = 0;
	unsigned base_ = 1;
	std::vector<S> data_;
	void pull(unsigned k) { data_[k] = op(data_[k * 2], data_[k * 2 + 1]); }
	void allocate(int n) {
		assert(n >= 0);
		n_ = n;
		base_ = std::bit_ceil(unsigned(n) ? unsigned(n) : 1U);
		data_.assign(std::size_t(base_) * 2, e());
	}

  public:
	explicit Segtree(int n = 0) { allocate(n); }
	explicit Segtree(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		allocate(static_cast<int>(a.size()));
		for (int i = 0; i < n_; ++i) data_[base_ + unsigned(i)] = a[i];
		for (unsigned k = base_ - 1; k; --k) pull(k);
	}
	Segtree(const Segtree &) = default;
	Segtree &operator=(const Segtree &other) {
		if (this != &other) { Segtree copy(other); swap(copy); }
		return *this;
	}
	Segtree(Segtree &&other) noexcept
		: n_(std::exchange(other.n_, 0)), base_(std::exchange(other.base_, 1)), data_(std::move(other.data_)) {}
	Segtree &operator=(Segtree &&other) noexcept {
		if (this != &other) {
			n_ = std::exchange(other.n_, 0);
			base_ = std::exchange(other.base_, 1);
			data_ = std::move(other.data_);
		}
		return *this;
	}
	void swap(Segtree &other) noexcept {
		std::swap(n_, other.n_), std::swap(base_, other.base_);
		data_.swap(other.data_);
	}
	[[nodiscard]] int size() const { return n_; }
	void set(int p, S value) {
		assert(0 <= p && p < n_);
		unsigned k = base_ + unsigned(p);
		data_[k] = std::move(value);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	void multiply(int p, S value) { set(p, op(get(p), std::move(value))); }
	[[nodiscard]] S get(int p) const {
		assert(0 <= p && p < n_);
		return data_[base_ + unsigned(p)];
	}
	[[nodiscard]] S all_prod() const { return n_ ? data_[1] : e(); }
	[[nodiscard]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n_);
		if (l == r) return e();
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r) - 1;
		unsigned level = std::bit_width(a ^ b);
		if (!level) return data_[a];
		unsigned left_stop = a >> (level - 1), right_stop = left_stop + 1;
		a >>= (std::min)(unsigned(std::countr_zero(a)), level - 1);
		b >>= (std::min)(unsigned(std::countr_one(b)), level - 1);
		S left = data_[a], right = data_[b];
		for (;;) {
			++a;
			a >>= std::countr_zero(a);
			if (a <= right_stop) break;
			left = op(std::move(left), data_[a]);
		}
		for (;;) {
			--b;
			b >>= std::countr_one(b);
			if (b <= right_stop) break;
			right = op(data_[b], std::move(right));
		}
		return op(std::move(left), std::move(right));
	}
	template<auto f> [[nodiscard]] int max_right(int l) const { return max_right(l, f); }
	template<class Pred> [[nodiscard]] int max_right(int l, Pred f) const {
		assert(0 <= l && l <= n_ && f(e()));
		if (l == n_) return n_;
		unsigned k = base_ + unsigned(l);
		S value = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(value, data_[k]);
			if (!f(next)) {
				while (k < base_) {
					k *= 2;
					next = op(value, data_[k]);
					if (f(next)) value = std::move(next), ++k;
				}
				return static_cast<int>(k - base_);
			}
			value = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template<auto f> [[nodiscard]] int min_left(int r) const { return min_left(r, f); }
	template<class Pred> [[nodiscard]] int min_left(int r, Pred f) const {
		assert(0 <= r && r <= n_ && f(e()));
		if (!r) return 0;
		unsigned k = base_ + unsigned(r);
		S value = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], value);
			if (!f(next)) {
				while (k < base_) {
					k = k * 2 + 1;
					next = op(data_[k], value);
					if (f(next)) value = std::move(next), --k;
				}
				return static_cast<int>(k + 1 - base_);
			}
			value = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
};

namespace cp_acl {
template<class S, auto op, auto e> using segtree = Segtree<S, op, e>;
}

#endif
