#include "kernel.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <variant>

namespace {
constexpr std::uint64_t max_repetitions = 128, max_storage = 512ULL << 20, max_rss = 1024ULL << 20;
constexpr std::uint64_t max_elapsed = 2000000000;
std::uint64_t clock_reads = 0;
struct Failure : std::runtime_error {
	std::string code;
	std::map<std::string, std::uint64_t> observations;
	Failure(std::string code, std::string message) : std::runtime_error(std::move(message)), code(std::move(code)) {}
};
void require(bool ok, const char *code, const char *message) { if (!ok) throw Failure(code, message); }
std::string quoted(const std::string& s) {
	std::ostringstream out;
	out << '"';
	for (unsigned char c : s) {
		if (c == '\\' || c == '"') out << '\\' << c;
		else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c);
		else out << c;
	}
	out << '"';
	return out.str();
}
std::string hex(std::uint64_t x) { std::ostringstream s; s << std::hex << std::setw(16) << std::setfill('0') << x; return s.str(); }
std::uint64_t hash_bytes(const std::vector<unsigned char>& bytes) {
	std::uint64_t h = 14695981039346656037ULL;
	for (auto x : bytes) h = (h ^ x) * 1099511628211ULL;
	return h;
}
void u32(std::vector<unsigned char>& bytes, std::uint32_t value) {
	for (int i = 0; i < 4; ++i) bytes.push_back((value >> (8 * i)) & 255);
}
void u64(std::vector<unsigned char>& bytes, std::uint64_t value) {
	for (int i = 0; i < 8; ++i) bytes.push_back((value >> (8 * i)) & 255);
}
struct Spec { int n, shape; bool matrix; };
Spec spec(int cell) {
	require(cell >= 1 && cell <= 10, "invalid_cell", "cell must be 1 through 10");
	return {cell <= 5 ? 32768 : 8192, (cell - 1) % 5, cell > 5};
}
std::uint64_t rss() {
	struct rusage usage {};
	require(getrusage(RUSAGE_SELF, &usage) == 0, "rss_unavailable", "getrusage failed");
	return std::uint64_t(usage.ru_maxrss) * 1024;
}
Input generate(int n, int shape, std::uint32_t seed, bool matrix) {
	Input input{n, 0, {}, {}};
	std::mt19937 rng(seed + 104729U * shape);
	std::vector<int> parent(n), labels(n);
	std::iota(labels.begin(), labels.end(), 0);
	for (int v = 1; v < n; ++v)
		parent[v] = shape == 0 ? v - 1 : shape == 1 ? 0 : shape == 2 ? (v - 1) / 2 :
			shape == 3 ? (v < n / 2 ? v - 1 : std::max(0, n / 2 - 1)) : int(rng() % v);
	for (int i = n - 1; i > 0; --i) std::swap(labels[i], labels[rng() % (i + 1)]);
	if (n) input.root = labels[0];
	for (int v = 1; v < n; ++v) {
		input.edges.emplace_back(labels[parent[v]], labels[v]);
		if (rng() & 1) std::swap(input.edges.back().first, input.edges.back().second);
	}
	for (int i = int(input.edges.size()) - 1; i > 0; --i) std::swap(input.edges[i], input.edges[rng() % (i + 1)]);
	if (matrix) {
		std::mt19937 values(seed ^ 0x9e3779b9U);
		input.matrices.resize(n);
		for (auto &a : input.matrices) for (auto &x : a) x = 1 + values() % 1000000006U;
	}
	return input;
}
std::vector<unsigned char> header(const char *magic, int cell, std::uint32_t seed, const Input& in, bool matrix) {
	std::vector<unsigned char> bytes(magic, magic + 8);
	for (auto x : {std::uint32_t(cell), seed, std::uint32_t(in.n), std::uint32_t(in.edges.size()), std::uint32_t(in.root), std::uint32_t(matrix)}) u32(bytes, x);
	return bytes;
}
std::vector<unsigned char> input_bytes(int cell, std::uint32_t seed, const Input& in, bool matrix) {
	auto bytes = header("RRIN0001", cell, seed, in, matrix);
	for (auto [u, v] : in.edges) u32(bytes, u), u32(bytes, v);
	for (const auto &a : in.matrices) for (auto x : a) u32(bytes, x);
	return bytes;
}
template<class T> std::vector<unsigned char> output_bytes(int cell, std::uint32_t seed, const Input& in, const std::vector<T>& values) {
	auto bytes = header("RROU0001", cell, seed, in, std::is_same_v<T, Matrix>);
	for (const auto &x : values) {
		if constexpr (std::is_same_v<T, PairData>) { u64(bytes, std::uint64_t(x.sum)); u32(bytes, x.count); }
		else for (auto v : x) u32(bytes, v);
	}
	return bytes;
}
void retain_file(const std::filesystem::path& path, const std::vector<unsigned char>& bytes) {
	if (std::filesystem::exists(path)) {
		std::ifstream in(path, std::ios::binary);
		require(bool(in), "fixture_io", "existing fixture cannot be opened");
		std::vector<unsigned char> existing((std::istreambuf_iterator<char>(in)), {});
		require(existing == bytes, "fixture_mismatch", "existing canonical fixture bytes differ");
	} else {
		std::ofstream out(path, std::ios::binary | std::ios::trunc);
		require(bool(out), "fixture_io", "fixture cannot be created");
		out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		out.close();
		require(bool(out), "fixture_io", "fixture write failed");
	}
}
Matrix identity() { Matrix a{}; for (int i = 0; i < 4; ++i) a[5 * i] = 1; return a; }
Matrix oracle_product(const Matrix& a, const Matrix& b) {
	Matrix c{};
	for (int i = 0; i < 4; ++i) for (int k = 0; k < 4; ++k) for (int j = 0; j < 4; ++j) {
		std::uint64_t product = std::uint64_t(a[4 * i + k]) * std::uint64_t(b[4 * k + j]);
		c[4 * i + j] = (std::uint64_t(c[4 * i + j]) + product % 1000000007) % 1000000007;
	}
	return c;
}
template<class T> Result invoke(const Input& input) {
	if constexpr (std::is_same_v<T, PairData>) return run_pair(input);
	else return run_matrix(input);
}
template<class T> std::vector<T> snapshot(const Result& result) {
	if constexpr (std::is_same_v<T, PairData>) return snapshot_pair(result);
	else return snapshot_matrix(result);
}
template<class T> std::vector<T> naive(const Input& in) {
	std::vector<std::vector<std::pair<int, int>>> graph(in.n);
	for (int id = 0; id < int(in.edges.size()); ++id) {
		auto [u, v] = in.edges[id];
		graph[u].emplace_back(v, id), graph[v].emplace_back(u, id);
	}
	auto merge = [](const T& a, const T& b) -> T {
		if constexpr (std::is_same_v<T, PairData>) return {a.sum + b.sum, a.count + b.count};
		else return oracle_product(a, b);
	};
	auto vertex = [&](const T& x, int v) -> T {
		if constexpr (std::is_same_v<T, PairData>) { (void)v; return {x.sum, x.count + 1}; }
		else return oracle_product(in.matrices[v], x);
	};
	auto lift = [](const T& x) -> T {
		if constexpr (std::is_same_v<T, PairData>) return {x.sum + x.count, x.count};
		else return x;
	};
	auto dfs = [&](auto&& self, int v, int banned) -> T {
		T acc{};
		if constexpr (std::is_same_v<T, Matrix>) acc = identity();
		for (auto [to, id] : graph[v]) if (id != banned) acc = merge(acc, lift(self(self, to, id)));
		return vertex(acc, v);
	};
	std::vector<T> result;
	for (int v = 0; v < in.n; ++v) result.push_back(dfs(dfs, v, -1));
	for (int id = 0; id < int(in.edges.size()); ++id) {
		auto [u, v] = in.edges[id];
		result.push_back(lift(dfs(dfs, u, id)));
		result.push_back(lift(dfs(dfs, v, id)));
	}
	return result;
}
template<class T> void check_small(int shape, std::uint32_t seed) {
	auto in = generate(8, shape, seed, std::is_same_v<T, Matrix>);
	if constexpr (std::is_same_v<T, Matrix>) {
		const auto &a = in.matrices[0], &b = in.matrices[1], &c = in.matrices[2];
		require(multiply_matrices(a, b) == oracle_product(a, b), "matrix_oracle", "widened products differ from termwise oracle");
		require(multiply_matrices(identity(), a) == a && multiply_matrices(a, identity()) == a, "matrix_identity", "matrix identity failed");
		require(multiply_matrices(multiply_matrices(a, b), c) == multiply_matrices(a, multiply_matrices(b, c)), "matrix_associativity", "matrix associativity failed");
		require(multiply_matrices(a, b) != multiply_matrices(b, a), "matrix_noncommutative", "fixed matrices unexpectedly commute");
		Matrix maximum; maximum.fill(1000000006);
		require(multiply_matrices(maximum, maximum) == oracle_product(maximum, maximum), "matrix_widening", "maximum-entry products overflowed");
	}
	for (int forest = 0; forest < 2; ++forest) {
		if (forest) in.edges.erase(in.edges.begin() + 3);
		auto expected = naive<T>(in);
		for (int root = 0; root < in.n; ++root) {
			in.root = root;
			require(snapshot<T>(invoke<T>(in)) == expected, "small_oracle", "small tree/forest all-root full output differs");
		}
	}
	for (int n : {0, 1}) {
		auto tiny = generate(n, shape, seed, std::is_same_v<T, Matrix>);
		require(snapshot<T>(invoke<T>(tiny)) == naive<T>(tiny), "small_oracle", "empty/singleton full output differs");
	}
}
template<class T> struct Dataset {
	int cell;
	std::uint32_t seed;
	Spec info;
	Input input;
	std::vector<T> expected;
	std::string input_hash, output_hash, input_file, output_file;
	std::uint64_t saved_merges;
	std::uint64_t storage_per_result() const { return 256 + 2 * (8ULL * input.edges.size() + (input.n + 2 * input.edges.size()) * sizeof(T)); }
};
template<class T> Dataset<T> prepare(int cell, std::uint32_t seed, const std::filesystem::path& root) {
	auto s = spec(cell);
	check_small<T>(s.shape, seed);
	Dataset<T> d{cell, seed, s, generate(s.n, s.shape, seed, s.matrix), {}, {}, {}, {}, {}, 0};
	d.expected = snapshot<T>(invoke<T>(d.input));
	std::vector<int> degrees(d.input.n);
	for (auto [u, v] : d.input.edges) ++degrees[u], ++degrees[v];
	for (int degree : degrees) d.saved_merges += degree > 1;
	d.saved_merges -= degrees[d.input.root] > 1;
	if (s.shape == 0) require(d.saved_merges == std::uint64_t(s.n - 2), "merge_count", "endpoint path saving differs");
	if (s.shape == 1) require(d.saved_merges == 0, "merge_count", "center star must save zero merges");
	auto in = input_bytes(cell, seed, d.input, s.matrix), out = output_bytes(cell, seed, d.input, d.expected);
	d.input_hash = hex(hash_bytes(in)); d.output_hash = hex(hash_bytes(out));
	std::ostringstream name; name << "cell-" << std::setw(2) << std::setfill('0') << cell << "-seed-" << seed;
	d.input_file = (root / (name.str() + ".input.bin")).string();
	d.output_file = (root / (name.str() + ".output.bin")).string();
	retain_file(d.input_file, in); retain_file(d.output_file, out);
	require(rss() <= max_rss, "rss_cap", "worker peak RSS exceeds 1024 MiB during prepare");
	return d;
}
template<class T> void print_prepare(const Dataset<T>& d) {
	std::cout << "{\"ok\":true,\"cell\":" << d.cell << ",\"seed\":" << d.seed << ",\"type\":" << quoted(d.info.matrix ? "matrix" : "pair")
		<< ",\"shape\":" << d.info.shape << ",\"n\":" << d.input.n << ",\"m\":" << d.input.edges.size() << ",\"root\":" << d.input.root
		<< ",\"output_length\":" << d.expected.size() << ",\"scalar_count\":" << d.expected.size() * (d.info.matrix ? 16 : 2)
		<< ",\"saved_merges\":" << d.saved_merges << ",\"input_hash\":" << quoted(d.input_hash) << ",\"output_hash\":" << quoted(d.output_hash)
		<< ",\"storage_per_result\":" << d.storage_per_result() << ",\"input_file\":" << quoted(d.input_file) << ",\"output_file\":" << quoted(d.output_file)
		<< ",\"clock_reads\":" << clock_reads << ",\"small_oracle_passed\":true,\"peak_rss_bytes\":" << rss() << '}' << std::endl;
}
struct Mode { bool enabled = false, synthetic = false; std::uint64_t elapsed = 0; std::string fault; };
template<class T> void batch(const Dataset<T>& d, std::uint64_t repetitions, bool timed, const Mode& mode) {
	require(!timed || mode.enabled || mode.synthetic, "timing_disabled", "real timing requires explicit --enable-timing");
	require(repetitions > 0 && repetitions <= max_repetitions, "repetition_cap", "repetitions must be 1 through 128");
	require(repetitions <= max_storage / d.storage_per_result(), "storage_cap", "retained tree estimate exceeds 512 MiB");
	std::uint64_t elapsed = 0, storage = repetitions * d.storage_per_result(), peak = 0, verified = 0, all_hash = 14695981039346656037ULL;
	auto post_check = [&](bool ok, const char *code, const char *message) {
		if (!ok) {
			Failure f(code, message);
			f.observations = {{"cell", std::uint64_t(d.cell)}, {"seed", d.seed}, {"repetitions", repetitions}, {"storage_bytes", storage}, {"full_outputs_verified", verified}};
			if (timed) f.observations["elapsed_ns"] = elapsed;
			if (peak) f.observations["peak_rss_bytes"] = peak;
			throw f;
		}
	};
	{
		std::vector<Result> results(repetitions);
		std::chrono::steady_clock::time_point start;
		if (timed && !mode.synthetic) { ++clock_reads; start = std::chrono::steady_clock::now(); }
		for (std::uint64_t i = 0; i < repetitions; ++i) results[i] = invoke<T>(d.input);
		if (timed && !mode.synthetic) { ++clock_reads; elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count(); }
		else if (timed) elapsed = mode.elapsed;
		for (const auto& result : results) {
			auto values = snapshot<T>(result);
			if (mode.fault == "bad_result" && !verified) {
				if constexpr (std::is_same_v<T, PairData>) ++values[0].sum;
				else values[0][0] ^= 1;
			}
			post_check(values == d.expected, "result_mismatch", "post-timer complete logical output comparison failed");
			++verified;
			auto bytes = output_bytes(d.cell, d.seed, d.input, values);
			post_check(hex(hash_bytes(bytes)) == d.output_hash, "result_mismatch", "canonical output hash differs after logical comparison");
			for (auto b : bytes) all_hash = (all_hash ^ b) * 1099511628211ULL;
		}
		if (mode.fault == "storage") ++storage;
		peak = rss();
		if (mode.fault == "rss") peak = max_rss + 1;
		post_check(storage == repetitions * d.storage_per_result() && storage <= max_storage, "storage_cap", "retained tree storage estimate mismatch or over cap");
		post_check(peak <= max_rss, "rss_cap", "worker peak RSS exceeds 1024 MiB");
		post_check(!timed || elapsed <= max_elapsed, "batch_duration_cap", "batch exceeds 2 seconds");
	}
	std::cout << "{\"ok\":true,\"cell\":" << d.cell << ",\"seed\":" << d.seed << ",\"repetitions\":" << repetitions << ",\"elapsed_ns\":";
	if (timed) std::cout << elapsed; else std::cout << "null";
	std::cout << ",\"synthetic_clock\":" << (mode.synthetic ? "true" : "false") << ",\"storage_bytes\":" << storage << ",\"peak_rss_bytes\":" << peak
		<< ",\"output_hash\":" << quoted(d.output_hash) << ",\"batch_hash\":" << quoted(hex(all_hash)) << ",\"full_outputs_verified\":" << verified
		<< ",\"logical_values_verified\":" << verified * d.expected.size() << ",\"clock_reads\":" << clock_reads << '}' << std::endl;
}
std::uint64_t number(const std::string& s) {
	require(!s.empty() && s.find_first_not_of("0123456789") == std::string::npos, "malformed_command", "expected nonnegative decimal integer");
	std::size_t used = 0;
	auto result = std::stoull(s, &used);
	require(used == s.size(), "malformed_command", "invalid integer suffix");
	return result;
}
}
int main(int argc, char **argv) {
	static_assert(sizeof(PairData) == 16 && sizeof(Matrix) == 64);
	Mode mode;
	std::filesystem::path root;
	try {
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "--enable-timing") mode.enabled = true;
			else if (arg == "--synthetic-clock-ns" && i + 1 < argc) mode.synthetic = true, mode.elapsed = number(argv[++i]);
			else if (arg == "--fault" && i + 1 < argc) mode.fault = argv[++i];
			else if (arg == "--fixtures-root" && i + 1 < argc) root = argv[++i];
			else throw Failure("invalid_option", "unrecognized worker option");
		}
		require(!root.empty() && root.is_absolute(), "invalid_option", "absolute --fixtures-root is required");
		require(!(mode.enabled && mode.synthetic), "invalid_option", "real and synthetic clocks are mutually exclusive");
		require(mode.fault.empty() || mode.synthetic, "invalid_option", "fault injection requires synthetic clock");
		require(mode.fault.empty() || mode.fault == "bad_result" || mode.fault == "storage" || mode.fault == "rss", "invalid_option", "unknown fault");
		std::filesystem::create_directories(root);
		root = std::filesystem::canonical(root);
	} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 2; }
	using Any = std::variant<Dataset<PairData>, Dataset<Matrix>>;
	std::map<std::pair<int, std::uint32_t>, Any> data;
	for (std::string line; std::getline(std::cin, line);) {
		try {
			std::istringstream input(line);
			std::vector<std::string> tokens;
			for (std::string x; input >> x;) tokens.push_back(x);
			require(!tokens.empty(), "malformed_command", "empty command");
			if (tokens[0] == "quit") {
				require(tokens.size() == 1, "malformed_command", "quit takes no arguments");
				std::cout << "{\"ok\":true,\"quit\":true}" << std::endl; return 0;
			}
			bool preparing = tokens[0] == "prepare", timed = tokens[0] == "batch", untimed = tokens[0] == "untimed_batch";
			require(preparing || timed || untimed, "malformed_command", "unknown command");
			require(tokens.size() == (preparing ? 3 : 4), "malformed_command", "wrong argument count");
			auto c = number(tokens[1]), s = number(tokens[2]);
			require(c >= 1 && c <= 10, "invalid_cell", "cell must be 1 through 10");
			require(s >= 857123 && s <= 857125, "invalid_seed", "seed must match fixed protocol");
			int cell = int(c); auto seed = std::uint32_t(s); auto key = std::make_pair(cell, seed);
			if (preparing) {
				if (!data.contains(key)) {
					if (cell <= 5) data.emplace(key, prepare<PairData>(cell, seed, root));
					else data.emplace(key, prepare<Matrix>(cell, seed, root));
				}
				std::visit([](const auto& d) { print_prepare(d); }, data.at(key));
			} else {
				require(data.contains(key), "not_prepared", "prepare first");
				auto repetitions = number(tokens[3]);
				std::visit([&](const auto& d) { batch(d, repetitions, timed, mode); }, data.at(key));
			}
		} catch (const Failure& e) {
			std::cout << "{\"ok\":false,\"error_code\":" << quoted(e.code) << ",\"error\":" << quoted(e.what()) << ",\"clock_reads\":" << clock_reads;
			for (const auto& [key, value] : e.observations) std::cout << ',' << quoted(key) << ':' << value;
			std::cout << '}' << std::endl;
		} catch (const std::exception& e) {
			std::cout << "{\"ok\":false,\"error_code\":\"exception\",\"error\":" << quoted(e.what()) << ",\"clock_reads\":" << clock_reads << '}' << std::endl;
		}
	}
}
