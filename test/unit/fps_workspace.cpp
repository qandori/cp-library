#include "../../components/fps_workspace.hpp"
#include "../../components/fps_operations.hpp"
#include <cstdint>
#include <cstdlib>
#include <utility>

using Operations = fps_detail::Operations<fps_mint>;
[[maybe_unused]] constexpr void (*original_join_windows)(unsigned int *, int, int) = &Operations::join_windows;

static void require(bool value) {
	if (!value) std::abort();
}

int main() {
	for (std::size_t n : {std::size_t(0), std::size_t(1), std::size_t(31), std::size_t(4097),
						 (std::size_t(1) << 19) - 1, std::size_t(1) << 19,
						 (std::size_t(1) << 19) + 1, (std::size_t(1) << 20) + 17}) {
		auto a = fps_detail::allocate_fps_words(n);
		require(reinterpret_cast<std::uintptr_t>(a.get()) % 64 == 0);
		for (std::size_t i = 0; i < n; ++i) a[i] = unsigned(i) * 731u + 17;
		auto *original = a.get();
		auto b = std::move(a);
		require(!a && b.get() == original);
		a = fps_detail::allocate_fps_words(16);
		a = std::move(b);
		require(!b && a.get() == original);
		for (std::size_t i = 0; i < n; ++i) require(a[i] == unsigned(i) * 731u + 17);
	}
	for (int length : {2, 8, 32, 128}) {
		for (int count : {1, 2, 7, 17}) {
			const int pitch = length + 16;
			auto a = fps_detail::allocate_fps_words(count * pitch);
			std::fill_n(a.get(), count * pitch, 0xffffffffu);
			for (int i = 0; i < count; ++i)
				for (int j = 0; j < length; ++j) a[i * pitch + j] = (i + j) & 1 ? fps_detail::NTT::twice - 1 : 0;
			Operations::join_windows_strided(a.get(), length, count, pitch);
			for (int i = 0; i < count; ++i) {
				for (int j = 0; j < length; ++j) {
					unsigned long long x = (i + j) & 1 ? fps_detail::NTT::twice - 1 : 0;
					unsigned long long y = (i + j) & 1 ? 0 : fps_detail::NTT::twice - 1;
					if (i) x = (x + (j < length / 2 ? y : fps_detail::NTT::twice - y)) % fps_detail::NTT::twice;
					require(a[i * pitch + j] == x);
				}
				for (int j = length; j < pitch; ++j) require(a[i * pitch + j] == 0xffffffffu);
			}
		}
	}
}
