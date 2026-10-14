#ifndef DISJOINT_SPARSE_TABLE_HPP
#define DISJOINT_SPARSE_TABLE_HPP

#include "../template.hpp"

template <class S, auto op, auto e = nullptr>
class DisjointSparseTable {
	template <class T>
	struct Alloc : allocator<T> {
		template <class U> struct rebind { using other = Alloc<U>; };
		Alloc() = default;
		template <class U> Alloc(const Alloc<U> &) noexcept {}
		template <class U> void construct(U *p) { ::new (static_cast<void *>(p)) U; }
	};
	using Value = conditional_t<is_same_v<S, bool>, uint8_t, S>;
	using Reference = conditional_t<is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	vec<vec<Value, Alloc<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (is_same_v<S, bool>) return bool(op(bool(a), bool(b)));
		else return op(a, b);
	}
public:
	DisjointSparseTable() = default;
	DisjointSparseTable(const DisjointSparseTable &) = default;
	DisjointSparseTable &operator=(const DisjointSparseTable &) = default;
	DisjointSparseTable(DisjointSparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	DisjointSparseTable &operator=(DisjointSparseTable &&other) noexcept {
		if (this != &other) { n = std::exchange(other.n, 0); data = std::move(other.data); }
		return *this;
	}
	explicit DisjointSparseTable(const vec<S> &a) { build(a); }
	void build(const vec<S> &a) {
		assert(a.size() <= size_t(numeric_limits<int>::max()));
		n = sz(a);
		int levels = max(1, int(bit_width(unsigned(max(0, n - 1)))));
		data.clear(); data.resize(levels);
		rep(k, levels) {
			size_t length = n, half = size_t(1) << k;
			if (k && (length & (half * 2 - 1)) <= half) length &= ~(half * 2 - 1);
			if constexpr (is_default_constructible_v<Value>) data[k].resize(length);
			else data[k].assign(a.begin(), a.begin() + length);
		}
		rep(i, n) data[0][i] = a[i];
		rep(k, 1, levels) {
			Value *row = data[k].data();
			size_t half = size_t(1) << k, block = half * 2;
			for (size_t start = 0; start < data[k].size(); start += block) {
				size_t mid = start + half, end = min(start + block, data[k].size());
				row[mid - 1] = a[mid - 1]; row[mid] = a[mid];
				for (size_t i = mid - 1; i > start; --i) row[i - 1] = combine(a[i - 1], row[i]);
				for (size_t i = mid + 1; i < end; ++i) row[i] = combine(row[i - 1], a[i]);
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	Reference operator[](int i) const { assert(0 <= i && i < n); return data[0][i]; }
	[[gnu::always_inline]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if constexpr (is_same_v<decltype(e), nullptr_t>) assert(l < r);
		else if (l == r) return e();
		if (l == --r) return data[0][l];
		const auto &row = data[bit_width(unsigned(l ^ r)) - 1];
		return combine(row[l], row[r]);
	}
	S all_prod() const { return prod(0, n); }
};

#endif
