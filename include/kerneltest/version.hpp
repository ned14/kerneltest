//! \file version.hpp Controls the version of KernelTest for cmake, shared library and C++ namespace mangling
#undef KERNELTEST_VERSION_MAJOR
#undef KERNELTEST_VERSION_MINOR
#undef KERNELTEST_VERSION_PATCH
#undef KERNELTEST_VERSION_REVISION
#undef KERNELTEST_VERSION_GLUE2
#undef KERNELTEST_VERSION_GLUE
#undef KERNELTEST_HEADERS_VERSION
#undef KERNELTEST_NAMESPACE_VERSION

//! \brief Major version for cmake and DLL version stamping \ingroup config
#define KERNELTEST_VERSION_MAJOR    1
//! \brief Minor version for cmake and DLL version stamping \ingroup config
#define KERNELTEST_VERSION_MINOR    0
//! \brief Patch version for cmake and DLL version stamping \ingroup config
#define KERNELTEST_VERSION_PATCH    0
//! \brief Revision version for cmake and DLL version stamping \ingroup config
#define KERNELTEST_VERSION_REVISION 0

//! \brief Defined between stable releases of KernelTest. It means the inline namespace
//! will be permuted per-commit to ensure ABI uniqueness. \ingroup config
#define KERNELTEST_UNSTABLE_VERSION

#define KERNELTEST_VERSION_GLUE2(a, b, c) a ## b ## c
#define KERNELTEST_VERSION_GLUE(a, b, c)  KERNELTEST_VERSION_GLUE2(a, b, c)
#define KERNELTEST_NAMESPACE_VERSION   KERNELTEST_VERSION_GLUE(KERNELTEST_VERSION_MAJOR, _, KERNELTEST_VERSION_MINOR)

#if defined(_MSC_VER) && !defined(__clang__)
#define KERNELTEST_HEADERS_VERSION     KERNELTEST_VERSION_GLUE(KERNELTEST_VERSION_MAJOR, ., KERNELTEST_VERSION_MINOR)
#elif !__PCPP_ALWAYS_FALSE__
#define KERNELTEST_HEADERS_VERSION     KERNELTEST_VERSION_MAJOR.KERNELTEST_VERSION_MINOR
#endif
//! \brief The namespace boost::kerneltest::v ## KERNELTEST_NAMESPACE_VERSION
#define KERNELTEST_NAMESPACE_VERSION KERNELTEST_VERSION_GLUE(KERNELTEST_VERSION_MAJOR, _, KERNELTEST_VERSION_MINOR)
