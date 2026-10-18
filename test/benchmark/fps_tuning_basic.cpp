#ifndef FPS_TUNING_HEADER
#define FPS_TUNING_HEADER "fps/fps.hpp"
#endif
#include FPS_TUNING_HEADER
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	std::string op = argv[1];
	const int n = std::atoi(argv[2]);
	std::mt19937 rng(731);
	fps a(n), b(n), result;
	for (int i = 0; i < n; ++i) a[i] = rng(), b[i] = rng();
	if (const auto colon = op.find(':'); colon != std::string::npos) {
		const int terms = std::atoi(op.c_str() + colon + 1);
		const auto next = op.find(':', colon + 1);
		const int stride = next == std::string::npos ? 0 : std::atoi(op.c_str() + next + 1);
		op.resize(colon);
		fps &sparse = op == "div_series" ? b : a;
		std::fill(sparse.begin(), sparse.end(), fps_mint(0));
		sparse[0] = 1;
		for (int i = 0; i < terms; ++i) {
			const long long at = stride ? (long long)stride * (i + 1) : 1 + 7LL * i;
			if (at >= n) break;
			sparse[at] = rng();
		}
	}
	if (op == "inv" || op == "log") a[0] = 1;
	if (op == "exp") a[0] = 0;
	if (op == "sqrt") a[0] = fps_mint(17) * 17;
	if (op == "div_series") b[0] = 1;
	auto start = std::chrono::steady_clock::now();
	if (op == "inv") result = a.inv();
	else if (op == "log") result = a.log();
	else if (op == "exp") result = a.exp();
	else if (op == "sqrt") result = a.sqrt();
	else if (op == "pow") result = a.pow(123456789);
	else if (op == "square") result = a * a;
	else if (op == "multiply") result = a * b;
	else if (op == "div_series") result = a.div_series(b);
	else if (op == "divmod" || op == "divmod_balanced") {
		b.resize(op == "divmod_balanced" ? (n + 1) / 2 : n / 2);
		auto qr = a.divmod(b);
		result = std::move(qr.first);
		result.insert(result.end(), qr.second.begin(), qr.second.end());
	} else return 1;
	auto stop = std::chrono::steady_clock::now();
	unsigned long long checksum = 0;
	for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	std::printf("%.9f %llu\n", std::chrono::duration<double>(stop - start).count(), checksum);
}
