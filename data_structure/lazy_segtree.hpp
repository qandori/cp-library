#ifndef LAZY_SEGTREE_HPP
#define LAZY_SEGTREE_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class LazySegtree {
	int n_ = 0;
	unsigned base_ = 1, height_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void pull(unsigned k) { data_[k] = op(data_[k * 2], data_[k * 2 + 1]); }
	void all_apply(unsigned k, const F &f) {
		data_[k] = mapping(f, data_[k]);
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = true;
		}
	}
	void push(unsigned k) {
		if (!dirty_[k]) return;
		all_apply(k * 2, lazy_[k]);
		all_apply(k * 2 + 1, lazy_[k]);
		dirty_[k] = false;
	}
	void push_path(unsigned p) {
		for (unsigned i = height_; i; --i) push(p >> i);
	}
	void push_boundaries(unsigned l, unsigned r) {
		for (unsigned i = height_; i; --i) {
			if ((l >> i) << i != l) push(l >> i);
			if ((r >> i) << i != r) push((r - 1) >> i);
		}
	}
	void allocate(int n) {
		assert(n >= 0);
		n_ = n;
		base_ = std::bit_ceil(unsigned(n) ? unsigned(n) : 1U);
		height_ = std::countr_zero(base_);
		data_.assign(std::size_t(base_) * 2, e());
		lazy_.assign(base_, id());
		dirty_.assign(base_, false);
	}
	void vertex([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	S prod_rec(unsigned k, unsigned l, unsigned r, unsigned ql, unsigned qr) const {
		if (ql <= l && r <= qr) return data_[k];
		unsigned m = l + (r - l) / 2;
		S value = qr <= m ? prod_rec(k * 2, l, m, ql, qr)
			: m <= ql ? prod_rec(k * 2 + 1, m, r, ql, qr)
			: op(prod_rec(k * 2, l, m, ql, qr), prod_rec(k * 2 + 1, m, r, ql, qr));
		return dirty_[k] ? mapping(lazy_[k], value) : value;
	}

  public:
	explicit LazySegtree(int n = 0) { allocate(n); }
	explicit LazySegtree(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		allocate(static_cast<int>(a.size()));
		for (int i = 0; i < n_; ++i) data_[base_ + unsigned(i)] = a[i];
		for (unsigned k = base_ - 1; k; --k) pull(k);
	}
	[[nodiscard]] int size() const { return n_; }
	void set(int p, S value) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = std::move(value);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	[[nodiscard]] S get(int p) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		return data_[k];
	}
	[[nodiscard]] S get(int p) const {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		S value = data_[k];
		for (k >>= 1; k; k >>= 1) if (dirty_[k]) value = mapping(lazy_[k], value);
		return value;
	}
	[[nodiscard]] S all_prod() const { return data_[1]; }
	[[nodiscard]] S prod(int l, int r) {
		range(l, r);
		if (l == r) return e();
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r);
		push_boundaries(a, b);
		S left = e(), right = e();
		while (a < b) {
			if (a & 1) left = op(left, data_[a++]);
			if (b & 1) right = op(data_[--b], right);
			a >>= 1, b >>= 1;
		}
		return op(left, right);
	}
	[[nodiscard]] S prod(int l, int r) const {
		range(l, r);
		if (l == r) return e();
		return prod_rec(1, 0, base_, unsigned(l), unsigned(r));
	}
	void apply(int p, F f) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = mapping(f, data_[k]);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	void apply(int l, int r, F f) {
		range(l, r);
		if (l == r) return;
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r), first = a, last = b;
		push_boundaries(a, b);
		while (a < b) {
			if (a & 1) all_apply(a++, f);
			if (b & 1) all_apply(--b, f);
			a >>= 1, b >>= 1;
		}
		for (unsigned i = 1; i <= height_; ++i) {
			if ((first >> i) << i != first) pull(first >> i);
			if ((last >> i) << i != last) pull((last - 1) >> i);
		}
	}
	template<auto predicate> [[nodiscard]] int max_right(int l) { return max_right(l, predicate); }
	template<class Pred> [[nodiscard]] int max_right(int l, Pred predicate) {
		range(l, l);
		assert(predicate(e()));
		if (l == n_) return n_;
		unsigned k = base_ + unsigned(l);
		push_path(k);
		S acc = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(acc, data_[k]);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k *= 2;
					S part = op(acc, data_[k]);
					if (predicate(part)) acc = std::move(part), ++k;
				}
				return static_cast<int>(k - base_);
			}
			acc = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template<auto predicate> [[nodiscard]] int min_left(int r) { return min_left(r, predicate); }
	template<class Pred> [[nodiscard]] int min_left(int r, Pred predicate) {
		range(r, r);
		assert(predicate(e()));
		if (!r) return 0;
		unsigned k = base_ + unsigned(r);
		push_path(k - 1);
		S acc = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], acc);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k = k * 2 + 1;
					S part = op(data_[k], acc);
					if (predicate(part)) acc = std::move(part), --k;
				}
				return static_cast<int>(k + 1 - base_);
			}
			acc = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
};

namespace cp_acl {
template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
using lazy_segtree = LazySegtree<S, op, e, F, mapping, composition, id>;
}

#endif
