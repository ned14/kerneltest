/* KernelTest config
(C) 2016 Niall Douglas http://www.nedproductions.biz/
File Created: Apr 2016
*/

#include "../boost-lite/include/import.h"

#ifndef __cpp_exceptions
#error Boost.AFIO needs C++ exceptions to be turned on
#endif
#ifndef __cpp_alias_templates
#error Boost.AFIO needs template alias support in the compiler
#endif
#ifndef __cpp_variadic_templates
#error Boost.AFIO needs variadic template support in the compiler
#endif
#ifndef __cpp_noexcept
#error Boost.AFIO needs noexcept support in the compiler
#endif
#ifndef __cpp_constexpr
#error Boost.AFIO needs constexpr (C++ 11) support in the compiler
#endif

#ifndef BOOST_KERNELTEST_LATEST_VERSION
#define BOOST_KERNELTEST_LATEST_VERSION 1
#endif

// Default to the C++ 11 STL for atomic, chrono, mutex and thread except on Mingw32
#if(defined(BOOST_KERNELTEST_USE_BOOST_THREAD) && BOOST_KERNELTEST_USE_BOOST_THREAD) || (defined(__MINGW32__) && !defined(__MINGW64__) && !defined(__MINGW64_VERSION_MAJOR))
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 1
#error You must configure Boost.Outcome and Boost.AFIO to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 1
#define BOOST_KERNELTEST_V2_STL11_IMPL boost
#ifndef BOOST_THREAD_VERSION
#define BOOST_THREAD_VERSION 3
#endif
#if BOOST_THREAD_VERSION < 3
#error Boost.AFIO requires that Boost.Thread be configured to v3 or later
#endif
#else
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 0
#error You must configure Boost.Outcome and Boost.AFIO to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 0
#define BOOST_KERNELTEST_V1_STL11_IMPL std
#ifndef BOOST_KERNELTEST_USE_BOOST_THREAD
#define BOOST_KERNELTEST_USE_BOOST_THREAD 0
#endif
#endif
// Default to the C++ 11 STL if on MSVC (Dinkumware ships a copy), else Boost
#ifndef BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#if _MSC_VER >= 1900  // >= VS 14
#define BOOST_KERNELTEST_USE_BOOST_FILESYSTEM 0
#endif
#endif
#ifndef BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#define BOOST_KERNELTEST_USE_BOOST_FILESYSTEM 1
#endif
#if BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#define BOOST_KERNELTEST_V1_FILESYSTEM_IMPL boost
#define BOOST_KERNELTEST_USE_LEGACY_FILESYSTEM_SEMANTICS 1
#else
#define BOOST_KERNELTEST_V1_FILESYSTEM_IMPL std
#endif
#define BOOST_KERNELTEST_V1 (boost), (kerneltest), (BOOSTLITE_NAMESPACE_VERSION(v1, BOOST_KERNELTEST_V1_STL11_IMPL, BOOST_KERNELTEST_V1_FILESYSTEM_IMPL), inline)
#if DOXYGEN_SHOULD_SKIP_THIS
#define BOOST_KERNELTEST_V1_NAMESPACE boost::kerneltest::v1
#define BOOST_KERNELTEST_V1_NAMESPACE_BEGIN                                                                                                                                                                                                                                                                                    \
  namespace boost                                                                                                                                                                                                                                                                                                              \
  {                                                                                                                                                                                                                                                                                                                            \
    namespace kerneltest                                                                                                                                                                                                                                                                                                       \
    {                                                                                                                                                                                                                                                                                                                          \
      inline namespace v1                                                                                                                                                                                                                                                                                                      \
      {
#define BOOST_KERNELTEST_V1_NAMESPACE_END                                                                                                                                                                                                                                                                                      \
  }                                                                                                                                                                                                                                                                                                                            \
  }                                                                                                                                                                                                                                                                                                                            \
  }
#else
#define BOOST_KERNELTEST_V1_NAMESPACE BOOSTLITE_NAMESPACE(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_BEGIN BOOSTLITE_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_END BOOSTLITE_NAMESPACE_END(BOOST_KERNELTEST_V1)
#endif

// From automated matrix generator
#undef BOOST_KERNELTEST_NEED_DEFINE
#undef BOOST_KERNELTEST_NEED_DEFINE_DESCRIPTION
#if !BOOST_KERNELTEST_USE_BOOST_THREAD && !BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#ifndef BOOST_KERNELTEST_NEED_DEFINE_00
#define BOOST_KERNELTEST_NEED_DEFINE_DESCRIPTION "BOOST_KERNELTEST_USE_BOOST_THREAD=0 BOOST_KERNELTEST_USE_BOOST_FILESYSTEM=0"
#define BOOST_KERNELTEST_NEED_DEFINE_00
#define BOOST_KERNELTEST_NEED_DEFINE 1
#endif
#elif BOOST_KERNELTEST_USE_BOOST_THREAD && !BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#ifndef BOOST_KERNELTEST_NEED_DEFINE_10
#define BOOST_KERNELTEST_NEED_DEFINE_DESCRIPTION "BOOST_KERNELTEST_USE_BOOST_THREAD=1 BOOST_KERNELTEST_USE_BOOST_FILESYSTEM=0"
#define BOOST_KERNELTEST_NEED_DEFINE_10
#define BOOST_KERNELTEST_NEED_DEFINE 1
#endif
#elif !BOOST_KERNELTEST_USE_BOOST_THREAD && BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#ifndef BOOST_KERNELTEST_NEED_DEFINE_01
#define BOOST_KERNELTEST_NEED_DEFINE_DESCRIPTION "BOOST_KERNELTEST_USE_BOOST_THREAD=0 BOOST_KERNELTEST_USE_BOOST_FILESYSTEM=1"
#define BOOST_KERNELTEST_NEED_DEFINE_01
#define BOOST_KERNELTEST_NEED_DEFINE 1
#endif
#elif BOOST_KERNELTEST_USE_BOOST_THREAD && BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#ifndef BOOST_KERNELTEST_NEED_DEFINE_11
#define BOOST_KERNELTEST_NEED_DEFINE_DESCRIPTION "BOOST_KERNELTEST_USE_BOOST_THREAD=1 BOOST_KERNELTEST_USE_BOOST_FILESYSTEM=1"
#define BOOST_KERNELTEST_NEED_DEFINE_11
#define BOOST_KERNELTEST_NEED_DEFINE 1
#endif
#endif

#ifdef BOOST_KERNELTEST_NEED_DEFINE
#undef BOOST_KERNELTEST_KERNELTEST_H

#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR32_T  // missing VS14
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR16_T  // missing VS14
// Match Dinkumware's TR2 implementation
#define BOOST_STL1z_FILESYSTEM_MAP_NO_SYMLINK_OPTION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COPY_OPTION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_CHANGE_EXTENSION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WRECURSIVE_DIRECTORY_ITERATOR
#define BOOST_STL1z_FILESYSTEM_MAP_NO_EXTENSION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_TYPE_PRESENT
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_FILE_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_DIRECTORY_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_POSIX_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_LEXICOGRAPHICAL_COMPARE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WINDOWS_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_BASENAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COMPLETE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_IS_REGULAR
#define BOOST_STL1z_FILESYSTEM_MAP_NO_INITIAL_PATH
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PERMISSIONS_PRESENT
#define BOOST_STL1z_FILESYSTEM_MAP_NO_CODECVT_ERROR_CATEGORY
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WPATH
#define BOOST_STL1z_FILESYSTEM_MAP_NO_SYMBOLIC_LINK_EXISTS
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COPY_DIRECTORY
#define BOOST_STL1z_FILESYSTEM_MAP_NO_NATIVE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_UNIQUE_PATH

#include "../boost-lite/include/bind/stl11/std/atomic"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl11
{
  using namespace boost_lite::bind::std::atomic;
}
BOOST_KERNELTEST_V1_NAMESPACE_END
#if BOOST_OUTCOME_USE_BOOST_THREAD
#include "../boost-lite/include/bind/stl11/boost/chrono"
#include "../boost-lite/include/bind/stl11/boost/condition_variable"
#include "../boost-lite/include/bind/stl11/boost/future"
#include "../boost-lite/include/bind/stl11/boost/mutex"
#include "../boost-lite/include/bind/stl11/boost/ratio"
#include "../boost-lite/include/bind/stl11/boost/thread"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl11
{
  namespace chrono = boost_lite::bind::boost::chrono;
  using namespace boost_lite::bind::boost::condition_variable;
  using namespace boost_lite::bind::boost::future;
  using namespace boost_lite::bind::boost::mutex;
  using namespace boost_lite::bind::boost::ratio;
  using namespace boost_lite::bind::boost::thread;
}
#else
#include "../boost-lite/include/bind/stl11/std/chrono"
#include "../boost-lite/include/bind/stl11/std/condition_variable"
#include "../boost-lite/include/bind/stl11/std/future"
#include "../boost-lite/include/bind/stl11/std/mutex"
#include "../boost-lite/include/bind/stl11/std/ratio"
#include "../boost-lite/include/bind/stl11/std/thread"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl11
{
  namespace chrono = boost_lite::bind::std::chrono;
  using namespace boost_lite::bind::std::condition_variable;
  using namespace boost_lite::bind::std::future;
  using namespace boost_lite::bind::std::mutex;
  using namespace boost_lite::bind::std::ratio;
  using namespace boost_lite::bind::std::thread;
}
BOOST_KERNELTEST_V1_NAMESPACE_END
#endif
#if BOOST_AFIO_USE_BOOST_FILESYSTEM
#include "../boost-lite/include/bind/stl1z/boost/filesystem"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl1z
{
  namespace filesystem = boost_lite::bind::boost::filesystem;
  using fs_error_code = ::boost::error_code;
  using fs_system_error = ::boost::system_error;
}
BOOST_KERNELTEST_V1_NAMESPACE_END
#else
#include "../boost-lite/include/bind/stl1z/std/filesystem"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl1z
{
  namespace filesystem = boost_lite::bind::std::filesystem;
  using fs_error_code = ::std::error_code;
  using fs_system_error = ::std::system_error;
}
BOOST_KERNELTEST_V1_NAMESPACE_END
#endif


#include "../outcome/include/boost/outcome.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
// We are so heavily tied into Outcome we just import it wholesale into our namespace
using namespace BOOST_OUTCOME_V1_NAMESPACE;
// Force these to the same overloading precedence as if they were defined in the AFIO namespace
using BOOST_OUTCOME_V1_NAMESPACE::outcome;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_result;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_outcome;
BOOST_KERNELTEST_V1_NAMESPACE_END

// We need an aggregate initialisable collection of heterogeneous types
#if __cplusplus >= 20170000L || __GNUC__ >= 6
#include <tuple>
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
template <class... Types> using parameters = std::tuple<Types...>;
template <class T> using parameters_size = std::tuple_size<T>;
template <size_t N, class T> using parameters_element = std::tuple_element<N, T>;
BOOST_KERNELTEST_V1_NAMESPACE_END
#else
#include "../boost-lite/include/atuple.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
template <class... Types> using parameters = boost_lite::aggregate_tuple::tuple<Types...>;
template <class T> using parameters_size = boost_lite::aggregate_tuple::tuple_size<T>;
template <size_t N, class T> using parameters_element = boost_lite::aggregate_tuple::tuple_element<N, T>;
BOOST_KERNELTEST_V1_NAMESPACE_END
#endif

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

//! Many <filesystem> TS implementations are not implementing std::hash for filesystem::path
struct path_hasher
{
  size_t operator()(const stl1z::filesystem::path &p) const { return std::hash<stl1z::filesystem::path::string_type>()(p.native()); }
};

//! Keeps what the current test kernel is for the calling thread
static BOOSTLITE_THREAD_LOCAL struct current_test_kernel_t
{
  const char *category;
  const char *product;
  const char *test;
  const char *name;
  const char *description;
} current_test_kernel;

BOOST_KERNELTEST_V1_NAMESPACE_END

#endif  // need define
