#include "components/fast_io.hpp"
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <thread>
#endif

static void require(bool value) {
	if (!value) std::abort();
}

static void check(FastInput &input, const std::vector<unsigned> &values) {
	for (auto expected : values) {
		unsigned actual = 0;
		require(input.read(actual) && actual == expected);
	}
	unsigned extra = 0;
	require(!input.read(extra));
}

int main() {
	std::vector<unsigned> values;
	for (unsigned i = 0; i < 65536; ++i) values.push_back(i);
	for (unsigned long long power = 10; power <= 1000000000; power *= 10)
		for (int offset = -3; offset <= 3; ++offset) values.push_back(power + offset);
	values.push_back(std::numeric_limits<unsigned>::max());
	std::mt19937 rng(731);
	for (int i = 0; i < 100000; ++i) values.push_back(rng());
	std::string text;
	for (auto x : values) {
		char buffer[32];
		auto end = std::to_chars(buffer, buffer + sizeof(buffer), x).ptr;
		text.append(buffer, end);
		text += ' ';
	}
	text += '\n';
	FastInput memory(text);
	check(memory, values);
	FILE *file = std::tmpfile();
	require(file);
	{
		FastOutput output(file);
		for (auto x : values) output.write(x), output.write(' ');
		output.write('\n');
		require(output.flush());
	}
	std::rewind(file);
	std::string actual(text.size(), '\0');
	require(std::fread(actual.data(), 1, actual.size(), file) == actual.size() && actual == text);
	std::rewind(file);
	{
		FastInput mapped(file);
		check(mapped, values);
	}
	std::fclose(file);
	for (std::string token : {"0", "9999999", "10000000", "99999999", "100000000", "999999999", "1000000000", "4294967295", "0000000000000001"}) {
		unsigned expected = 0;
		std::from_chars(token.data(), token.data() + token.size(), expected);
		for (int padding : {0, 1, 2, 3, 16}) {
			std::string input = token + std::string(padding, ' ');
			FastInput in(input);
			check(in, {expected});
		}
	}
#if defined(__unix__) || defined(__APPLE__)
	int descriptors[2];
	require(pipe(descriptors) == 0);
	std::thread writer([&] {
		FILE *output = fdopen(descriptors[1], "w");
		require(output && std::fwrite(text.data(), 1, text.size(), output) == text.size());
		require(std::fclose(output) == 0);
	});
	FILE *input = fdopen(descriptors[0], "r");
	require(input);
	{
		FastInput stream(input);
		check(stream, values);
	}
	std::fclose(input);
	writer.join();
#endif
}
