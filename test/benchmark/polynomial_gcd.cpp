#include "fps/polynomial_gcd.hpp"
#include "components/fast_io.hpp"
#include <string_view>

int main(int argc, char** argv) {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	fps a(n), b(m);
	for (auto& x : a) { unsigned v = 0; input.read(v); x = v; }
	for (auto& x : b) { unsigned v = 0; input.read(v); x = v; }
	fps result;
	if (argc > 1 && std::string_view(argv[1]) == "euclid") {
		fps_detail::Operations<mint>::trim(a), fps_detail::Operations<mint>::trim(b);
		while (!b.empty()) { auto r = a.divmod(b).second; a = std::move(b), b = std::move(r); }
		if (!a.empty()) a /= a.back();
		result = std::move(a);
	} else result = polynomial_gcd(std::move(a), std::move(b));
	output.println(result.size());
	for (const auto& x : result) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}
