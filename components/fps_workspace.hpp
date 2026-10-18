#ifndef CP_COMPONENTS_FPS_WORKSPACE_HPP
#define CP_COMPONENTS_FPS_WORKSPACE_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#if defined(__linux__)
#include <sys/mman.h>
#endif

namespace fps_detail {

struct FpsWordsDelete {
	void *region = nullptr;
	std::size_t bytes = 0;
	void operator()(unsigned int *p) const {
#if defined(__linux__)
		if (region) {
			munmap(region, bytes);
			return;
		}
#endif
		::operator delete[](p, std::align_val_t(64));
	}
};

using FpsWords = std::unique_ptr<unsigned int[], FpsWordsDelete>;

inline FpsWords allocate_fps_words(std::size_t count) {
#if defined(__linux__) && defined(MADV_HUGEPAGE)
	constexpr std::size_t page = 1 << 21;
	if (count >= page / sizeof(unsigned int) &&
		count <= (std::numeric_limits<std::size_t>::max() - 2 * page) / sizeof(unsigned int)) {
		const std::size_t bytes = (count * sizeof(unsigned int) + page - 1) / page * page;
		void *region = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (region != MAP_FAILED) {
			const auto address = (reinterpret_cast<std::uintptr_t>(region) + page - 1) & ~(page - 1);
			auto *p = ::new (reinterpret_cast<void *>(address)) unsigned int[count];
			madvise(p, bytes, MADV_HUGEPAGE);
			return FpsWords(p, FpsWordsDelete{region, bytes + page});
		}
	}
#endif
	return FpsWords(new (std::align_val_t(64)) unsigned int[count]);
}

}

#endif
