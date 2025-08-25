#pragma once

#ifdef _WIN32
    #include <malloc.h>
#else
    #include <cstdlib>
#endif

namespace platform {

    /**
     * Cross-platform aligned memory allocation
     * @param size Size of memory to allocate
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to aligned memory or nullptr on failure
     */
    inline void* aligned_alloc(size_t size, size_t alignment) {
#ifdef _WIN32
        return _aligned_malloc(size, alignment);
#else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) == 0) {
            return ptr;
        }
        return nullptr;
#endif
    }

    /**
     * Free memory allocated by aligned_alloc
     * @param ptr Pointer to memory allocated by aligned_alloc
     */
    inline void aligned_free(void* ptr) {
        if (ptr) {
#ifdef _WIN32
            _aligned_free(ptr);
#else
            free(ptr);
#endif
        }
    }

}
