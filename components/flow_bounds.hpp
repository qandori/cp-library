#ifndef FLOW_BOUNDS_HPP
#define FLOW_BOUNDS_HPP

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace flow_detail {

template <class Cap>
struct CapacityBounds {
	static_assert(std::is_integral_v<Cap> && std::is_signed_v<Cap> && sizeof(Cap) <= 8);
	using Wide = __int128_t;
	using Residual = std::make_unsigned_t<Cap>;
	static Cap value(Wide amount) {
		assert(std::numeric_limits<Cap>::min() <= amount && amount <= std::numeric_limits<Cap>::max());
		return Cap(amount);
	}
	static Residual amount(Wide value) {
		assert(0 <= value && value <= Wide(std::numeric_limits<Residual>::max()));
		return Residual(value);
	}
	static Residual capacity(Cap lower, Cap upper) {
		assert(lower <= upper);
		return amount(Wide(upper) - Wide(lower));
	}
};

}

#endif
