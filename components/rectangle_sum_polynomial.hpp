#ifndef RECTANGLE_SUM_POLYNOMIAL_HPP
#define RECTANGLE_SUM_POLYNOMIAL_HPP

namespace rectangle_sum_detail {

template <class T>
struct Polynomial {
	T weight{}, x{}, y{}, xy{};
	static Polynomial corner(const T &a, const T &b, const T &value) {
		T ax = value * a, by = value * b;
		return {value, ax, by, ax * b};
	}
	Polynomial &operator+=(const Polynomial &other) {
		weight += other.weight; x += other.x; y += other.y; xy += other.xy;
		return *this;
	}
	Polynomial &operator-=(const Polynomial &other) {
		weight -= other.weight; x -= other.x; y -= other.y; xy -= other.xy;
		return *this;
	}
	T evaluate(const T &a, const T &b) const { return (weight * a - x) * b - y * a + xy; }
};

}

#endif
