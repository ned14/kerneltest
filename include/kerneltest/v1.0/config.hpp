/* KernelTest config
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: Apr 2016


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef KERNELTEST_CONFIG_HPP
#define KERNELTEST_CONFIG_HPP

#if !defined(KERNELTEST_HEADERS_ONLY) && !defined(BOOST_ALL_DYN_LINK)
//! \brief Whether KernelTest is a headers only library. Defaults to 1 unless BOOST_ALL_DYN_LINK is defined. \ingroup config
#define KERNELTEST_HEADERS_ONLY 1
#endif

#include "quickcpplib/include/cpp_feature.h"

#ifndef __cpp_exceptions
#error KernelTest needs C++ exceptions to be turned on
#endif
#ifndef __cpp_alias_templates
#error KernelTest needs template alias support in the compiler
#endif
#ifndef __cpp_variadic_templates
#error KernelTest needs variadic template support in the compiler
#endif
#if __cpp_constexpr < 201304L && !defined(_MSC_VER)
#error KernelTest needs relaxed constexpr (C++ 14) support in the compiler
#endif
#ifdef __has_include
// clang-format off
#if !__has_include(<filesystem>) && !__has_include(<experimental/filesystem>)
// clang-format on
#error KernelTest needs an implementation of the Filesystem TS in the standard library
#endif
#endif

#include "quickcpplib/include/import.h"

#if defined(KERNELTEST_UNSTABLE_VERSION)
#include "../revision.hpp"
#define KERNELTEST_V1 (QUICKCPPLIB_BIND_NAMESPACE_VERSION(kerneltest_v1, KERNELTEST_PREVIOUS_COMMIT_UNIQUE))
#else
#define KERNELTEST_V1 (QUICKCPPLIB_BIND_NAMESPACE_VERSION(kerneltest_v1))
#endif

#if DOXYGEN_SHOULD_SKIP_THIS
#define KERNELTEST_V1_NAMESPACE kerneltest_v1_xxx
#define KERNELTEST_V1_NAMESPACE_BEGIN                                                                                                                                                                                                                                                                                          \
  namespace kerneltest_v1_xxx                                                                                                                                                                                                                                                                                                  \
  {

#define KERNELTEST_V1_NAMESPACE_END }

#else
#define KERNELTEST_V1_NAMESPACE QUICKCPPLIB_BIND_NAMESPACE(KERNELTEST_V1)
#define KERNELTEST_V1_NAMESPACE_BEGIN QUICKCPPLIB_BIND_NAMESPACE_BEGIN(KERNELTEST_V1)
#define KERNELTEST_V1_NAMESPACE_EXPORT_BEGIN QUICKCPPLIB_BIND_NAMESPACE_BEGIN(KERNELTEST_V1)
#define KERNELTEST_V1_NAMESPACE_END QUICKCPPLIB_BIND_NAMESPACE_END(KERNELTEST_V1)
#endif

// Bring in the Boost-lite macros
#include "quickcpplib/include/config.hpp"
// Bring in scoped undo
#include "quickcpplib/include/scoped_undo.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
using QUICKCPPLIB_NAMESPACE::scoped_undo::undoer;
KERNELTEST_V1_NAMESPACE_END
// Bring in an optional implementation
#include "quickcpplib/include/optional.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
using namespace QUICKCPPLIB_NAMESPACE::optional;
KERNELTEST_V1_NAMESPACE_END
// Bring in filesystem
#if defined(__has_include)
// clang-format off
#if __has_include(<filesystem>) && __cplusplus > 201700
#include <filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::filesystem;
KERNELTEST_V1_NAMESPACE_END
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::experimental::filesystem;
KERNELTEST_V1_NAMESPACE_END
#endif
// clang-format on
#elif defined(_MSC_VER)
#include <filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::experimental::filesystem;
KERNELTEST_V1_NAMESPACE_END
#else
#error No <filesystem> implementation found
#endif


// Configure KERNELTEST_DECL
#if(defined(KERNELTEST_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(KERNELTEST_STATIC_LINK)

#if defined(KERNELTEST_SOURCE)
#define KERNELTEST_DECL QUICKCPPLIB_SYMBOL_EXPORT
#define KERNELTEST_BUILD_DLL
#else
#define KERNELTEST_DECL QUICKCPPLIB_SYMBOL_IMPORT
#endif
#else
#define KERNELTEST_DECL
#endif  // building a shared library


#if KERNELTEST_HEADERS_ONLY == 1 && !defined(KERNELTEST_SOURCE)
/*! \brief Expands into the appropriate markup to declare an `extern`
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define KERNELTEST_HEADERS_ONLY_FUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a class member
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a virtual class member
function exported from the KernelTest DLL if not building headers only.
\ingroup config
*/
#define KERNELTEST_HEADERS_ONLY_VIRTUAL_SPEC inline virtual
#else
#define KERNELTEST_HEADERS_ONLY_FUNC_SPEC extern KERNELTEST_DECL
#define KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC
#define KERNELTEST_HEADERS_ONLY_VIRTUAL_SPEC virtual
#endif


#include "outcome/include/outcome.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
// We are so heavily tied into Outcome we just import it wholesale into our namespace
using namespace OUTCOME_V2_NAMESPACE;
KERNELTEST_V1_NAMESPACE_END

// We need an aggregate initialisable collection of heterogeneous types
#include <tuple>
KERNELTEST_V1_NAMESPACE_BEGIN
template <class... Types> using parameters = std::tuple<Types...>;
template <class T> using parameters_size = std::tuple_size<T>;
template <size_t N, class T> using parameters_element = std::tuple_element<N, T>;
KERNELTEST_V1_NAMESPACE_END

#if !defined(KERNELTEST_COUT) && !defined(KERNELTEST_CERR)
#include <iostream>
#endif

KERNELTEST_V1_NAMESPACE_BEGIN

//! Lets you redefine where cout is sent
#ifndef KERNELTEST_COUT
#define KERNELTEST_COUT(...) std::cout << __VA_ARGS__
#endif
//! Lets you redefine where cerr is sent
#ifndef KERNELTEST_CERR
#define KERNELTEST_CERR(...) std::cerr << __VA_ARGS__
#endif

//! Many <filesystem> TS implementations are not implementing std::hash for filesystem::path
struct path_hasher
{
  size_t operator()(const filesystem::path &p) const { return std::hash<filesystem::path::string_type>()(p.native()); }
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
static QUICKCPPLIB_THREAD_LOCAL struct current_test_kernel_t
{
  const char *category;     //!< The category of the current test kernel
  const char *product;      //!< The product of the current test kernel
  const char *test;         //!< The current test kernel
  const char *name;         //!< The name of the test within the current test kernel
  const char *description;  //!< The human readable description of the test
  //! The working directory for the calling thread, if any (see hooks::filesystem_setup).
  const filesystem::path::value_type *working_directory;
} current_test_kernel;


//! \brief Enumeration of the ways in which a kernel test may fail
enum class kerneltest_errc
{
  check_failed = 1,  //!< A KERNELTEST_CHECK failed

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
  class kerneltest_category : public std::error_category
  {
  public:
    virtual const char *name() const noexcept { return "kerneltest"; }
    virtual std::string message(int c) const
    {
      switch(c)
      {
      case 1:
        return "KERNELTEST_CHECK() failure";

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
class QUICKCPPLIB_SYMBOL_VISIBLE kerneltest_error : public std::system_error
{
public:
  kerneltest_error(std::error_code ec)
      : std::system_error(ec)
  {
  }
};

//! \brief ADL looked up by the STL to convert a kerneltest_errc into an error_code.
inline std::error_code make_error_code(kerneltest_errc e)
{
  return std::error_code(static_cast<int>(e), kerneltest_category());
}


KERNELTEST_V1_NAMESPACE_END

#endif
