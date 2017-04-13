/* child_process.hpp
Routines for handling child processes
(C) 2016 Niall Douglas http://www.nedprod.com/
File Created: Marc 2016


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "../../../child_process.hpp"

#include <limits.h>
#include <fcntl.h>
#include <signal.h>  // for siginfo_t
#include <unistd.h>
#include <sys/wait.h>

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

namespace child_process
{
  child_process::~child_process()
  {
    (void) wait();
    if(_stdin || _cin)
    {
      // Handles are already closed, no need to do so again
      _readh.fd = -1;
      _writeh.fd = -1;
      _errh.fd = -1;
    }
    _deinitialise_files();
    _deinitialise_streams();
    if(_processh)
    {
      ::close(_processh.fd);
      _processh.fd = -1;
    }
    if(_readh)
    {
      ::close(_readh.fd);
      _readh.fd = -1;
    }
    if(_writeh)
    {
      ::close(_writeh.fd);
      _writeh.fd = -1;
    }
    if(_errh)
    {
      if(!_use_parent_errh)
        ::close(_errh.fd);
      _errh.fd = -1;
    }
  }

  BOOST_KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC result<child_process> child_process::launch(stl1z::filesystem::path __path, std::vector<stl1z::filesystem::path::string_type> __args, std::map<stl1z::filesystem::path::string_type, stl1z::filesystem::path::string_type> __env, bool use_parent_errh) noexcept
  {
    child_process ret(std::move(__path), use_parent_errh, std::move(__args), std::move(__env));
    native_handle_type childreadh, childwriteh, childerrh;

    int temp[2];
    if(-1 == ::pipe(temp))
      return make_errored_result<child_process>(errno);
    childreadh.fd = temp[0];
    ret._readh.fd = temp[1];
    if(-1 == ::pipe(temp))
      return make_errored_result<child_process>(errno);
    ret._writeh.fd = temp[0];
    childwriteh.fd = temp[1];

    if(use_parent_errh)
    {
      childerrh.fd = STDERR_FILENO;
    }
    else
    {
      if(-1 == ::pipe(temp))
        return make_errored_result<child_process>(errno);
      ret._errh.fd = temp[0];
      childerrh.fd = temp[1];
    }

    auto unmypipes = undoer([&] {
      ::close(ret._readh.fd);
      ret._readh.fd = -1;
      ::close(ret._writeh.fd);
      ret._writeh.fd = -1;
      if(!use_parent_errh)
      {
        ::close(ret._errh.fd);
        ret._errh.fd = -1;
      }
    });
    auto unhispipes = undoer([&] {
      ::close(childreadh.fd);
      ::close(childwriteh.fd);
      if(!use_parent_errh)
        ::close(childerrh.fd);
    });

    if(-1 == ::fcntl(ret._readh.fd, F_SETFD, FD_CLOEXEC))
      return make_errored_result<child_process>(errno);
    if(-1 == ::fcntl(ret._writeh.fd, F_SETFD, FD_CLOEXEC))
      return make_errored_result<child_process>(errno);
    if(!use_parent_errh && -1 == ::fcntl(ret._errh.fd, F_SETFD, FD_CLOEXEC))
      return make_errored_result<child_process>(errno);

    std::vector<const char *> argptrs(ret._args.size() + 2);
    argptrs[0] = ret._path.c_str();
    for(size_t n = 0; n < ret._args.size(); ++n)
      argptrs[n + 1] = ret._args[n].c_str();
    std::vector<std::string> envs;
    std::vector<const char *> envptrs;
    envs.reserve(ret._env.size());
    envptrs.reserve(ret._env.size() + 1);
    for(const auto &i : ret._env)
    {
      envs.push_back(i.first + "=" + i.second);
      envptrs.push_back(envs.back().c_str());
    }
    envptrs.push_back(nullptr);
    ret._processh.fd = ::fork();
    if(0 == ret._processh.fd)
    {
      // I am the child
      ::dup2(childreadh.fd, STDIN_FILENO);
      ::dup2(childwriteh.fd, STDOUT_FILENO);
      if(!use_parent_errh)
        ::dup2(childerrh.fd, STDERR_FILENO);
      ::exit(::execve(ret._path.c_str(), (char **) argptrs.data(), (char **) envptrs.data()));
    }
    if(-1 == ret._processh.fd)
      return make_errored_result<child_process>(errno);
    unmypipes.dismiss();

    // Wait until the primary thread has launched
    ::usleep(30 * 1000);

    return std::move(ret);
  }

  bool child_process::is_running() const noexcept
  {
    if(!_processh)
      return false;
    siginfo_t info;
    memset(&info, 0, sizeof(info));
    if(-1 == ::waitid(P_PID, _processh.pid, &info, WNOHANG|WNOWAIT))
      return false;
    return info.si_pid != 0;
  }

  result<intptr_t> child_process::wait_until(stl11::chrono::steady_clock::time_point d) noexcept
  {
    if(!_processh)
      return make_errored_result<intptr_t>(ECHILD);
    intptr_t ret = 0;
    auto check_child = [&]() -> result<bool> {
      siginfo_t info;
      memset(&info, 0, sizeof(info));
      int options = WEXITED|WSTOPPED;
      if(d != stl11::chrono::steady_clock::time_point())
        options |= WNOHANG;
      if(-1 == ::waitid(P_PID, _processh.pid, &info, options))
        return make_errored_result<>(errno);
      if(info.si_signo == SIGCHLD)
      {
        ret = info.si_status;
        return false;
      }
      return true;
    };
    // Do an initial check, if no timeout was specified this will block until child is done
    {
      BOOST_OUTCOME_TRY(running, check_child());
      if(!running)
        return ret;
    }
#if 1
    // TODO FIXME: Implement timed waits for child processes to exit
    return make_errored_result<intptr_t>(EOPNOTSUPP);
#else
    // If he specified a timeout, we now have considerable work to do in order to do this race free
    // Firstly install a signal handler for SIGCHLD
    std::atomic<bool> child_exited(false);
    auto signal_handler=[&child_exited](int sig) { child_exited = true; }  // FIXME we might get the signal for a different pid to ours for another wait on another thread
    
    // Check our child process hasn't exited in the meantime
    
    // Now do a timed wait until a SIGCHLD comes through
    sigset_t set;
    siginfo_t siginfo;
    struct timespec ts_, *ts = nullptr;

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    if(d != stl11::chrono::steady_clock::time_point())
    {
      auto timeout = (stl11::chrono::steady_clock::now() > d) ? (DWORD) stl11::chrono::duration_cast<stl11::chrono::nanoseconds>(stl11::chrono::steady_clock::now() - d).count() : 0;
      ts_.tv_sec = timeout / 1000000000ULL;
      ts_.tv_nsec = timeout % 1000000000ULL;
      ts = &ts_;
    }
    int signal;
    do
    {
      signal = sigtimedwait(&set, &siginfo, ts);
    } while(signal < 0 && errno == EINTR);
    if(signal < 0)
    {
      if(EAGAIN == errno && d != stl11::chrono::steady_clock::time_point())
        return make_errored_result<intptr_t>(ETIMEDOUT);
      return make_errored_result<intptr_t>(errno);
    }
#endif
  }

  stl1z::filesystem::path current_process_path()
  {
    char buffer[PATH_MAX+1];
#ifdef __linux__
    // Read what the symbolic link at /proc/self/exe points at
    ssize_t len = ::readlink("/proc/self/exe", buffer, PATH_MAX);
    if(len > 0)
    {
      buffer[len] = 0;
      return stl1z::filesystem::path::string_type(buffer, len);
    }
#elif defined(__FreeBSD__)
    int mib[4]={CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, getpid()};
    size_t len = PATH_MAX;
    int ret = sysctl(mib, 4, buffer, &len, NULL, 0);
    if(ret > 0)
    {
      buffer[len] = 0;
      return stl1z::filesystem::path::string_type(buffer, len);
    }
#else
#error Unknown platform
#endif
    fprintf(stderr, "FATAL: child_process::current_process_path() failed with code %d\n", errno);
    abort();
  }

  std::map<stl1z::filesystem::path::string_type, stl1z::filesystem::path::string_type> current_process_env()
  {
#ifdef __linux__
    char **environ = __environ;
#endif
    std::map<stl1z::filesystem::path::string_type, stl1z::filesystem::path::string_type> ret;
    for(char **env = environ; *env; ++env)
    {
      char *equals = strchr(*env, '=');
      char *end = strchr(*env, 0);
      if(!equals) equals = end;
      ret.insert(std::make_pair(stl1z::filesystem::path::string_type(*env, equals - *env), stl1z::filesystem::path::string_type(equals + 1, end - equals)));
    }
    return ret;
  }

}

BOOST_KERNELTEST_V1_NAMESPACE_END
