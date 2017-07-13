/* Routines for handling child processes
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
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

extern "C" __declspec(dllimport) errno_t rand_s(unsigned *random);

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

namespace child_process
{
  child_process::~child_process()
  {
    wait();
    if(_stdin || _cin)
    {
      // Handles are already closed, no need to do so again
      _readh.h = nullptr;
      _writeh.h = nullptr;
      _errh.h = nullptr;
    }
    _deinitialise_files();
    _deinitialise_streams();
    if(_processh)
    {
      CloseHandle(_processh.h);
      _processh.h = nullptr;
    }
    if(_readh)
    {
      CloseHandle(_readh.h);
      _readh.h = nullptr;
    }
    if(_writeh)
    {
      CloseHandle(_writeh.h);
      _writeh.h = nullptr;
    }
    if(_errh)
    {
      if(!_use_parent_errh)
        CloseHandle(_errh.h);
      _errh.h = nullptr;
    }
  }

  BOOST_KERNELTEST_HEADERS_ONLY_MEMFUNC_SPEC result<child_process> child_process::launch(stl1z::filesystem::path __path, std::vector<stl1z::filesystem::path::string_type> __args, std::map<stl1z::filesystem::path::string_type, stl1z::filesystem::path::string_type> __env, bool use_parent_errh) noexcept
  {
    using string_type = stl1z::filesystem::path::string_type;
    using char_type = string_type::value_type;
    child_process ret(std::move(__path), use_parent_errh, std::move(__args), std::move(__env));
    native_handle_type childreadh, childwriteh, childerrh;

    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    if(!CreatePipe(&childreadh.h, &ret._readh.h, nullptr, 0))
      return make_errored_result<child_process>(GetLastError());
    if(!CreatePipe(&ret._writeh.h, &childwriteh.h, nullptr, 0))
      return make_errored_result<child_process>(GetLastError());

    if(use_parent_errh)
    {
      childerrh.h = GetStdHandle(STD_ERROR_HANDLE);
    }
    else
    {
      // stderr needs to not be buffered
      /*if(!CreatePipe(&ret._errh.h, &childerrh.h, nullptr, 0))
      return make_errored_result<child_process>(GetLastError());
      */
      char randomname[] = "\\\\.\\pipe\\pipename";
      rand_s((unsigned *) (randomname + 8));
      rand_s((unsigned *) (randomname + 12));
      randomname[16] = randomname[8];
      randomname[8] = '\\';
      ret._errh.h = CreateNamedPipeA(randomname, PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_WRITE_THROUGH, PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);
      if(!ret._errh.h)
        return make_errored_result<child_process>(GetLastError());
      childerrh.h = CreateFileA(randomname, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, nullptr);
      if(!childerrh.h)
        return make_errored_result<child_process>(GetLastError());
    }

    auto unmypipes = undoer([&] {
      CloseHandle(ret._readh.h);
      ret._readh.h = nullptr;
      CloseHandle(ret._writeh.h);
      ret._writeh.h = nullptr;
      if(!use_parent_errh)
      {
        CloseHandle(ret._errh.h);
        ret._errh.h = nullptr;
      }
    });
    auto unhispipes = undoer([&] {
      CloseHandle(childreadh.h);
      CloseHandle(childwriteh.h);
      if(!use_parent_errh)
        CloseHandle(childerrh.h);
    });

    si.hStdInput = childreadh.h;
    si.hStdOutput = childwriteh.h;
    si.hStdError = childerrh.h;
    if(!SetHandleInformation(si.hStdInput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
      return make_errored_result<child_process>(GetLastError());
    if(!SetHandleInformation(si.hStdOutput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
      return make_errored_result<child_process>(GetLastError());
    if(!SetHandleInformation(si.hStdError, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
      return make_errored_result<child_process>(GetLastError());

    PROCESS_INFORMATION pi;
    char_type argsbuffer[32768], *argsbuffere = argsbuffer;
    // First copy in the executable path
    *argsbuffere++ = '"';
    memcpy(argsbuffere, ret._path.c_str(), sizeof(char_type) * ret._path.native().size());
    argsbuffere += ret._path.native().size();
    *argsbuffere++ = '"';
    *argsbuffere++ = ' ';
    for(auto &arg : ret._args)
    {
      if(argsbuffere - argsbuffer + arg.size() + 1 >= 32767)
        return make_errored_result<child_process>(EOVERFLOW);
      memcpy(argsbuffere, arg.data(), sizeof(char_type) * arg.size());
      argsbuffere += arg.size();
      *argsbuffere++ = ' ';
    }
    *(--argsbuffere) = 0;
    char_type envbuffer[32768], *envbuffere = envbuffer;
    for(auto &env : ret._env)
    {
      if(envbuffere - envbuffer + env.first.size() + env.second.size() + 2 >= 32767)
        return make_errored_result<child_process>(EOVERFLOW);
      memcpy(envbuffere, env.first.data(), sizeof(char_type) * env.first.size());
      envbuffere += env.first.size();
      *envbuffere++ = '=';
      memcpy(envbuffere, env.second.data(), sizeof(char_type) * (1 + env.second.size()));
      envbuffere += env.second.size() + 1;
    }
    *envbuffere = 0;
    if(!CreateProcess(ret._path.c_str(), argsbuffer, nullptr, nullptr, true, CREATE_UNICODE_ENVIRONMENT, envbuffer, nullptr, &si, &pi))
      return make_errored_result<child_process>(GetLastError());
    ret._processh.h = pi.hProcess;
    unmypipes.dismiss();

    // Wait until the primary thread has launched
    Sleep(30);

    // Close handles I no longer need
    CloseHandle(pi.hThread);
    return std::move(ret);
  }

  bool child_process::is_running() const noexcept
  {
    DWORD retcode = 0;
    if(!GetExitCodeProcess(_processh.h, &retcode))
      return false;
    return retcode == STILL_ACTIVE;
  }

  result<intptr_t> child_process::wait_until(stl11::chrono::steady_clock::time_point d) noexcept
  {
    if(!_processh.h)
      return -1;
    DWORD timeout = INFINITE;
    if(d != stl11::chrono::steady_clock::time_point())
    {
      timeout = (stl11::chrono::steady_clock::now() < d) ? (DWORD) stl11::chrono::duration_cast<stl11::chrono::milliseconds>(d - stl11::chrono::steady_clock::now()).count() : 0;
    }
    DWORD ret = WaitForSingleObject(_processh.h, timeout);
    if(WAIT_TIMEOUT == ret)
      return make_errored_result<intptr_t>(ETIMEDOUT);
    if(WAIT_OBJECT_0 != ret)
      return make_errored_result<intptr_t>(GetLastError());
    DWORD retcode = 0;
    if(!GetExitCodeProcess(_processh.h, &retcode))
      return make_errored_result<intptr_t>(GetLastError());
    return (intptr_t) retcode;
  }

  stl1z::filesystem::path current_process_path()
  {
    stl1z::filesystem::path::string_type buffer(32768, 0);
    DWORD len = GetModuleFileName(nullptr, const_cast<stl1z::filesystem::path::string_type::value_type *>(buffer.data()), (DWORD) buffer.size());
    if(!len)
      throw std::system_error(GetLastError(), std::system_category());
    buffer.resize(len);
    return stl1z::filesystem::path(std::move(buffer));
  }

  std::map<stl1z::filesystem::path::string_type, stl1z::filesystem::path::string_type> current_process_env()
  {
    using string_type = stl1z::filesystem::path::string_type;
    std::map<string_type, string_type> ret;
    string_type::value_type *strings = GetEnvironmentStrings();
    auto unstrings = undoer([strings] { FreeEnvironmentStrings(strings); });
    for(auto *s = strings, *e = strings; *s; s = (e = e + 1))
    {
      auto *c = s;
      for(c = nullptr; *e; e++)
      {
        if(!c && *e == '=')
          c = e;
      }
      if(*s != '=')
        ret.insert(std::make_pair(string_type(s, c - s), string_type(c + 1, e - c - 1)));
    }
    return ret;
  }
}

BOOST_KERNELTEST_V1_NAMESPACE_END
