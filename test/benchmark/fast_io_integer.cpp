#include "components/fast_io.hpp"
#include "fast_io_integer_baseline.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <thread>
#endif

void require(bool condition) {
	if (!condition) std::abort();
}

struct Timing { double wall, cpu; };

int selected_source = -1;

template <class Input, class T>
Timing consume(Input &input, std::vector<T> &values) {
	auto cpu_start = std::clock();
	auto start = std::chrono::steady_clock::now();
	for (auto &value : values) require(input.read(value));
	T last{};
	require(!input.read(last));
	auto finish = std::chrono::steady_clock::now();
	auto cpu_finish = std::clock();
	return {std::chrono::duration<double, std::milli>(finish - start).count(),
		1000.0 * (cpu_finish - cpu_start) / CLOCKS_PER_SEC};
}

template <class Input, class T>
Timing run(const std::string &text, int source, FILE *mapped, std::vector<T> &values) {
	if (!source) {
		Input input{std::string_view(text)};
		return consume(input, values);
	}
	if (source == 1) {
		std::rewind(mapped);
		Input input(mapped);
		return consume(input, values);
	}
#if defined(__unix__) || defined(__APPLE__)
	int descriptors[2];
	require(pipe(descriptors) == 0);
	FILE *file = fdopen(descriptors[0], "r");
	require(file);
	std::thread writer([&text, fd = descriptors[1]] {
		FILE *output = fdopen(fd, "w");
		require(output);
		require(std::fwrite(text.data(), 1, text.size(), output) == text.size());
		require(std::fclose(output) == 0);
	});
	Timing elapsed;
	{
		Input input(file);
		elapsed = consume(input, values);
	}
	require(std::fclose(file) == 0);
	writer.join();
	return elapsed;
#else
	return {};
#endif
}

template <class T>
void benchmark(const char *name, std::size_t count, int rounds, int kind) {
	std::mt19937_64 generator(20261017);
	std::vector<T> expected(count), actual(count);
	std::string text;
	text.reserve(count * 22);
	for (std::size_t i = 0; i < count; ++i) {
		if (kind == 0) expected[i] = std::bit_cast<std::int64_t>(generator());
		else if (kind == 1) expected[i] = generator();
		else if (kind == 2) expected[i] = int(generator() % 2001) - 1000;
		else expected[i] = generator() % 10;
		if (kind == 4) text += std::string(64, '0');
		text += std::to_string(expected[i]);
		text += (i & 1 ? '\n' : ' ');
	}
	if (!text.empty()) text.pop_back();
	FILE *file = std::tmpfile();
	require(file);
	require(std::fwrite(text.data(), 1, text.size(), file) == text.size());
	require(std::fflush(file) == 0);
	for (int source = 0; source < 3; ++source) {
		if (selected_source >= 0 && source != selected_source) continue;
#if !defined(__unix__) && !defined(__APPLE__)
		if (source == 2) continue;
#endif
		run<FastInputIntegerBaseline>(text, source, file, actual);
		require(actual == expected);
		run<FastInput>(text, source, file, actual);
		require(actual == expected);
		std::vector<double> times[2], cpu_times[2], paired;
		for (int round = 0; round < rounds; ++round) {
			Timing pair[2];
			for (int turn = 0; turn < 2; ++turn) {
				int variant = (round + turn) % 2;
				Timing elapsed = variant ? run<FastInput>(text, source, file, actual)
					: run<FastInputIntegerBaseline>(text, source, file, actual);
				require(actual == expected);
				times[variant].push_back(elapsed.wall);
				cpu_times[variant].push_back(elapsed.cpu);
				pair[variant] = elapsed;
			}
			paired.push_back(pair[0].wall / pair[1].wall);
		}
		for (auto &time : times) std::sort(time.begin(), time.end());
		for (auto &time : cpu_times) std::sort(time.begin(), time.end());
		std::sort(paired.begin(), paired.end());
		const char *mode[] = {"memory", "mmap", "pipe"};
		std::cout << name << ',' << mode[source] << ',' << count << ',' << text.size() << ','
			<< times[0][rounds / 2] << ',' << times[1][rounds / 2] << ','
			<< times[0][rounds / 2] / times[1][rounds / 2] << ',' << paired[rounds / 2] << ','
			<< cpu_times[0][rounds / 2] << ',' << cpu_times[1][rounds / 2] << ','
			<< cpu_times[0][rounds / 2] / cpu_times[1][rounds / 2] << '\n' << std::flush;
	}
	require(std::fclose(file) == 0);
}

int main(int argc, char **argv) {
	std::size_t count = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1000000;
	int rounds = argc > 2 ? std::atoi(argv[2]) : 11;
	std::string selected_case = argc > 3 ? argv[3] : "all";
	selected_source = argc > 4 ? std::atoi(argv[4]) : -1;
	require(count && rounds > 0 && selected_source >= -1 && selected_source < 3);
	std::cout << std::fixed << std::setprecision(6);
	std::cout << "case,source,count,bytes,baseline_ms,optimized_ms,speedup,paired_speedup,baseline_cpu_ms,optimized_cpu_ms,cpu_speedup\n";
	if (selected_case == "all" || selected_case == "signed64") benchmark<std::int64_t>("signed64", count, rounds, 0);
	if (selected_case == "all" || selected_case == "unsigned64") benchmark<std::uint64_t>("unsigned64", count, rounds, 1);
	if (selected_case == "all" || selected_case == "unsigned32") benchmark<std::uint32_t>("unsigned32", count, rounds, 1);
	if (selected_case == "all" || selected_case == "small_signed") benchmark<std::int32_t>("small_signed", count, rounds, 2);
	if (selected_case == "all" || selected_case == "single_digit") benchmark<std::uint32_t>("single_digit", count, rounds, 3);
	if (selected_case == "all" || selected_case == "leading_zeros") benchmark<std::uint64_t>("leading_zeros", count / 8 + 1, rounds, 4);
}
