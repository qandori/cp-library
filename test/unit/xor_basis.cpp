#include "math/xor_basis.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <set>
#include <vector>

template <class U>
void append(std::vector<U> &span, U x) {
	std::vector<U> next = span;
	for (U y : span) next.push_back(U(y ^ x));
	std::sort(next.begin(), next.end());
	next.erase(std::unique(next.begin(), next.end()), next.end());
	span.swap(next);
}

template <class U, int B>
void check(const XorBasis<U, B> &basis, const std::vector<U> &span, U offset) {
	assert(basis.count() == span.size());
	assert(basis.empty() == (span.size() == 1));
	assert((std::size_t(1) << basis.rank()) == span.size());
	auto values = span;
	for (U &x : values) x ^= offset;
	std::sort(values.begin(), values.end());
	assert(basis.min_xor(offset) == values.front());
	assert(basis.max_xor(offset) == values.back());
	assert(basis.reduce(offset) == values.front());
	assert(!basis.kth(values.size(), offset));
	assert(basis.min_nonzero() == (span.size() > 1 ? std::optional<U>(span[1]) : std::nullopt));
	std::size_t mex = 0;
	while (mex < values.size() && values[mex] == mex) ++mex;
	assert(basis.mex(offset) == mex);
	for (std::size_t k = 0; k < span.size(); ++k) {
		assert(basis.kth(k, offset) == values[k]);
		assert(basis.index_of(values[k], offset) == k);
		auto coordinates = basis.coordinates(span[k]);
		assert(coordinates && basis.combine(*coordinates) == span[k]);
	}
	auto generators = basis.vectors();
	assert(int(generators.size()) == basis.rank());
	assert((XorBasis<U, B>(generators) == basis));
	auto canonical = basis.canonical();
	assert(canonical == basis && canonical.canonical().vectors() == canonical.vectors());
	std::reverse(generators.begin(), generators.end());
	assert((XorBasis<U, B>(generators).canonical().vectors() == canonical.vectors()));
	for (std::uint64_t query : {0ULL, 1ULL, 2ULL, 7ULL, 15ULL, 31ULL, 63ULL, 127ULL, 255ULL,
		0x80000000ULL, 0xffffffffULL, 0x8000000000000000ULL, 0xffffffffffffffffULL}) {
		U x = U(query);
		auto lower = std::lower_bound(values.begin(), values.end(), x);
		auto upper = std::upper_bound(values.begin(), values.end(), x);
		assert(basis.count_less(x, offset) == std::size_t(lower - values.begin()));
		assert(basis.count_leq(x, offset) == std::size_t(upper - values.begin()));
		assert(basis.lower_bound(x, offset) == (lower == values.end() ? std::nullopt : std::optional<U>(*lower)));
		assert(basis.upper_bound(x, offset) == (upper == values.end() ? std::nullopt : std::optional<U>(*upper)));
		assert(basis.predecessor(x, offset) == (lower == values.begin() ? std::nullopt : std::optional<U>(*(lower - 1))));
		assert(basis.contains(x) == std::binary_search(span.begin(), span.end(), x));
		assert(bool(basis.coordinates(x)) == basis.contains(x));
		assert(bool(basis.index_of(x, offset)) == std::binary_search(values.begin(), values.end(), x));
		assert(basis.count_range(U(0), x, offset) == std::size_t(lower - values.begin()));
		assert(basis.count_range(x, x, offset) == 0);
	}
}

void small_spaces() {
	using Basis = XorBasis<std::uint8_t, 4>;
	std::vector<Basis> spaces(1);
	std::set<std::vector<std::uint8_t>> seen;
	seen.insert({});
	for (std::size_t i = 0; i < spaces.size(); ++i) for (int x = 1; x < 16; ++x) {
		auto next = spaces[i]; next.insert(x); next = next.canonical();
		if (seen.insert(next.vectors()).second) spaces.push_back(next);
	}
	assert(spaces.size() == 67);
	std::vector<std::vector<std::uint8_t>> spans;
	for (const auto &basis : spaces) {
		std::vector<std::uint8_t> values;
		for (int x = 0; x < 16; ++x) if (basis.contains(x)) values.push_back(x);
		spans.push_back(values);
		for (int x = 0; x < 32; ++x) check(basis, values, std::uint8_t(x));
		auto perpendicular = basis.orthogonal_complement();
		assert(perpendicular.rank() + basis.rank() == 4 && perpendicular.orthogonal_complement() == basis);
		for (int x = 0; x < 16; ++x) {
			bool expected = true;
			for (auto y : values) if (std::popcount(unsigned(x & y)) & 1) expected = false;
			assert(perpendicular.contains(x) == expected);
		}
		auto self = basis;
		assert(self.merge(self) == 0 && self == basis);
		self &= self; assert(self == basis);
		self.clear(); assert(self.empty() && self.count() == 1);
	}
	for (std::size_t i = 0; i < spaces.size(); ++i) for (std::size_t j = 0; j < spaces.size(); ++j) {
		auto intersection = spaces[i] & spaces[j];
		auto sum = spaces[i] | spaces[j];
		assert(intersection.rank() + sum.rank() == spaces[i].rank() + spaces[j].rank());
		for (int x = 0; x < 16; ++x) assert(intersection.contains(x) == (spaces[i].contains(x) && spaces[j].contains(x)));
		assert(spaces[i].is_subspace_of(spaces[j]) == (intersection == spaces[i]));
		auto expected = spans[i]; for (auto x : spans[j]) append(expected, x);
		check(sum, expected, std::uint8_t(0));
		auto copy = spaces[i]; int before = copy.rank();
		assert(copy.merge(spaces[j]) == sum.rank() - before && copy == sum);
		copy = spaces[i]; copy |= spaces[j]; assert(copy == sum);
		copy = spaces[i]; copy &= spaces[j]; assert(copy == intersection);
	}
}

template <class U, int B>
void random_small(std::mt19937_64 &random) {
	constexpr U mask = xor_basis_detail::Core<U, B>::mask;
	for (int round = 0; round < 60; ++round) {
		XorBasis<U, B> basis;
		XorBasisWithWitness<U, B> witness;
		PrefixXorBasis<U, B> prefix;
		std::vector<U> input, span{0};
		for (int i = 0; i < 9; ++i) {
			U x = U(random()) & mask;
			if (i % 4 == 0) x = i ? input.back() : U(0);
			bool independent = !std::binary_search(span.begin(), span.end(), x);
			assert(basis.insert(x) == independent);
			assert(witness.insert(x) == independent && prefix.push_back(x) == independent);
			input.push_back(x); append(span, x);
			assert(witness.basis() == basis && witness.size() == input.size());
			assert(witness.basis().kth(basis.count() - 1) == basis.max_xor());
			check(basis, span, U(random()));
			for (U value : span) {
				auto answer = witness.represent(value);
				assert(answer && std::is_sorted(answer->begin(), answer->end()));
				U actual = 0;
				for (auto id : *answer) { assert(id < input.size()); actual ^= input[id]; }
				assert(actual == value);
			}
			for (int query = 0; query < 5; ++query) {
				U value = U(random());
				assert(bool(witness.represent(value)) == basis.contains(value));
			}
			for (std::size_t l = 0; l <= input.size(); ++l) {
				XorBasis<U, B> expected(std::vector<U>(input.begin() + l, input.end()));
				U offset = U(random());
				assert(prefix.basis(l) == expected && prefix.rank(l) == expected.rank());
				assert(prefix.basis(l).kth(expected.count() - 1, offset) == expected.max_xor(offset));
				assert(prefix.max_xor(l, offset) == expected.max_xor(offset));
				assert(prefix.min_xor(l, offset) == expected.min_xor(offset));
				assert(prefix.contains(l, offset) == expected.contains(offset));
			}
		}
		assert(prefix.basis() == basis && prefix.size() == input.size());
		witness.clear(); prefix.clear();
		assert(witness.size() == 0 && witness.represent(U(0))->empty());
		assert(prefix.size() == 0 && prefix.basis().empty());
		assert(witness.insert(U(1)) && *witness.represent(U(1)) == std::vector<std::size_t>{0});
		assert(prefix.push_back(U(1)) && prefix.basis().contains(U(1)) && prefix.basis(1).empty());
	}
}

void rollback_test(std::mt19937_64 &random) {
	RollbackXorBasis<std::uint64_t, 16> basis;
	std::vector<std::uint64_t> input;
	for (int step = 0; step < 2000; ++step) {
		if (input.empty() || random() % 3) {
			auto token = basis.snapshot();
			auto value = random() % 65536;
			basis.insert(value); input.push_back(value);
			assert(basis.snapshot() == token + 1);
		} else {
			std::size_t token = random() % (input.size() + 1);
			basis.rollback(token); input.resize(token);
		}
		assert((basis.basis() == XorBasis<std::uint64_t, 16>(input)));
		assert(basis.basis().kth(basis.basis().count() - 1) == basis.basis().max_xor());
	}
	while (!input.empty()) { basis.undo(); input.pop_back(); assert(basis.snapshot() == input.size()); }
	assert(basis.basis().empty());
	basis.insert(0); basis.insert(1); basis.insert(1); basis.undo();
	assert(basis.basis().rank() == 1); basis.undo(); assert(basis.basis().empty());
	basis.clear(); assert(basis.snapshot() == 0);
}

template <class U>
void full_width() {
	constexpr int B = std::numeric_limits<U>::digits;
	XorBasis<U> full;
	XorBasisWithWitness<U> witness;
	std::vector<U> input;
	for (int i = B - 1; i >= 0; --i) { U x = U(U(1) << i); assert(full.insert(x) && witness.insert(x)); input.push_back(x); }
	using C = typename XorBasis<U>::count_type;
	C count = C(1) << B;
	U maximum = std::numeric_limits<U>::max();
	assert(full.count() == count && full.mex() == count);
	assert(full.min_xor(maximum) == 0 && full.max_xor(maximum) == maximum);
	assert(full.kth(count - 1) == maximum && !full.kth(count));
	assert(full.count_less(maximum) == count - 1 && full.count_leq(maximum) == count);
	assert(full.count_less(count) == count && full.count_range(0, count) == count);
	assert(full.predecessor(count) == maximum && !full.index_of(count) && !full.lower_bound(count));
	assert(full.index_of(maximum) == count - 1 && !full.upper_bound(maximum));
	assert(!full.predecessor(0) && full.predecessor(maximum) == U(maximum - 1));
	assert(full.orthogonal_complement().empty());
	assert(!full.insert(maximum) && !witness.insert(maximum));
	assert(full.combine(maximum) == maximum && full.coordinates(maximum) == maximum);
	auto represented = witness.represent(maximum);
	U restored = 0; for (auto i : *represented) restored ^= input[i];
	assert(restored == maximum);
	XorBasis<U> half{U(U(1) << (B - 1))};
	assert((full & half) == half && (half & full) == half);
	assert(half.count_leq(maximum, maximum) == 2 && half.count_less(maximum, maximum) == 1);
}

template <class U, int B>
void intersections(std::mt19937_64 &random) {
	for (int round = 0; round < 100; ++round) {
		std::vector<U> columns;
		for (int i = 0; i < B; ++i) columns.push_back(U((U(1) << i) | (U(random()) & U((U(1) << i) - 1))));
		std::shuffle(columns.begin(), columns.end(), random);
		std::vector<U> a, b;
		XorBasis<U, B> expected;
		for (int i = 0; i < B; ++i) {
			bool left = round % 2 ? i != 0 : random() % 2;
			bool right = round % 2 ? i != B - 1 : random() % 3;
			if (left) a.push_back(columns[i]);
			if (right) b.push_back(columns[i]);
			if (left && right) expected.insert(columns[i]);
		}
		std::shuffle(a.begin(), a.end(), random); std::shuffle(b.begin(), b.end(), random);
		XorBasis<U, B> x(a), y(b);
		assert((x & y) == expected && (y & x) == expected);
		assert((x | y).rank() + expected.rank() == x.rank() + y.rank());
		auto perpendicular = x.orthogonal_complement();
		assert(perpendicular.rank() + x.rank() == B && perpendicular.orthogonal_complement() == x);
		for (auto v : perpendicular.vectors()) for (auto w : a) assert(!(std::popcount(std::uint64_t(v & w)) & 1));
	}
}

int main() {
	small_spaces();
	std::mt19937_64 random(713);
	random_small<std::uint8_t, 5>(random);
	random_small<std::uint16_t, 12>(random);
	random_small<std::uint32_t, 30>(random);
	random_small<std::uint64_t, 64>(random);
	rollback_test(random);
	full_width<std::uint8_t>(); full_width<std::uint16_t>();
	full_width<std::uint32_t>(); full_width<std::uint64_t>();
	intersections<std::uint32_t, 32>(random); intersections<std::uint64_t, 31>(random);
	intersections<std::uint64_t, 64>(random);
	XorBasis<std::uint64_t, 1> one{1};
	assert(one.max_xor(8) == 9 && one.min_xor(8) == 8 && !one.contains(8));
	assert(one.count_less(8, 8) == 0 && one.count_leq(9, 8) == 2 && one.mex(8) == 0);
}
