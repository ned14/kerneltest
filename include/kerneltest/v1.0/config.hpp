/* KernelTest config
(C) 2016-2025 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
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

#if !(_HAS_CXX17 || __cplusplus >= 201700)
#error "KernelTest needs a minimum of C++ 17 support in the compiler"
#endif
#ifdef __has_include
#if !__has_include(<filesystem>)
#error "KernelTest needs an implementation of C++ 17 <filesystem> in the standard library"
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
#define KERNELTEST_V1_NAMESPACE_BEGIN                                                                                                                          \
  namespace kerneltest_v1_xxx                                                                                                                                  \
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
#include <optional>
KERNELTEST_V1_NAMESPACE_BEGIN
template <class T> using optional = std::optional<T>;
KERNELTEST_V1_NAMESPACE_END
// Bring in filesystem
#include <filesystem>
KERNELTEST_V1_NAMESPACE_BEGIN
namespace filesystem = std::filesystem;
KERNELTEST_V1_NAMESPACE_END


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

#ifndef KERNELTEST_EXCEPTION_TRY
#ifndef __cpp_exceptions
#define KERNELTEST_EXCEPTION_TRY if(true)
#else
#define KERNELTEST_EXCEPTION_TRY try
#endif
#endif
#ifndef KERNELTEST_EXCEPTION_CATCH
#ifndef __cpp_exceptions
#define KERNELTEST_EXCEPTION_CATCH(init, ...) else if(__VA_ARGS__ = init; false)
#else
#define KERNELTEST_EXCEPTION_CATCH(init, ...) catch(__VA_ARGS__)
#endif
#endif
#ifndef KERNELTEST_EXCEPTION_CATCH_ALL
#ifndef __cpp_exceptions
#define KERNELTEST_EXCEPTION_CATCH_ALL else if(false)
#else
#define KERNELTEST_EXCEPTION_CATCH_ALL catch(...)
#endif
#endif
#ifndef KERNELTEST_EXCEPTION_THROW
#ifndef __cpp_exceptions
#define KERNELTEST_EXCEPTION_THROW(...)                                                                                                                        \
  {                                                                                                                                                            \
    fprintf(stderr, "FATAL: throw " #__VA_ARGS__ " at " __FILE__ ":%d\n", __LINE__);                                                                           \
    abort();                                                                                                                                                   \
  }
#else
#define KERNELTEST_EXCEPTION_THROW(...) throw(__VA_ARGS__)
#endif
#endif
#ifndef KERNELTEST_EXCEPTION_RETHROW
#ifndef __cpp_exceptions
#define KERNELTEST_EXCEPTION_RETHROW
#else
#define KERNELTEST_EXCEPTION_RETHROW throw
#endif
#endif


#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
#if !OUTCOME_USE_SYSTEM_STATUS_CODE && __has_include("outcome/experimental/status-code/include/status-code/system_code_from_exception.hpp")
#ifdef __cpp_exceptions
#include "outcome/experimental/status-code/include/status-code/system_code_from_exception.hpp"
#endif
#else
#ifdef __cpp_exceptions
#include <status-code/system_code_from_exception.hpp>
#endif
#endif
#include "outcome/experimental/status_outcome.hpp"
#include "outcome/iostream_support.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
#ifdef __cpp_exceptions
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class P = OUTCOME_V2_NAMESPACE::experimental::policy::default_status_result_policy<R, S>>
using result = OUTCOME_V2_NAMESPACE::experimental::status_result<R, S, P>;
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class P = std::exception_ptr,
          class N = OUTCOME_V2_NAMESPACE::experimental::policy::default_status_outcome_policy<R, S, P>>
using outcome = OUTCOME_V2_NAMESPACE::experimental::status_outcome<R, S, P, N>;
#else
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class P = OUTCOME_V2_NAMESPACE::policy::terminate>
using result = OUTCOME_V2_NAMESPACE::experimental::status_result<R, S, P>;
template <class R, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class P = std::exception_ptr, class N = OUTCOME_V2_NAMESPACE::policy::terminate>
using outcome = OUTCOME_V2_NAMESPACE::experimental::status_outcome<R, S, P, N>;
#endif
using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::in_place_type;
using OUTCOME_V2_NAMESPACE::success;
KERNELTEST_V1_NAMESPACE_END
#else
#include "outcome/iostream_support.hpp"
#include "outcome/outcome.hpp"
KERNELTEST_V1_NAMESPACE_BEGIN
#ifdef __cpp_exceptions
using OUTCOME_V2_NAMESPACE::error_from_exception;
#else
inline std::error_code error_from_exception(std::exception_ptr && = std::current_exception(),
                                            std::error_code = std::make_error_code(std::errc::resource_unavailable_try_again)) noexcept
{
  abort();  // should never be called
}
#endif
using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::in_place_type;
#ifdef __cpp_exceptions
using OUTCOME_V2_NAMESPACE::outcome;
using OUTCOME_V2_NAMESPACE::result;
#else
template <class R, class S = std::error_code, class P = std::exception_ptr, class N = OUTCOME_V2_NAMESPACE::policy::terminate>
using outcome = OUTCOME_V2_NAMESPACE::outcome<R, S, P, N>;
template <class R, class S = std::error_code, class P = OUTCOME_V2_NAMESPACE::policy::terminate> using result = OUTCOME_V2_NAMESPACE::result<R, S, P>;
#endif
using OUTCOME_V2_NAMESPACE::success;
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
}  // namespace detail

#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
class _kerneltest_domain;
using kerneltest_code = SYSTEM_ERROR2_NAMESPACE::status_code<_kerneltest_domain>;
class _kerneltest_domain final : public SYSTEM_ERROR2_NAMESPACE::status_code_domain
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
  using payload_info_t = _base::payload_info_t;

  static inline constexpr const _kerneltest_domain &get();

  virtual string_ref name() const noexcept override final { return string_ref("kerneltest domain"); }  // NOLINT
  virtual payload_info_t payload_info() const noexcept override
  {
    return {sizeof(value_type), sizeof(status_code_domain *) + sizeof(value_type),
            (alignof(value_type) > alignof(status_code_domain *)) ? alignof(value_type) : alignof(status_code_domain *)};
  }

protected:
  virtual bool _do_failure(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override
  {
    assert(code.domain() == *this);
    return static_cast<const kerneltest_code &>(code).value() != kerneltest_errc::success;  // NOLINT
  }
  virtual bool _do_equivalent(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code1,
                              const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code2) const noexcept override
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
    KERNELTEST_EXCEPTION_THROW(SYSTEM_ERROR2_NAMESPACE::status_error<_kerneltest_domain>(c));
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

inline SYSTEM_ERROR2_NAMESPACE::system_code error_from_exception(std::exception_ptr &&ep = std::current_exception(),
                                                                 SYSTEM_ERROR2_NAMESPACE::system_code not_matched = SYSTEM_ERROR2_NAMESPACE::generic_code(
                                                                 SYSTEM_ERROR2_NAMESPACE::errc::resource_unavailable_try_again)) noexcept
{
#ifdef __cpp_exceptions
  return SYSTEM_ERROR2_NAMESPACE::system_code_from_exception(std::move(ep), std::move(not_matched));
#else
  abort();
#endif
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
}  // namespace detail

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
