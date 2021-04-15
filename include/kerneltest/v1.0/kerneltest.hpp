//! \file v1.0/kerneltest.hpp The master *versioned* KernelTest include file. All version specific KernelTest consuming libraries should include this header only.

#undef KERNELTEST_VERSION_MAJOR
#undef KERNELTEST_VERSION_MINOR
#undef KERNELTEST_VERSION_PATCH
// Remove any previously defined versioning
#undef KERNELTEST_VERSION_REVISION
#undef KERNELTEST_VERSION_GLUE2
#undef KERNELTEST_VERSION_GLUE
#undef KERNELTEST_HEADERS_VERSION
#undef KERNELTEST_NAMESPACE_VERSION
#undef KERNELTEST_MODULE_NAME

#define KERNELTEST_VERSION_GLUE2(a, b, c) a##b##c
#define KERNELTEST_VERSION_GLUE(a, b, c) KERNELTEST_VERSION_GLUE2(a, b, c)

// Hard coded as this is a specific version
#define KERNELTEST_VERSION_MAJOR 1
#define KERNELTEST_VERSION_MINOR 0
#define KERNELTEST_VERSION_PATCH 0
#define KERNELTEST_VERSION_REVISION 0
//! \brief The namespace boost::kerneltest::v ## KERNELTEST_NAMESPACE_VERSION
#define KERNELTEST_NAMESPACE_VERSION KERNELTEST_VERSION_GLUE(KERNELTEST_VERSION_MAJOR, _, KERNELTEST_VERSION_MINOR)

#if defined(__cpp_modules) || defined(DOXYGEN_SHOULD_SKIP_THIS)
#if defined(_MSC_VER) && !defined(__clang__)
//! \brief The KernelTest C++ module name
#define KERNELTEST_MODULE_NAME KERNELTEST_VERSION_GLUE(kerneltest_v, KERNELTEST_NAMESPACE_VERSION, )
#else
//! \brief The KernelTest C++ module name
#define KERNELTEST_MODULE_NAME KERNELTEST_VERSION_GLUE(kerneltest_v, KERNELTEST_NAMESPACE_VERSION, )
#endif
#endif

// If C++ Modules are on and we are not compiling the library,
// we are either generating the interface or importing
#if !defined(__cpp_modules) || defined(GENERATING_KERNELTEST_MODULE_INTERFACE) || KERNELTEST_DISABLE_CXX_MODULES
// C++ Modules not on, therefore include as usual
#define KERNELTEST_INCLUDE_ALL
#elif defined(GENERATING_KERNELTEST_MODULE_INTERFACE)
// We are generating this module's interface
#define QUICKCPPLIB_HEADERS_ONLY 0
#define KERNELTEST_HEADERS_ONLY 0
#define KERNELTEST_INCLUDE_ALL
#elif defined(KERNELTEST_SOURCE)
// We are implementing this module
#define KERNELTEST_INCLUDE_ALL
#else
// We are importing this module
import KERNELTEST_MODULE_NAME;
#undef KERNELTEST_INCLUDE_ALL
#endif

#ifdef KERNELTEST_INCLUDE_ALL

#include "config.hpp"

#include "test_kernel.hpp"

#include "permute_parameters.hpp"
#include "child_process.hpp"

#include "hooks/custom.hpp"
#include "hooks/filesystem_workspace.hpp"

#endif
