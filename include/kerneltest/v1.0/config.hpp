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

#if !defined(KERNELTEST_EXPERIMENTAL_STATUS_CODE)
//! \brief Whether to use SG14 experimental `status_code` instead of `std::error_code`
#define KERNELTEST_EXPERIMENTAL_STATUS_CODE 0
#endif


#include "quickcpplib/cpp_feature.h"

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

#include "quickcpplib/import.h"

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
#include "quickcpplib/config.hpp"
// Bring in a scope implementation
#include "quickcpplib/scope.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
using QUICKCPPLIB_NAMESPACE::scope::make_scope_exit;
using QUICKCPPLIB_NAMESPACE::scope::make_scope_fail;
using QUICKCPPLIB_NAMESPACE::scope::make_scope_success;
KERNELTEST_V1_NAMESPACE_END
// Bring in an optional implementation
#include "quickcpplib/optional.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
using namespace QUICKCPPLIB_NAMESPACE::optional;
KERNELTEST_V1_NAMESPACE_END
// Bring in filesystem
#if defined(__has_include)
// clang-format off
#if !KERNELTEST_FORCE_EXPERIMENTAL_FILESYSTEM && __has_include(<filesystem>) && (__cplusplus >= 201700 || _HAS_CXX17)
#include <filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::filesystem;
KERNELTEST_V1_NAMESPACE_END
// C++ 14 filesystem support was dropped in VS2019 16.3
// C++ 14 filesystem support was dropped in LLVM 11
#elif __has_include(<experimental/filesystem>) && (!defined(_MSC_VER) || _MSC_VER < 1923) && (!defined(_LIBCPP_VERSION) || _LIBCPP_VERSION < 11000)  
#include <experimental/filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::experimental::filesystem;
KERNELTEST_V1_NAMESPACE_END
#elif !KERNELTEST_FORCE_EXPERIMENTAL_FILESYSTEM && __has_include(<filesystem>)
#if defined(_MSC_VER) && _MSC_VER >= 1923
#error MSVC dropped support for C++ 14 <filesystem> from VS2019 16.3 onwards. Please enable C++ 17 or later.
#endif
#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 11000
#error libc++ dropped support for C++ 14 <filesystem> from LLVM 11 onwards. Please enable C++ 17 or later.
#endif
#include <filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::filesystem;
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


#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
#include "outcome/experimental/status_outcome.hpp"
#include "outcome/iostream_support.hpp"
#include "outcome/experimental/status-code/include/system_code_from_exception.hpp"
#include "outcome/iostream_support.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code> using result = OUTCOME_V2_NAMESPACE::experimental::status_result<R, S>;
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class P = std::exception_ptr> using outcome = OUTCOME_V2_NAMESPACE::experimental::status_outcome<R, S, P>;
using OUTCOME_V2_NAMESPACE::success;
using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::in_place_type;
KERNELTEST_V1_NAMESPACE_END
#else
#include "outcome/outcome.hpp"
#include "outcome/iostream_support.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
using OUTCOME_V2_NAMESPACE::result;
using OUTCOME_V2_NAMESPACE::outcome;
using OUTCOME_V2_NAMESPACE::success;
using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::in_place_type;
using OUTCOME_V2_NAMESPACE::error_from_exception;
KERNELTEST_V1_NAMESPACE_END
#endif

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
  success = 0,       //!< Nothing failed
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
  inline const char *message(kerneltest_errc c) noexcept
  {
    switch(c)
    {
    case kerneltest_errc::success:
      return "success";
    case kerneltest_errc::check_failed:
      return "KERNELTEST_CHECK() failure";

    case kerneltest_errc::setup_exception_thrown:
      return "C++ exception thrown during kernel setup";
    case kerneltest_errc::kernel_exception_thrown:
      return "C++ exception thrown during kernel execution";
    case kerneltest_errc::teardown_exception_thrown:
      return "C++ exception thrown during kernel teardown";

    case kerneltest_errc::setup_seh_exception_thrown:
      return "SEH exception thrown during kernel setup";
    case kerneltest_errc::kernel_seh_exception_thrown:
      return "SEH exception thrown during kernel execution";
    case kerneltest_errc::teardown_seh_exception_thrown:
      return "SEH exception thrown during kernel teardown";

    case kerneltest_errc::setup_signal_thrown:
      return "signal thrown during kernel setup";
    case kerneltest_errc::kernel_signal_thrown:
      return "signal thrown during kernel execution";
    case kerneltest_errc::teardown_signal_thrown:
      return "signal thrown during kernel teardown";

    case kerneltest_errc::filesystem_setup_internal_failure:
      return "filesystem_setup internal failure";
    case kerneltest_errc::filesystem_comparison_internal_failure:
      return "filesystem_comparison internal failure";
    case kerneltest_errc::filesystem_comparison_failed:
      return "filesystem comparison failed";

    default:
      return "unknown";
    }
  }
}

#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
class _kerneltest_domain;
using kerneltest_code = SYSTEM_ERROR2_NAMESPACE::status_code<_kerneltest_domain>;
class _kerneltest_domain : public SYSTEM_ERROR2_NAMESPACE::status_code_domain
{
  template <class DomainType> friend class SYSTEM_ERROR2_NAMESPACE::status_code;
  using _base = SYSTEM_ERROR2_NAMESPACE::status_code_domain;

public:
  constexpr _kerneltest_domain()
      : _base(0x4915711429d0065a)
  {
  }
  using value_type = kerneltest_errc;
  using string_ref = _base::string_ref;

  static inline constexpr const _kerneltest_domain &get();

  virtual string_ref name() const noexcept override final { return string_ref("kerneltest domain"); }  // NOLINT
protected:
  virtual bool _do_failure(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override
  {
    assert(code.domain() == *this);
    return static_cast<const kerneltest_code &>(code).value() != kerneltest_errc::success;  // NOLINT
  }
  virtual bool _do_equivalent(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code1, const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code2) const noexcept override
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const kerneltest_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const kerneltest_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    return false;
  }
  virtual SYSTEM_ERROR2_NAMESPACE::generic_code _generic_code(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override
  {
    (void) code;
    assert(code.domain() == *this);
    return SYSTEM_ERROR2_NAMESPACE::errc::unknown;
  }
  virtual string_ref _do_message(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const kerneltest_code &>(code);  // NOLINT
    return string_ref(detail::message(c.value()));
  }
  virtual void _do_throw_exception(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const override
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const kerneltest_code &>(code);  // NOLINT
    throw SYSTEM_ERROR2_NAMESPACE::status_error<_kerneltest_domain>(c);
  }
};
constexpr _kerneltest_domain kerneltest_domain;
inline constexpr const _kerneltest_domain &_kerneltest_domain::get()
{
  return kerneltest_domain;
}

//! \brief ADL looked up by the STL to convert a kerneltest_errc into a kerneltest_code.
inline SYSTEM_ERROR2_NAMESPACE::system_code make_error_code(kerneltest_errc e)
{
  return kerneltest_code(e);
}

//! Choose an errc implementation
using SYSTEM_ERROR2_NAMESPACE::errc;

#ifndef _WIN32
//! Helper for constructing an error code from a POSIX errno
inline SYSTEM_ERROR2_NAMESPACE::system_code posix_error(int c = errno)
{
  return SYSTEM_ERROR2_NAMESPACE::posix_code(c);
}
#else
//! Helper for constructing an error code from a DWORD
inline SYSTEM_ERROR2_NAMESPACE::system_code win32_error(SYSTEM_ERROR2_NAMESPACE::win32::DWORD c = SYSTEM_ERROR2_NAMESPACE::win32::GetLastError())
{
  return SYSTEM_ERROR2_NAMESPACE::win32_code(c);
}
#endif

inline SYSTEM_ERROR2_NAMESPACE::system_code error_from_exception(std::exception_ptr &&ep = std::current_exception(), SYSTEM_ERROR2_NAMESPACE::system_code not_matched = SYSTEM_ERROR2_NAMESPACE::generic_code(SYSTEM_ERROR2_NAMESPACE::errc::resource_unavailable_try_again)) noexcept
{
  return SYSTEM_ERROR2_NAMESPACE::system_code_from_exception(std::move(ep), std::move(not_matched));
}

#else
namespace detail
{
  class kerneltest_category : public std::error_category
  {
  public:
    virtual const char *name() const noexcept { return "kerneltest"; }
    virtual std::string message(int c) const { return detail::message(static_cast<kerneltest_errc>(c)); }
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

//! Choose an errc implementation
using std::errc;

#ifndef _WIN32
//! Helper for constructing an error code from a POSIX errno
inline std::error_code posix_error(int c = errno)
{
  return {c, std::system_category()};
}
#else
namespace win32
{
  // A Win32 DWORD
  using DWORD = unsigned long;
  // Used to retrieve the current Win32 error code
  extern DWORD __stdcall GetLastError();
#pragma comment(lib, "kernel32.lib")
#if defined(_WIN64)
#ifdef KERNELTEST_UNSTABLE_VERSION
#define KERNELTEST_GETLASTERROR_SYMBOL2(x) "/alternatename:?GetLastError@win32@kerneltest_v1_" #x "@@YAKXZ=GetLastError"
#else
#define KERNELTEST_GETLASTERROR_SYMBOL2(x) "/alternatename:?GetLastError@win32@kerneltest_v1@@YAKXZ=GetLastError"
#endif
#else
#ifdef KERNELTEST_UNSTABLE_VERSION
#define KERNELTEST_GETLASTERROR_SYMBOL2(x) "/alternatename:?GetLastError@win32@kerneltest_v1_" #x "@@YGKXZ=__imp__GetLastError@0"
#else
#define KERNELTEST_GETLASTERROR_SYMBOL2(x) "/alternatename:?GetLastError@win32@kerneltest_v1@@YGKXZ=__imp__GetLastError@0"
#endif
#endif
#define KERNELTEST_GETLASTERROR_SYMBOL1(x) KERNELTEST_GETLASTERROR_SYMBOL2(x)
#define KERNELTEST_GETLASTERROR_SYMBOL KERNELTEST_GETLASTERROR_SYMBOL1(KERNELTEST_PREVIOUS_COMMIT_UNIQUE)
#pragma comment(linker, KERNELTEST_GETLASTERROR_SYMBOL)
}  // namespace win32

//! Helper for constructing an error code from a DWORD
inline std::error_code win32_error(win32::DWORD c = win32::GetLastError())
{
  return {static_cast<int>(c), std::system_category()};
}
#endif

#endif

KERNELTEST_V1_NAMESPACE_END

#endif
