#ifndef REROOTING_SCREEN_KERNEL_HPP
#define REROOTING_SCREEN_KERNEL_HPP
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

struct PairData {
	std::int64_t sum;
	std::uint32_t count;
	bool operator==(const PairData&) const = default;
};
using Matrix = std::array<std::uint32_t, 16>;
struct Input {
	int n, root;
	std::vector<std::pair<int, int>> edges;
	std::vector<Matrix> matrices;
};
class Result {
	void *value_ = nullptr;
	void (*destroy_)(void *) = nullptr;
  public:
	Result() = default;
	Result(void *value, void (*destroy)(void *)) : value_(value), destroy_(destroy) {}
	Result(const Result&) = delete;
	Result& operator=(const Result&) = delete;
	Result(Result&& other) noexcept : value_(std::exchange(other.value_, nullptr)), destroy_(other.destroy_) {}
	Result& operator=(Result&& other) noexcept {
		if (value_) destroy_(value_);
		value_ = std::exchange(other.value_, nullptr);
		destroy_ = other.destroy_;
		return *this;
	}
	~Result() { if (value_) destroy_(value_); }
	void *get() const { return value_; }
};
Matrix multiply_matrices(const Matrix&, const Matrix&);
Result run_pair(const Input&);
Result run_matrix(const Input&);
std::vector<PairData> snapshot_pair(const Result&);
std::vector<Matrix> snapshot_matrix(const Result&);
#endif
