#include "test/fps_io.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static void require(bool value) { if (!value) std::abort(); }

static void check(FastInput &input, const std::vector<unsigned> &values) {
	const fps actual = read_fps(input, values.size());
	require(actual.size() == values.size());
	for (std::size_t i = 0; i < values.size(); ++i) require(actual[i] == mint::raw(values[i]));
	unsigned last = 0;
	require(input.read(last) && last == 1234567);
	FILE *file = std::tmpfile();
	require(file != nullptr);
	{
		FastOutput output(file);
		write_fps(output, actual);
		require(output.flush());
	}
	std::rewind(file);
	{
		FastInput output(file);
		for (auto expected : actual) {
			unsigned x = 0;
			require(output.read(x) && x == expected.val());
		}
		require(output.token().empty());
	}
	std::fclose(file);
}

int main() {
	std::mt19937 rng(20261018);
	for (int n : {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 4095, 4096, 4097, 20000}) {
		std::vector<unsigned> values(n);
		for (auto &x : values) x = rng();
		const unsigned edge[] = {0, 1, 9, 10, 99, 100, 9999, 10000, 99999999, 100000000, 998244352, 4294967295u};
		for (std::size_t i = 0; i < values.size() && i < std::size(edge); ++i) values[i] = edge[i];
		for (bool irregular : {false, true}) {
			std::string text;
			for (auto x : values) text += std::to_string(x) + (irregular ? "\r\n \t " : " ");
			text += "1234567";
			{
				FastInput input{std::string_view(text)};
				check(input, values);
			}
#if defined(__linux__)
			FILE *memory = fmemopen(text.data(), text.size(), "r");
			require(memory != nullptr);
			{
				FastInput input(memory);
				check(input, values);
			}
			std::fclose(memory);
			const auto page = std::size_t(sysconf(_SC_PAGESIZE));
			const auto bytes = (text.size() + page - 1) / page * page;
			void *mapping = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			require(mapping != MAP_FAILED);
			require(mprotect(static_cast<char *>(mapping) + bytes, page, PROT_NONE) == 0);
			char *first = static_cast<char *>(mapping) + bytes - text.size();
			std::memcpy(first, text.data(), text.size());
			{
				FastInput input{std::string_view(first, text.size())};
				check(input, values);
			}
			munmap(mapping, bytes + page);
#endif
		}
	}
}
