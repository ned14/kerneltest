/* KernelTest config
(C) 2016 Niall Douglas http://www.nedproductions.biz/
File Created: Apr 2016
*/

#define BOOST_KERNELTEST_CONFIGURED

#if !defined(BOOST_KERNELTEST_HEADERS_ONLY) && !defined(BOOST_ALL_DYN_LINK)
//! \brief Whether KernelTest is a headers only library. Defaults to 1 unless BOOST_ALL_DYN_LINK is defined. \ingroup config
#define BOOST_KERNELTEST_HEADERS_ONLY 1
#endif

#include "../boost-lite/include/cpp_feature.h"
#include "../boost-lite/include/import.h"

#ifndef __cpp_exceptions
#error Boost.KernelTest needs C++ exceptions to be turned on
#endif
#ifndef __cpp_alias_templates
#error Boost.KernelTest needs template alias support in the compiler
#endif
#ifndef __cpp_variadic_templates
#error Boost.KernelTest needs variadic template support in the compiler
#endif
#ifndef __cpp_constexpr
#error Boost.KernelTest needs constexpr (C++ 11) support in the compiler
#endif

#ifndef BOOST_KERNELTEST_LATEST_VERSION
#define BOOST_KERNELTEST_LATEST_VERSION 1
#endif

// Default to the C++ 11 STL for atomic, chrono, mutex and thread except on Mingw32
#if(defined(BOOST_KERNELTEST_USE_BOOST_THREAD) && BOOST_KERNELTEST_USE_BOOST_THREAD) || (defined(__MINGW32__) && !defined(__MINGW64__) && !defined(__MINGW64_VERSION_MAJOR))
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 1
#error You must configure Boost.Outcome and Boost.KernelTest to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 1
#define BOOST_KERNELTEST_V2_STL11_IMPL boost
#ifndef BOOST_THREAD_VERSION
#define BOOST_THREAD_VERSION 3
#endif
#if BOOST_THREAD_VERSION < 3
#error Boost.KernelTest requires that Boost.Thread be configured to v3 or later
#endif
#else
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 0
#error You must configure Boost.Outcome and Boost.KernelTest to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 0
#define BOOST_KERNELTEST_V1_STL11_IMPL std
#ifndef BOOST_KERNELTEST_USE_BOOST_THREAD
#define BOOST_KERNELTEST_USE_BOOST_THREAD 0
#endif
#endif
// Default to the C++ 11 STL if on VS2015 or has <experimental/filesystem>
#ifndef BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifdef __has_include
// clang-format off
#if __has_include(<filesystem>) || __has_include(<experimental/filesystem>)
// clang-format on
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 0
#endif
#endif
#if !defined(BOOST_AFIO_USE_BOOST_FILESYSTEM) && _MSC_VER >= 1900 /* >= VS2015 */
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 0
#endif
#endif
#ifndef BOOST_AFIO_USE_BOOST_FILESYSTEM
//! \brief Whether to use Boost.Filesystem instead of the C++ 17 TS `std::filesystem`.
//! Defaults to the C++ 17 TS filesystem if that is available, else Boost. \ingroup config
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 1
#endif
#if BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
#define BOOST_KERNELTEST_V1_FILESYSTEM_IMPL boost
#define BOOST_KERNELTEST_USE_LEGACY_FILESYSTEM_SEMANTICS 1
#else
#define BOOST_KERNELTEST_V1_FILESYSTEM_IMPL std
#endif
#define BOOST_KERNELTEST_V1 (boost), (kerneltest), (BOOSTLITE_BIND_NAMESPACE_VERSION(v1, BOOST_KERNELTEST_V1_STL11_IMPL, BOOST_KERNELTEST_V1_FILESYSTEM_IMPL), inline)
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
#define BOOST_KERNELTEST_V1_NAMESPACE BOOSTLITE_BIND_NAMESPACE(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_BEGIN BOOSTLITE_BIND_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_EXPORT_BEGIN BOOSTLITE_BIND_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_END BOOSTLITE_BIND_NAMESPACE_END(BOOST_KERNELTEST_V1)
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
#include "../boost-lite/include/bind/stl11/std/system_error"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace stl11
{
  using namespace boost_lite::bind::std::atomic;
  using namespace boost_lite::bind::std::system_error;
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
#if BOOST_KERNELTEST_USE_BOOST_FILESYSTEM
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


// Bring in the Boost-lite macros
#include "../boost-lite/include/config.hpp"
// Bring in scoped undo
#include "../boost-lite/include/scoped_undo.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
using BOOSTLITE_NAMESPACE::scoped_undo::undoer;
BOOST_KERNELTEST_V1_NAMESPACE_END

// Configure BOOST_KERNELTEST_DECL
#if(defined(BOOST_KERNELTEST_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_KERNELTEST_STATIC_LINK)

#if defined(BOOST_KERNELTEST_SOURCE)
#define BOOST_KERNELTEST_DECL BOOSTLITE_SYMBOL_EXPORT
#define BOOST_KERNELTEST_BUILD_DLL
#else
#define BOOST_KERNELTEST_DECL BOOSTLITE_SYMBOL_IMPORT
#endif
#else
#define BOOST_KERNELTEST_DECL
#endif  // building a shared library


#if BOOST_KERNELTEST_HEADERS_ONLY == 1 && !defined(BOOST_KERNELTEST_SOURCE)
/*! \brief Expands into the appropriate markup to declare an `extern`
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define BOOST_KERNELTEST_HEADERS_ONLY_FUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a class member
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define BOOST_KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a virtual class member
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define BOOST_KERNELTEST_HEADERS_ONLY_VIRTUAL_SPEC inline virtual
#else
#define BOOST_KERNELTEST_HEADERS_ONLY_FUNC_SPEC extern BOOST_KERNELTEST_DECL
#define BOOST_KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC
#define BOOST_KERNELTEST_HEADERS_ONLY_VIRTUAL_SPEC virtual
#endif


#include "../outcome/include/boost/outcome.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
// We are so heavily tied into Outcome we just import it wholesale into our namespace
using namespace BOOST_OUTCOME_V1_NAMESPACE;
// Force these to the same overloading precedence as if they were defined in the KernelTest namespace
using BOOST_OUTCOME_V1_NAMESPACE::outcome;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_result;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_outcome;
BOOST_KERNELTEST_V1_NAMESPACE_END

// We need an aggregate initialisable collection of heterogeneous types
#if __cplusplus >= 20170000L || __GNUC__ >= 6 || BOOST_KERNELTEST_HAVE_AGGREGATE_TUPLE
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

#if !defined(BOOST_KERNELTEST_COUT) && !defined(BOOST_KERNELTEST_CERR)
#include <iostream>
#endif

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

//! Lets you redefine where cout is sent
#ifndef BOOST_KERNELTEST_COUT
#define BOOST_KERNELTEST_COUT(...) std::cout << __VA_ARGS__
#endif
//! Lets you redefine where cerr is sent
#ifndef BOOST_KERNELTEST_CERR
#define BOOST_KERNELTEST_CERR(...) std::cerr << __VA_ARGS__
#endif

//! Many <filesystem> TS implementations are not implementing std::hash for filesystem::path
struct path_hasher
{
  size_t operator()(const stl1z::filesystem::path &p) const { return std::hash<stl1z::filesystem::path::string_type>()(p.native()); }
};

//! Contains various hooks which can be inserted into a `parameter_permuter`
namespace hooks
{
}
//! Alias hooks to precondition
namespace precondition = hooks;
//! Alias hooks to postcondition
namespace postcondition = hooks;

//! Keeps what the current test kernel is for the calling thread
static BOOSTLITE_THREAD_LOCAL struct current_test_kernel_t
{
  const char *category;     //!< The category of the current test kernel
  const char *product;      //!< The product of the current test kernel
  const char *test;         //!< The current test kernel
  const char *name;         //!< The name of the test within the current test kernel
  const char *description;  //!< The human readable description of the test
  //! The working directory for the calling thread, if any (see hooks::filesystem_setup).
  const stl1z::filesystem::path::value_type *working_directory;
} current_test_kernel;


//! \brief Enumeration of the ways in which a kernel test may fail
enum class kerneltest_errc
{
  check_failed = 1,  //!< A BOOST_KERNELTEST_CHECK failed

  setup_exception_thrown = 4,     //!< A C++ exception was thrown during the kernel hook setup
  kernel_exception_thrown = 5,    //!< A C++ exception was thrown during the kernel execution
  teardown_exception_thrown = 6,  //!< A C++ exception was thrown during the kernel teardown

  setup_seh_exception_thrown = 8,      //!< A SEH exception was thrown during the kernel hook setup
  kernel_seh_exception_thrown = 9,     //!< A SEH exception was thrown during the kernel execution
  teardown_seh_exception_thrown = 10,  //!< A SEH exception was thrown during the kernel teardown

  setup_signal_thrown = 12,     //!< A signal was thrown during the kernel hook setup
  kernel_signal_thrown = 13,    //!< A signal was thrown during the kernel execution
  teardown_signal_thrown = 14,  //!< A signal was thrown during the kernel teardown

  filesystem_setup_internal_failure = 256,  //!< hooks::filesystem_setup failed during setup or teardown
  filesystem_comparison_internal_failure,   //!< hooks::filesystem_comparison failed during setup or teardown
  filesystem_comparison_failed              //!< hooks::filesystem_comparison found workspaces differed
};

namespace detail
{
  class kerneltest_category : public stl11::error_category
  {
  public:
    virtual const char *name() const noexcept { return "kerneltest"; }
    virtual std::string message(int c) const
    {
      switch(c)
      {
      case 1:
        return "BOOST_KERNELTEST_CHECK() failure";

      case 4:
        return "C++ exception thrown during kernel setup";
      case 5:
        return "C++ exception thrown during kernel execution";
      case 6:
        return "C++ exception thrown during kernel teardown";

      case 8:
        return "SEH exception thrown during kernel setup";
      case 9:
        return "SEH exception thrown during kernel execution";
      case 10:
        return "SEH exception thrown during kernel teardown";

      case 12:
        return "signal thrown during kernel setup";
      case 13:
        return "signal thrown during kernel execution";
      case 14:
        return "signal thrown during kernel teardown";

      case 256:
        return "filesystem_setup internal failure";
      case 257:
        return "filesystem_comparison internal failure";
      case 258:
        return "filesystem comparison failed";

      default:
        return "unknown";
      }
    }
  };
}

/*! \brief Returns a reference to a kerneltest error category. Note the address
of one of these may not be constant throughout the process as per the ISO spec.
*/
inline const detail::kerneltest_category &kerneltest_category()
{
  static detail::kerneltest_category c;
  return c;
}

//! \brief A kerneltest error object
class BOOSTLITE_SYMBOL_VISIBLE kerneltest_error : public std::system_error
{
public:
  kerneltest_error(stl11::error_code ec)
      : std::system_error(ec)
  {
  }
};

//! \brief ADL looked up by the STL to convert a kerneltest_errc into an error_code.
inline stl11::error_code make_error_code(kerneltest_errc e)
{
  return stl11::error_code(static_cast<int>(e), kerneltest_category());
}


BOOST_KERNELTEST_V1_NAMESPACE_END

#endif  // need define
