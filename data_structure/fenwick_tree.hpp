#ifndef FENWICK_TREE_HPP
#define FENWICK_TREE_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <vector>

namespace fenwick_tree_detail {
template<class T, bool = std::is_integral_v<T>> struct Storage { using type = T; };
template<class T> struct Storage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct Storage<__int128_t, false> { using type = __uint128_t; };
#endif
}

template<class T, bool Padded = true> class FenwickTree {
	static_assert(!std::is_same_v<T, bool>);
	using U = typename fenwick_tree_detail::Storage<T>::type;
	std::vector<U> data_;
	int n_ = 0;
	static std::size_t index(unsigned i) {
		if constexpr (Padded) return std::size_t(i) + (i >> 10);
		else return i;
	}
	template<bool Upper> int bound(T x) const {
		if (x < T{}) return 0;
		if constexpr (!Upper) if (!(T{} < x)) return 0;
		U remaining(x);
		unsigned p = 0;
		for (unsigned step = std::bit_floor(unsigned(n_)); step; step >>= 1) {
			unsigned next = p + step;
			if (next <= unsigned(n_) && (Upper ? !(remaining < data_[index(next)]) : data_[index(next)] < remaining)) {
				remaining -= data_[index(next)];
				p = next;
			}
		}
		return int(p);
	}

  public:
	explicit FenwickTree(int n = 0) : n_(n) {
		assert(n >= 0);
		data_.resize(index(unsigned(n)) + 1);
	}
	explicit FenwickTree(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		data_.resize(index(unsigned(n_)) + 1);
		for (unsigned i = 2; i <= unsigned(n_); i += 2) {
			data_[index(i - 1)] += U(a[i - 2]);
			data_[index(i)] += data_[index(i - 1)];
			data_[index(i)] += U(a[i - 1]);
			unsigned p = i + (i & -i);
			if (p <= unsigned(n_)) data_[index(p)] += data_[index(i)];
		}
		if (n_ & 1) data_[index(unsigned(n_))] += U(a.back());
	}
	[[nodiscard]] int size() const { return n_; }
	[[nodiscard]] T get(int p) const { return sum(p, p + 1); }
	void set(int p, T x) {
		U difference(x);
		difference -= U(get(p));
		add(p, T(difference));
	}
	[[nodiscard]] int lower_bound(T x) const { return bound<false>(x); }
	[[nodiscard]] int upper_bound(T x) const { return bound<true>(x); }
	void add(int p, T x) {
		assert(0 <= p && p < n_);
		for (unsigned i = unsigned(p) + 1; i <= unsigned(n_); i += i & -i) data_[index(i)] += U(x);
	}
	[[nodiscard]] T sum(int r) const {
		assert(0 <= r && r <= n_);
		U result{};
		for (unsigned i = unsigned(r); i; i &= i - 1) result += data_[index(i)];
		return T(result);
	}
	[[nodiscard]] T sum(int l, int r) const {
		assert(0 <= l && l <= r && r <= n_);
		U result{};
		unsigned a = unsigned(l), b = unsigned(r);
		while (b > a) result += data_[index(b)], b &= b - 1;
		while (a > b) result -= data_[index(a)], a &= a - 1;
		return T(result);
	}
};

#endif
