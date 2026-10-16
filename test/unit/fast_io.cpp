#include "../../components/fast_io.hpp"
#include "../../math/bigint.hpp"
#include <cassert>
#include <limits>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <thread>
#endif

void input(FastInput &in, const std::string &long_value) {
	int t;
	long long low;
	unsigned long long high;
	BigInt a;
	std::string s;
	assert(in.read(t) && t == -123);
	assert(in.read(low) && low == std::numeric_limits<long long>::min());
	assert(in.read(high) && high == std::numeric_limits<unsigned long long>::max());
	assert(in.read(a) && a.str() == long_value);
	assert(in.read(s) && s == "end");
	assert(!in.read(t) && in.token().empty());
}

int main() {
	std::string number(200003, '7');
	std::string text = "\t -123\r\n-9223372036854775808 18446744073709551615 " + number + "\nend";
	FastInput memory(text);
	input(memory, number);
	FILE *file = std::tmpfile();
	assert(file);
	{
		FastOutput out(file);
		out.write('\t');
		out.println(-123);
		out.println(std::numeric_limits<long long>::min());
		out.write(std::numeric_limits<unsigned long long>::max());
		out.write(' ');
		out.println(BigInt(number));
		out.write("end");
		assert(out.flush());
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		assert(fstat(fileno(file), &status) == 0 && status.st_size > 0);
#endif
	}
	std::rewind(file);
	{
		FastInput mapped(file);
		input(mapped, number);
	}
	std::fclose(file);
#if defined(__unix__) || defined(__APPLE__)
	int descriptors[2];
	assert(pipe(descriptors) == 0);
	std::thread writer([&] {
		FILE *out = fdopen(descriptors[1], "w");
		assert(out);
		assert(std::fwrite(text.data(), 1, text.size(), out) == text.size());
		std::fclose(out);
	});
	file = fdopen(descriptors[0], "r");
	assert(file);
	{
		FastInput piped(file);
		input(piped, number);
	}
	std::fclose(file);
	writer.join();
#endif
	bool flag = false;
	FastInput bits("1 0");
	assert(bits.read(flag) && flag);
	assert(bits.read(flag) && !flag);
	for (int n : {0, 1, 65535, 65536, 65537, 131072}) {
		std::string s(n, ' ');
		FastInput in(s);
		assert(in.token().empty());
	}
	for (int n = 1; n <= 40; ++n) {
		BigInt x("-" + std::string(n, '9'));
		std::vector<char> buffer(x.decimal_capacity() + 1, '#');
		char *end = x.to_chars(buffer.data());
		assert(std::string(buffer.data(), end) == x.str());
		assert(buffer[x.decimal_capacity()] == '#');
	}
}
