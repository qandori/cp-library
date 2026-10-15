#include "data_structure/default_init_allocator.hpp"
#include <cassert>
#include <string>
#include <vector>

struct Value {
	static inline int alive = 0;
	int n;
	Value() : n(13) { ++alive; }
	explicit Value(int n) : n(n) { ++alive; }
	Value(const Value &v) : n(v.n) { ++alive; }
	~Value() { --alive; }
};
int main() {
	std::vector<int, DefaultInitAllocator<int>> a(200);
	for (int i = 0; i < 200; ++i) a[i] = i * i;
	auto b = a;
	for (int i = 0; i < 200; ++i) assert(b[i] == i * i);
	{
		std::vector<Value, DefaultInitAllocator<Value>> values(5);
		assert(Value::alive == 5);
		for (auto &v : values) assert(v.n == 13);
		values.emplace_back(19);
		assert(values.back().n == 19 && Value::alive == 6);
	}
	assert(Value::alive == 0);
	std::vector<std::string, DefaultInitAllocator<std::string>> s(2);
	s.emplace_back("abc");
	assert(s[0].empty() && s[2] == "abc");
}
