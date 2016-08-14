//! \file v1.0/kerneltest.hpp The master *versioned* KernelTest include file. All version specific KernelTest consuming libraries should include this header only.

#undef BOOST_KERNELTEST_VERSION_MAJOR
#undef BOOST_KERNELTEST_VERSION_MINOR
#undef BOOST_KERNELTEST_VERSION_PATCH
// Remove any previously defined versioning
#undef BOOST_KERNELTEST_VERSION_REVISION
#undef BOOST_KERNELTEST_VERSION_GLUE2
#undef BOOST_KERNELTEST_VERSION_GLUE
#undef BOOST_KERNELTEST_HEADERS_VERSION
#undef BOOST_KERNELTEST_NAMESPACE_VERSION
#undef BOOST_KERNELTEST_MODULE_NAME

#define BOOST_KERNELTEST_VERSION_GLUE2(a, b, c) a ## b ## c
#define BOOST_KERNELTEST_VERSION_GLUE(a, b, c)  BOOST_KERNELTEST_VERSION_GLUE2(a, b, c)

// Hard coded as this is a specific version
#define BOOST_KERNELTEST_VERSION_MAJOR    1
#define BOOST_KERNELTEST_VERSION_MINOR    0
#define BOOST_KERNELTEST_VERSION_PATCH    0
#define BOOST_KERNELTEST_VERSION_REVISION 0
//! \brief The namespace boost::kerneltest::v ## BOOST_KERNELTEST_NAMESPACE_VERSION
#define BOOST_KERNELTEST_NAMESPACE_VERSION   BOOST_KERNELTEST_VERSION_GLUE(BOOST_KERNELTEST_VERSION_MAJOR, _, BOOST_KERNELTEST_VERSION_MINOR)

#if defined(__cpp_modules) || defined(DOXYGEN_SHOULD_SKIP_THIS)
#if defined(_MSC_VER) && !defined(__clang__)
//! \brief The KernelTest C++ module name
#define BOOST_KERNELTEST_MODULE_NAME BOOST_KERNELTEST_VERSION_GLUE(afio_v, BOOST_KERNELTEST_NAMESPACE_VERSION,)
#else
//! \brief The KernelTest C++ module name
#define BOOST_KERNELTEST_MODULE_NAME BOOST_KERNELTEST_VERSION_GLUE(afio_v, BOOST_KERNELTEST_NAMESPACE_VERSION,)
#endif
#endif

// If C++ Modules are on and we are not compiling the library,
// we are either generating the interface or importing
#if defined(__cpp_modules)
#if defined(GENERATING_CXX_MODULE_INTERFACE)
// We are generating this module's interface
#define BOOSTLITE_HEADERS_ONLY 0
#define BOOST_KERNELTEST_HEADERS_ONLY 0
#define BOOST_KERNELTEST_INCLUDE_ALL
#elif defined(BOOST_KERNELTEST_SOURCE)
// We are implementing this module
#define BOOST_KERNELTEST_INCLUDE_ALL
#else
// We are importing this module
import BOOST_KERNELTEST_MODULE_NAME;
#undef BOOST_KERNELTEST_INCLUDE_ALL
#endif
#else
// C++ Modules not on, therefore include as usual
#define BOOST_KERNELTEST_INCLUDE_ALL
#endif

#ifdef BOOST_KERNELTEST_INCLUDE_ALL

#include "config.hpp"

#include "permute_parameters.hpp"
#include "test_kernel.hpp"

#include "hooks/filesystem_workspace.hpp"

#endif
