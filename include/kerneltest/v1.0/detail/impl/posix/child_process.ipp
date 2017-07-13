/* Routines for handling child processes
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (9 commits)
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

#include "../../../child_process.hpp"

#include <limits.h>
#include <fcntl.h>
#include <signal.h>  // for siginfo_t
#include <spawn.h>
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
    if(_processh)
    {
      (void) wait();
    }
    _deinitialise_files();
    _deinitialise_streams();
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
#if 0
    ret._processh.fd = ::fork();
    if(0 == ret._processh.fd)
    {
      // I am the child
      if(-1 == ::dup2(childreadh.fd, STDIN_FILENO))
      {
        ::perror("dup2 readh");
        ::exit(1);
      }
      ::close(childreadh.fd);
      if(-1 == ::dup2(childwriteh.fd, STDOUT_FILENO))
      {
        ::perror("dup2 writeh");
        ::exit(1);
      }
      ::close(childwriteh.fd);
      if(!use_parent_errh)
      {
        if(-1 == ::dup2(childerrh.fd, STDERR_FILENO))
        {
          ::perror("dup2 errh");
          ::exit(1);
        }
        ::close(childerrh.fd);
      }
      if(-1 == ::execve(ret._path.c_str(), (char **) argptrs.data(), (char **) envptrs.data()))
      {
        ::perror("execve");
        ::exit(1);
      }
    }
    if(-1 == ret._processh.fd)
      return make_errored_result<child_process>(errno);
#else
    posix_spawn_file_actions_t child_fd_actions;
    int err = ::posix_spawn_file_actions_init(&child_fd_actions);
    if(err)
      return make_errored_result<child_process>(err);
    err = ::posix_spawn_file_actions_adddup2(&child_fd_actions, childreadh.fd, STDIN_FILENO);
    if(err)
      return make_errored_result<child_process>(err);
    err = ::posix_spawn_file_actions_addclose(&child_fd_actions, childreadh.fd);
    if(err)
      return make_errored_result<child_process>(err);
    err = ::posix_spawn_file_actions_adddup2(&child_fd_actions, childwriteh.fd, STDOUT_FILENO);
    if(err)
      return make_errored_result<child_process>(err);
    err = ::posix_spawn_file_actions_addclose(&child_fd_actions, childwriteh.fd);
    if(err)
      return make_errored_result<child_process>(err);
    if(!use_parent_errh)
    {
      err = ::posix_spawn_file_actions_adddup2(&child_fd_actions, childerrh.fd, STDERR_FILENO);
      if(err)
        return make_errored_result<child_process>(err);
      err = ::posix_spawn_file_actions_addclose(&child_fd_actions, childerrh.fd);
      if(err)
        return make_errored_result<child_process>(err);
    }
    err = ::posix_spawn(&ret._processh.pid, ret._path.c_str(), &child_fd_actions, nullptr, (char **) argptrs.data(), (char **) envptrs.data());
    if(err)
      return make_errored_result<child_process>(err);    
    auto unfdactions = undoer([&] {
      ::posix_spawn_file_actions_destroy(&child_fd_actions);
    });
#endif
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
    // TODO FIXME: Implement proper timed waits for child processes to exit
    do
    {
      // If timeout is not set, this will block forever
      BOOST_OUTCOME_TRY(running, check_child());
      if(!running)
        return ret;
      stl11::this_thread::sleep_for(stl11::chrono::milliseconds(100));
    } while(d == stl11::chrono::steady_clock::time_point() || stl11::chrono::steady_clock::now() < d);
    return make_errored_result<intptr_t>(ETIMEDOUT);
#if 0
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
