#ifndef PRIME_SIEVE_HPP
#define PRIME_SIEVE_HPP

#include <cstddef>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<bool> composite(n);
	std::vector<std::size_t> result;
	for (std::size_t p = 2; p < n; ++p) {
		if (composite[p]) continue;
		result.push_back(p);
		if (p > (n - 1) / p) continue;
		for (std::size_t j = p * p; j < n; j += p) composite[j] = true;
	}
	return result;
}

#endif
