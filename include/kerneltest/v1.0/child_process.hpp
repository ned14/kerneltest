/* Routines for handling child processes
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Marc 2016


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

#include "config.hpp"

#ifndef KERNELTEST_CHILD_PROCESS_H
#define KERNELTEST_CHILD_PROCESS_H

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)  // dll interface
#endif

KERNELTEST_V1_NAMESPACE_BEGIN

namespace child_process
{
  // Native handle support
  namespace win
  {
    using handle = void *;
    using dword = unsigned long;
  }

  //! A native handle type
  union native_handle_type {
    int fd;         //!< A POSIX file descriptor
    int pid;        //!< A POSIX process identifier
    win::handle h;  //!< A Windows HANDLE
    
    constexpr native_handle_type() noexcept :
#ifdef _WIN32
    h(nullptr)
#else
    fd(-1)
#endif
     { }
     explicit operator bool() const noexcept {
#ifdef _WIN32
       return h != nullptr;
#else
       return fd != -1;
#endif
     }
  };

  //! Returns the path of the calling process
  KERNELTEST_HEADERS_ONLY_FUNC_SPEC filesystem::path current_process_path();

  //! Returns the environment of the calling process
  KERNELTEST_HEADERS_ONLY_FUNC_SPEC std::map<filesystem::path::string_type, filesystem::path::string_type> current_process_env();

  /*! \class child_process
  \brief Launches and manages a child process with stdin, stdout and stderr.

  \todo If we ever make more use of child_process, its handles need to be managed by
  handle and use async_pipe_handle or something as the handle implementation to avoid
  deadlocking stdout and stderr.
  */
  class KERNELTEST_DECL child_process
  {
    filesystem::path _path;
    native_handle_type _processh;
    native_handle_type _readh, _writeh, _errh;
    bool _use_parent_errh;
    std::vector<filesystem::path::string_type> _args;
    std::map<filesystem::path::string_type, filesystem::path::string_type> _env;
    FILE *_stdin, *_stdout, *_stderr;
    std::ostream *_cin;
    std::istream *_cout, *_cerr;

  protected:
    child_process(filesystem::path path, bool use_parent_errh, std::vector<filesystem::path::string_type> args, std::map<filesystem::path::string_type, filesystem::path::string_type> env)
        : _path(std::move(path))
        , _use_parent_errh(use_parent_errh)
        , _args(std::move(args))
        , _env(std::move(env))
        , _stdin(nullptr)
        , _stdout(nullptr)
        , _stderr(nullptr)
        , _cin(nullptr)
        , _cout(nullptr)
        , _cerr(nullptr)
    {
    }

    void _initialise_files() const;
    void _deinitialise_files();
    void _initialise_streams() const;
    void _deinitialise_streams();

  public:
    child_process(const child_process &) = delete;
    child_process(child_process &&o) noexcept : _path(std::move(o._path)),
                                                _processh(std::move(o._processh)),
                                                _readh(std::move(o._readh)),
                                                _writeh(std::move(o._writeh)),
                                                _errh(std::move(o._errh)),
                                                _use_parent_errh(std::move(o._use_parent_errh)),
                                                _args(std::move(o._args)),
                                                _env(std::move(o._env)),
                                                _stdin(std::move(o._stdin)),
                                                _stdout(std::move(o._stdout)),
                                                _stderr(std::move(o._stderr)),
                                                _cin(std::move(o._cin)),
                                                _cout(std::move(o._cout)),
                                                _cerr(std::move(o._cerr))
    {
      o._processh = native_handle_type();
      o._readh = native_handle_type();
      o._writeh = native_handle_type();
      o._errh = native_handle_type();
      o._stdin = nullptr;
      o._stdout = nullptr;
      o._stderr = nullptr;
      o._cin = nullptr;
      o._cout = nullptr;
      o._cerr = nullptr;
    }
    child_process &operator=(const child_process &) = delete;
    child_process &operator=(child_process &&o) noexcept
    {
      this->~child_process();
      new(this) child_process(std::move(o));
      return *this;
    }
    ~child_process();

    //! Launches an executable as a child process. No shell is invoked on POSIX.
    static KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC result<child_process> launch(filesystem::path path, std::vector<filesystem::path::string_type> args, std::map<filesystem::path::string_type, filesystem::path::string_type> env = current_process_env(),
                                                                                   bool use_parent_errh = false) noexcept;

    //! Returns the path of the executable
    const filesystem::path &path() const noexcept { return _path; }
    //! Returns the args used to launch the executable
    const std::vector<filesystem::path::string_type> &arguments() const noexcept { return _args; }
    //! Returns the environment used to launch the executable
    const std::map<filesystem::path::string_type, filesystem::path::string_type> &environment() const noexcept { return _env; }
    //! Returns the process identifier
    const native_handle_type &process_native_handle() const noexcept { return _processh; }
    //! Returns the read handle
    const native_handle_type &read_native_handle() const noexcept { return _readh; }
    //! Returns the write handle
    const native_handle_type &write_native_handle() const noexcept { return _writeh; }
    //! Returns the error handle
    const native_handle_type &error_native_handle() const noexcept { return _errh; }

    //! Returns the read handle as a FILE *
    FILE *file_in() const
    {
      if(!_stdin)
        _initialise_files();
      return _stdin;
    }
    //! Returns the write handle as a FILE *
    FILE *file_out() const
    {
      if(!_stdout)
        _initialise_files();
      return _stdout;
    }
    //! Returns the error handle as a FILE *
    FILE *file_err() const
    {
      if(!_stderr)
        _initialise_files();
      return _stderr;
    }

    //! Returns the read handle as a ostream &
    std::ostream &cin() const
    {
      if(!_cin)
        _initialise_streams();
      return *_cin;
    }
    //! Returns the write handle as a istream &
    std::istream &cout() const
    {
      if(!_cout)
        _initialise_streams();
      return *_cout;
    }
    //! Returns the error handle as a istream &
    std::istream &cerr() const
    {
      if(!_cerr)
        _initialise_streams();
      return *_cerr;
    }

    //! True if child process is currently running
    bool is_running() const noexcept;

    //! Waits for a child process to exit until deadline /em d
    result<intptr_t> wait_until(std::chrono::steady_clock::time_point d) noexcept;
    //! \overload
    result<intptr_t> wait() noexcept { return wait_until(std::chrono::steady_clock::time_point()); }
  };
}

KERNELTEST_V1_NAMESPACE_END

#if KERNELTEST_HEADERS_ONLY == 1 && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#define KERNELTEST_INCLUDED_BY_HEADER 1
#include "detail/impl/child_process.ipp"
#undef KERNELTEST_INCLUDED_BY_HEADER
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
