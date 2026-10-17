#include "components/fast_io.hpp"
#include "test/benchmark/fast_io_integer_baseline.hpp"
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <csignal>
#include <sys/wait.h>
#include <thread>
#endif

void require(bool condition) {
	if (!condition) std::abort();
}

template <class Input, class T>
std::vector<T> collect(Input &input) {
	std::vector<T> values;
	for (;;) {
		T value = T(77);
		if (!input.read(value)) {
			require(value == T(77));
			break;
		}
		values.push_back(value);
	}
	T value = T(77);
	require(!input.read(value) && value == T(77) && input.token().empty());
	return values;
}

template <class Input, class T>
std::vector<T> read_source(const std::string &text, int source) {
	if (!source) {
		Input input{std::string_view(text)};
		return collect<Input, T>(input);
	}
	FILE *file = nullptr;
#if defined(__unix__) || defined(__APPLE__)
	std::thread writer;
	if (source == 2) {
		int descriptors[2];
		require(pipe(descriptors) == 0);
		file = fdopen(descriptors[0], "r");
		require(file);
		writer = std::thread([&text, fd = descriptors[1]] {
			FILE *output = fdopen(fd, "w");
			require(output);
			require(std::fwrite(text.data(), 1, text.size(), output) == text.size());
			require(std::fclose(output) == 0);
		});
	} else
#endif
	{
		file = std::tmpfile();
		require(file);
		require(std::fwrite("prefix", 1, 6, file) == 6);
		require(std::fwrite(text.data(), 1, text.size(), file) == text.size());
		require(std::fseek(file, 6, SEEK_SET) == 0);
	}
	std::vector<T> values;
	{
		Input input(file);
		values = collect<Input, T>(input);
	}
	require(std::fclose(file) == 0);
#if defined(__unix__) || defined(__APPLE__)
	if (writer.joinable()) writer.join();
#endif
	return values;
}

template <class T>
void compare(const std::string &text) {
	auto expected = read_source<FastInputIntegerBaseline, T>(text, 0);
	for (int source = 0; source < 3; ++source) {
		require(read_source<FastInputIntegerBaseline, T>(text, source) == expected);
		require(read_source<FastInput, T>(text, source) == expected);
	}
}

template <class T>
void valid() {
	const auto low = std::to_string(std::numeric_limits<T>::min());
	const auto high = std::to_string(std::numeric_limits<T>::max());
	std::string text = low + ' ' + high + " 0 1 7 ";
	if constexpr (std::is_signed_v<T>) text += "-0 -1 -7 ";
	for (int c = 0; c <= ' '; ++c) text += std::string(3, char(c)) + "7";
	for (int offset = -24; offset <= 24; ++offset) {
		std::size_t position = ((text.size() / 65536) + 1) * 65536 + offset;
		if (position < text.size()) position += 65536;
		text.append(position - text.size(), ' ');
		text += high + ' ' + low + '\n';
	}
	text += std::string(131073, '0') + "7 ";
	std::mt19937 generator(20261017);
	for (int i = 0; i < 10000; ++i) text += std::to_string(generator() % 100) + (i % 2 ? "\r\n" : "\t");
	text += high;
	compare<T>(text);
	for (auto token : {low, high, std::string("0")}) {
		compare<T>(token);
		compare<T>(std::string(65536 - token.size(), ' ') + token);
	}
}

struct Token {
	std::string text;
	explicit Token(std::string_view value) : text(value) {}
};

template <class Input>
void mixed() {
	std::string text = "\t7 1 custom -8 0 " + std::string(131073, 'x') + " 9";
	Input input{std::string_view(text)};
	int value = 0;
	bool flag = false;
	Token token("initial");
	require(input.read(value) && value == 7);
	require(input.read(flag) && flag);
	require(input.read(token) && token.text == "custom");
	require(input.read(value) && value == -8);
	require(input.read(flag) && !flag);
	require(input.token() == std::string(131073, 'x'));
	require(input.read(value) && value == 9);
	require(!input.read(token) && token.text == "custom");
}

#ifdef NDEBUG
template <class T>
void invalid() {
	std::string text = "+1 - --1 abc 12x 1-2 0x10 99999999999999999999999999999999999999 ";
	if constexpr (std::is_unsigned_v<T>) text += "-0 -1 -18446744073709551615 ";
	for (int c = 33; c < 256; ++c)
		if (c < '0' || c > '9') text += "12" + std::string(1, char(c)) + "7 ";
	for (int offset = -24; offset <= 24; ++offset) {
		std::size_t position = ((text.size() / 65536) + 1) * 65536 + offset;
		if (position < text.size()) position += 65536;
		text.append(position - text.size(), ' ');
		text += "184467440737095516150 12x +1 - 7 ";
	}
	text += std::string(131073, '0') + "999999999999999999999999999 7";
	compare<T>(text);
	compare<T>("99999999999999999999999999999999");
	compare<T>("123x");
}
#else
#if defined(__unix__) || defined(__APPLE__)
template <class Input>
void rejects(const std::string &text) {
	pid_t pid = fork();
	require(pid >= 0);
	if (!pid) {
		std::freopen("/dev/null", "w", stderr);
		Input input{std::string_view(text)};
		int value = 77;
		input.read(value);
		std::_Exit(0);
	}
	int status;
	require(waitpid(pid, &status, 0) == pid);
	require(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}
#endif
#endif

int main() {
	valid<char>();
	valid<signed char>();
	valid<unsigned char>();
	valid<short>();
	valid<unsigned short>();
	valid<int>();
	valid<unsigned>();
	valid<long>();
	valid<unsigned long>();
	valid<long long>();
	valid<unsigned long long>();
	mixed<FastInput>();
	mixed<FastInputIntegerBaseline>();
	compare<bool>("0 1\n1\t0");
	for (int n : {0, 1, 65535, 65536, 65537, 131072}) compare<int>(std::string(n, ' '));
#ifdef NDEBUG
	invalid<signed char>();
	invalid<unsigned char>();
	invalid<short>();
	invalid<unsigned short>();
	invalid<int>();
	invalid<unsigned>();
	invalid<long long>();
	invalid<unsigned long long>();
	compare<bool>("2 10 -1 01 abc");
#else
#if defined(__unix__) || defined(__APPLE__)
	for (const char *text : {"+1", "-", "12x", "99999999999999999999999999"}) {
		rejects<FastInput>(text);
		rejects<FastInputIntegerBaseline>(text);
	}
#endif
#endif
}
