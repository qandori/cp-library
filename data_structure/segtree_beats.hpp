#ifndef SEGTREE_BEATS_HPP
#define SEGTREE_BEATS_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class SegtreeBeats {
	int n_ = 0, base_ = 1, log_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const {
		assert(0 <= l && l <= r && r <= n_);
	}
	void pull(int k) { data_[k] = op(data_[2 * k], data_[2 * k + 1]); }
	void all_apply(int k, const F &f) {
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = 1;
		}
		if (mapping(f, data_[k])) return;
		assert(k < base_);
		push(k);
		pull(k);
	}
	void push(int k) {
		if (!dirty_[k]) return;
		all_apply(2 * k, lazy_[k]);
		all_apply(2 * k + 1, lazy_[k]);
		dirty_[k] = 0;
	}
	void push_path(int k) {
		for (int h = log_; h; --h) push(k >> h);
	}
	void push_range(int l, int r) {
		for (int h = log_; h; --h) {
			if (((l >> h) << h) != l) push(l >> h);
			if (((r >> h) << h) != r) push((r - 1) >> h);
		}
	}
	void pull_range(int l, int r) {
		for (int h = 1; h <= log_; ++h) {
			if (((l >> h) << h) != l) pull(l >> h);
			if (((r >> h) << h) != r) pull((r - 1) >> h);
		}
	}

  public:
	SegtreeBeats() : SegtreeBeats(0) {}
	explicit SegtreeBeats(int n, const S &value = e()) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 4);
		build(std::vector<S>(n, value));
	}
	explicit SegtreeBeats(const std::vector<S> &a) { build(a); }
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		n_ = static_cast<int>(a.size());
		base_ = static_cast<int>(std::bit_ceil(static_cast<unsigned>(std::max(1, n_))));
		log_ = std::countr_zero(static_cast<unsigned>(base_));
		data_.assign(2 * base_, e());
		lazy_.assign(base_, id());
		dirty_.assign(base_, 0);
		std::copy(a.begin(), a.end(), data_.begin() + base_);
		for (int k = base_ - 1; k; --k) pull(k);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	S all_prod() const { return data_[1]; }
	S get(int p) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		return data_[k];
	}
	void set(int p, const S &value) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		data_[k] = value;
		while (k >>= 1) pull(k);
	}
	S prod(int l, int r) {
		check(l, r);
		if (l == r) return e();
		l += base_, r += base_;
		push_range(l, r);
		S left = e(), right = e();
		while (l < r) {
			if (l & 1) left = op(std::move(left), data_[l++]);
			if (r & 1) right = op(data_[--r], std::move(right));
			l >>= 1, r >>= 1;
		}
		return op(std::move(left), std::move(right));
	}
	void apply(const F &f) { if (n_) all_apply(1, f); }
	void apply(int p, const F &f) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		all_apply(k, f);
		while (k >>= 1) pull(k);
	}
	void apply(int l, int r, const F &f) {
		check(l, r);
		if (l == r) return;
		l += base_, r += base_;
		push_range(l, r);
		int first = l, last = r;
		while (l < r) {
			if (l & 1) all_apply(l++, f);
			if (r & 1) all_apply(--r, f);
			l >>= 1, r >>= 1;
		}
		pull_range(first, last);
	}
	template <auto f> int max_right(int l) { return max_right(l, f); }
	template <class Predicate>
	int max_right(int l, Predicate predicate) {
		assert(0 <= l && l <= n_ && predicate(e()));
		if (l == n_) return n_;
		int k = l + base_;
		push_path(k);
		S value = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(value, data_[k]);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k *= 2;
					next = op(value, data_[k]);
					if (predicate(next)) value = std::move(next), ++k;
				}
				return std::min(k - base_, n_);
			}
			value = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template <auto f> int min_left(int r) { return min_left(r, f); }
	template <class Predicate>
	int min_left(int r, Predicate predicate) {
		assert(0 <= r && r <= n_ && predicate(e()));
		if (!r) return 0;
		int k = r + base_;
		push_path(k - 1);
		S value = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], value);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k = 2 * k + 1;
					next = op(data_[k], value);
					if (predicate(next)) value = std::move(next), --k;
				}
				return k + 1 - base_;
			}
			value = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
	std::vector<S> values() {
		for (int k = 1; k < base_; ++k) push(k);
		return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
	}
};

#endif
