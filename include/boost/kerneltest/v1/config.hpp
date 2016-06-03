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
#define BOOST_KERNELTEST_V1 (boost), (kerneltest), (BOOST_BINDLIB_NAMESPACE_VERSION(v1, BOOST_KERNELTEST_V1_STL11_IMPL, BOOST_KERNELTEST_V1_FILESYSTEM_IMPL), inline)
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
#define BOOST_KERNELTEST_V1_NAMESPACE BOOST_BINDLIB_NAMESPACE(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1)
#define BOOST_KERNELTEST_V1_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1)
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

#define BOOST_STL11_ATOMIC_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_ATOMIC_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR32_T  // missing VS14
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR16_T  // missing VS14
#define BOOST_STL11_CHRONO_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11), (chrono))
#define BOOST_STL11_CHRONO_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11), (chrono))
#define BOOST_STL11_CONDITION_VARIABLE_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_CONDITION_VARIABLE_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL1z_FILESYSTEM_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl1z), (filesystem))
#define BOOST_STL1z_FILESYSTEM_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl1z), (filesystem))
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
#define BOOST_STL11_FUTURE_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_FUTURE_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_FUTURE_MAP_NO_FUTURE
#define BOOST_STL11_MUTEX_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_MUTEX_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_RATIO_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_RATIO_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_THREAD_MAP_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_KERNELTEST_V1, (stl11))
#define BOOST_STL11_THREAD_MAP_NAMESPACE_END BOOST_BINDLIB_NAMESPACE_END(BOOST_KERNELTEST_V1, (stl11))
// clang-format off
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, atomic)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, chrono)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, condition_variable)
#include BOOST_BINDLIB_INCLUDE_STL1z(../boost-lite, BOOST_KERNELTEST_V1_FILESYSTEM_IMPL, filesystem)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, future)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, mutex)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, ratio)
#include BOOST_BINDLIB_INCLUDE_STL11(../boost-lite, BOOST_KERNELTEST_V1_STL11_IMPL, thread)
// clang-format on


#include "../outcome/include/boost/outcome.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
// We are so heavily tied into Outcome we just import it wholesale into our namespace
using namespace BOOST_OUTCOME_V1_NAMESPACE;
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

//#define BOOST_CATCH_CUSTOM_MAIN_DEFINED
#include "../boost-lite/include/boost/test/unit_test.hpp"


BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
namespace console_colours
{
#ifdef _WIN32
  namespace detail
  {
    inline bool &am_in_bold()
    {
      static bool v;
      return v;
    }
    inline void set(WORD v)
    {
      if(am_in_bold())
        v |= FOREGROUND_INTENSITY;
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), v);
    }
  }
  inline std::ostream &red(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED);
    return s;
  }
  inline std::ostream &green(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_GREEN);
    return s;
  }
  inline std::ostream &blue(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_BLUE);
    return s;
  }
  inline std::ostream &yellow(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_GREEN);
    return s;
  }
  inline std::ostream &magenta(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_BLUE);
    return s;
  }
  inline std::ostream &cyan(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_GREEN | FOREGROUND_BLUE);
    return s;
  }
  inline std::ostream &white(std::ostream &s)
  {
    s.flush();
    detail::set(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return s;
  }
  inline std::ostream &bold(std::ostream &s)
  {
    detail::am_in_bold() = true;
    return s;
  }
  inline std::ostream &normal(std::ostream &s)
  {
    detail::am_in_bold() = false;
    return white(s);
  }
#else
  constexpr const char red[] = {0x1b, '[', '3', '1', 'm', 0};
  constexpr const char green[] = {0x1b, '[', '3', '2', 'm', 0};
  constexpr const char blue[] = {0x1b, '[', '3', '4', 'm', 0};
  constexpr const char yellow[] = {0x1b, '[', '3', '3', 'm', 0};
  constexpr const char magenta[] = {0x1b, '[', '3', '5', 'm', 0};
  constexpr const char cyan[] = {0x1b, '[', '3', '6', 'm', 0};
  constexpr const char white[] = {0x1b, '[', '3', '7', 'm', 0};
  constexpr const char bold[] = {0x1b, '[', '1', 'm', 0};
  constexpr const char normal[] = {0x1b, '[', '0', 'm', 0};
#endif
}
template <class T> inline void print_result(bool v, const T &result)
{
  using namespace console_colours;
  if(v)
    std::cout << bold << green << result << normal << std::endl;
  else
    std::cout << bold << red << "FAILED" << normal << std::endl;
}
BOOST_KERNELTEST_V1_NAMESPACE_END

#endif  // need define
