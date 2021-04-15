//! \file kerneltest/kerneltest.hpp The master *latest version* KernelTest include file. All KernelTest consuming libraries should include this header only.
#include "version.hpp"

#if defined(_MSC_VER) && !defined(__clang__) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#define KERNELTEST_HEADERS_PATH2 KERNELTEST_VERSION_GLUE(v, KERNELTEST_HEADERS_VERSION, /kerneltest.hpp)
#else
#define KERNELTEST_HEADERS_PATH2 KERNELTEST_VERSION_GLUE(v, KERNELTEST_HEADERS_VERSION,)/kerneltest.hpp
#endif

#if 0 // cmake's Makefiles and Ninja generators won't pick up dependent headers without this
#include "v1.0/kerneltest.hpp"
#endif

#define KERNELTEST_HEADERS_PATH4(a) #a
#define KERNELTEST_HEADERS_PATH3(a) KERNELTEST_HEADERS_PATH4(a)
//! \brief The KernelTest headers path generated by the preprocessor from the version
#define KERNELTEST_HEADERS_PATH KERNELTEST_HEADERS_PATH3(KERNELTEST_HEADERS_PATH2)
#include KERNELTEST_HEADERS_PATH
#undef KERNELTEST_HEADERS_PATH
#undef KERNELTEST_HEADERS_PATH2
#undef KERNELTEST_HEADERS_PATH3
#undef KERNELTEST_HEADERS_PATH4
