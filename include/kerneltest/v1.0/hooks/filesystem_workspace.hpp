/* Filesystem workspace test kernel hooks
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
File Created: May 2016


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

#include "../config.hpp"

#ifndef KERNELTEST_HOOKS_FILESYSTEM_WORKSPACE_HPP
#define KERNELTEST_HOOKS_FILESYSTEM_WORKSPACE_HPP

#include "quickcpplib/algorithm/string.hpp"
#include "quickcpplib/utils/thread.hpp"

#include <cstring>  // memcmp/memset are used below (this header is not self-contained without it)
#include <fstream>
#include <map>
#include <mutex>
#include <unordered_map>

#ifdef _WIN32
#include <winioctl.h>  // for FSCTL_GET_REPARSE_POINT
#endif

KERNELTEST_V1_NAMESPACE_BEGIN

namespace hooks
{
  namespace filesystem_setup_impl
  {
    //! Record the current working directory and store it
    static inline const std::filesystem::path &starting_path()
    {
      static std::filesystem::path p = std::filesystem::current_path();
      return p;
    }
    static inline std::filesystem::path _has_product(std::filesystem::path dir, const std::string &product)
    {
      if(std::filesystem::exists(dir / product))
        return dir / product;
      if(std::filesystem::exists(dir / ("boost." + product)))
        return dir / ("boost." + product);
      return std::filesystem::path();
    }

    struct library_directory_storage
    {
      std::unique_lock<std::mutex> lock;
      std::filesystem::path &path;
      library_directory_storage(std::unique_lock<std::mutex> &&_lock, std::filesystem::path &_path)
          : lock(std::move(_lock))
          , path(_path)
      {
      }
    };
    /*! You can override the library directory chosen by calling library_directory(product)
    and then call this function, setting library_directory_storage.path to the new directory.
    Note that library_directory_storage holds a mutex to the directory storage and will
    therefore deadlock all other users until it is destroyed.
    */
    inline library_directory_storage override_library_directory()
    {
      static std::mutex lock;
      static std::filesystem::path ret;
      return library_directory_storage(std::unique_lock<std::mutex>(lock), ret);
    }
    /*! Figure out an absolute path to the base of the product's directory
    and cache it for later fast returns. Changing the product from the
    cached value will recalculate the path.

    The environment variable KERNELTEST_product_HOME is first checked,
    only if it doesn't exist the working directory is checked for a directory
    called product and every directory up the hierarchy until the root of the
    drive.
    \tparam is_throwing If true, throw exceptions for any errors encountered,
    else print a useful message to KERNELTEST_CERR() and terminate the
    process.
    */
    template <bool is_throwing = false>
    inline std::filesystem::path library_directory(const char *__product = current_test_kernel.product)  // noexcept(!is_throwing)
    {
      KERNELTEST_EXCEPTION_TRY
      {
        static std::string product;
        auto ret = override_library_directory();
        if(__product == product)
          return ret.path;
        std::filesystem::path library_dir = starting_path();

        // Is there an environment variable KERNELTEST_product_HOME?
        std::string _product(__product);
        std::string envkey("KERNELTEST_" + QUICKCPPLIB_NAMESPACE::algorithm::string::toupper(_product) + "_HOME");
#ifdef _MSC_VER
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma warning(push)
#pragma warning(disable : 4996)  // Stupid deprecation warning
#endif
#endif
#ifdef _UNICODE
        std::wstring _envkey;
        for(auto &i : envkey)
          _envkey.push_back(i);
        auto env = _wgetenv(_envkey.c_str());
#else
        auto env = getenv(envkey.c_str());
#endif
#ifdef _MSC_VER
#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma warning(pop)
#endif
#endif
        if(env)
        {
          ret.path.assign(env);
          product = _product;
          return ret.path;
        }

        // If no environment variable, start searching from the current working directory
        // Layout is <boost.afio>/test/tests/<test_name>/<workspace_templates>
        // We must also account for an out-of-tree build
        std::filesystem::path temp;
        for(;;)
        {
          temp = _has_product(library_dir, _product);
          if(!temp.empty() && std::filesystem::exists(temp / "test" / "tests"))
          {
            ret.path = temp;
            product = _product;
            return ret.path;
          }
          if(library_dir.native().size() > 3)
            library_dir = std::filesystem::canonical(library_dir / "..");
          else
            break;
        }
#ifdef __cpp_exceptions
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the product lives");
        else
#endif
        {
          KERNELTEST_CERR("FATAL: Couldn't figure out where the product " << _product << " lives. You need a " << _product
                                                                          << " directory somewhere in or above the directory you run the tests from."
                                                                          << std::endl);
          std::terminate();
        }
      }
      KERNELTEST_EXCEPTION_CATCH_ALL
      {
#ifdef __cpp_exceptions
        if(!is_throwing)
#endif
        {
          KERNELTEST_CERR("library_directory() unexpectedly failed" << std::endl);
          std::terminate();
        }
#ifdef __cpp_exceptions
        else
          throw;
#endif
      }
    }

    /*! Figure out an absolute path to the correct test workspace template. Uses
    library_directory() for the base of the product and assumes any test workspace
    templates live in product/test/tests.

    \tparam is_throwing If true, throw exceptions for any errors encountered,
    else print a useful message to KERNELTEST_CERR() and terminate the
    process.
    */
    template <bool is_throwing = false> inline std::filesystem::path workspace_template_path(const std::filesystem::path &workspace)  // noexcept(!is_throwing)
    {
      KERNELTEST_EXCEPTION_TRY
      {
        std::filesystem::path library_dir = library_directory();
        if(std::filesystem::exists(library_dir / "test" / "tests" / workspace))
        {
          return library_dir / "test" / "tests" / workspace;
        }
        // The final directory is allowed to not exist
        auto workspace2 = workspace.parent_path();
        if(std::filesystem::exists(library_dir / "test" / "tests" / workspace2))
        {
          return library_dir / "test" / "tests" / workspace;
        }
#ifdef __cpp_exceptions
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the test workspace templates live");
        else
#endif
        {
          KERNELTEST_CERR("FATAL: Couldn't figure out where the test workspace templates live for test "
                          << workspace << ". Product source directory is thought to be " << library_dir << std::endl);
          std::terminate();
        }
      }
      KERNELTEST_EXCEPTION_CATCH_ALL
      {
#ifdef __cpp_exceptions
        if(!is_throwing)
#endif
        {
          KERNELTEST_CERR("workspace_template_path() unexpectedly failed" << std::endl);
          std::terminate();
        }
#ifdef __cpp_exceptions
        else
          KERNELTEST_EXCEPTION_RETHROW;
#endif
      }
    }

    template <bool is_throwing, class Parent, class RetType> struct impl
    {
      std::filesystem::path _current;

      void _remove_workspace()  // noexcept(!is_throwing)
      {
        std::error_code ec;
        auto begin = std::chrono::steady_clock::now();
        do
        {
          ec.clear();
          bool exists = std::filesystem::exists(_current, ec);
          if(!exists && (!ec || ec == std::errc::no_such_file_or_directory))
            return;
          std::filesystem::remove_all(_current, ec);
        } while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() < 5);
#ifdef __cpp_exceptions
        if(is_throwing)
          throw std::runtime_error("Couldn't delete workspace after five seconds of trying");
#endif
        KERNELTEST_CERR("FATAL: Couldn't delete " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl);
        std::terminate();
      }

      impl(Parent *, RetType &, size_t, std::filesystem::path &&workspace)  // noexcept(!is_throwing)
      {
        auto template_path = workspace_template_path<is_throwing>(workspace);
        // Make the workspace we choose unique to this thread
        _current = starting_path() / ("kerneltest_workspace_" + std::to_string(QUICKCPPLIB_NAMESPACE::utils::thread::this_thread_id()));
        // Clear out any stale workspace with the same name at this path just in case
        _remove_workspace();

        std::error_code ec;
        auto fatalexit = [&]
        {
#ifdef __cpp_exceptions
          if(is_throwing)
            throw std::system_error(ec);
#endif
          KERNELTEST_CERR("FATAL: Couldn't copy " << template_path << " to " << _current << " due to " << ec.message() << " after five seconds of trying."
                                                  << std::endl);
          std::terminate();
        };
        // Is the input workspace no workspace? In which case create an empty directory
        bool exists = std::filesystem::exists(template_path, ec);
        if(ec && ec != std::errc::no_such_file_or_directory)
          fatalexit();
        if(!exists)
        {
          std::filesystem::create_directory(_current, ec);
          if(ec)
            fatalexit();
        }
        else
        {
          auto begin = std::chrono::steady_clock::now();
          do
          {
            // VS2017 still doesn't understand symlinks :(, so copy in the starting filesystem environment by hand
            struct _
            {
              static void copy_level(const std::filesystem::path &srcdir, const std::filesystem::path &destdir, std::error_code &ec)
              {
                for(std::filesystem::directory_iterator it(srcdir); it != std::filesystem::directory_iterator(); ++it)
                {
#ifdef _WIN32
                  typedef struct _REPARSE_DATA_BUFFER  // NOLINT
                  {
                    ULONG ReparseTag;
                    USHORT ReparseDataLength;
                    USHORT Reserved;
                    union
                    {
                      struct
                      {
                        USHORT SubstituteNameOffset;
                        USHORT SubstituteNameLength;
                        USHORT PrintNameOffset;
                        USHORT PrintNameLength;
                        ULONG Flags;
                        WCHAR PathBuffer[1];
                      } SymbolicLinkReparseBuffer;
                      struct
                      {
                        USHORT SubstituteNameOffset;
                        USHORT SubstituteNameLength;
                        USHORT PrintNameOffset;
                        USHORT PrintNameLength;
                        WCHAR PathBuffer[1];
                      } MountPointReparseBuffer;
                      struct
                      {
                        UCHAR DataBuffer[1];
                      } GenericReparseBuffer;
                    };
                  } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
                  bool is_symlink = false;
                  HANDLE h = CreateFileW(it->path().c_str(), SYNCHRONIZE | FILE_READ_ATTRIBUTES | STANDARD_RIGHTS_READ,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                                         FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
                  if(h == INVALID_HANDLE_VALUE)
                  {
                    ec = std::error_code(GetLastError(), std::system_category());
                    return;
                  }
                  TCHAR buffer[32769];
                  auto *rpd = (REPARSE_DATA_BUFFER *) buffer;
                  DWORD read = 0, written = 0;
                  if(DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, rpd, (DWORD) sizeof(buffer), &read, NULL))
                  {
                    is_symlink = true;
                    CloseHandle(h);
                    auto destpath = destdir / it->path().filename();
                    h = CreateFileW(destpath.c_str(), SYNCHRONIZE | FILE_READ_ATTRIBUTES | STANDARD_RIGHTS_READ | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);
                    if(h == INVALID_HANDLE_VALUE)
                    {
                      ec = std::error_code(GetLastError(), std::system_category());
                      return;
                    }
                    if(!DeviceIoControl(h, FSCTL_SET_REPARSE_POINT, rpd, read, NULL, 0, &written, NULL))
                    {
                      ec = std::error_code(GetLastError(), std::system_category());
                      CloseHandle(h);
                      return;
                    }
                  }
                  CloseHandle(h);
                  if(is_symlink)
                    continue;
#else
                  // symlink_status() does not follow the link, status() does: for a
                  // symlink to a directory status() says "directory" and the link
                  // was then recursed into, copying out-of-tree content into the
                  // workspace (and never terminating on a symlink loop).
                  if(std::filesystem::is_symlink(it->symlink_status()))
                  {
                    std::filesystem::copy_symlink(it->path(), destdir / it->path().filename(), ec);
                    if(ec)
                      return;
                  }
#endif
                  else if(std::filesystem::is_directory(it->status()))
                  {
                    std::filesystem::create_directory(destdir / it->path().filename(), ec);
                    ec.clear();
                    copy_level(it->path(), destdir / it->path().filename(), ec);
                    if(ec)
                      return;
                  }
                  else if(std::filesystem::is_regular_file(it->status()))
                  {
                    std::filesystem::copy_file(it->path(), destdir / it->path().filename(), ec);
                    if(ec)
                      return;
                  }
                }
              }
            };
            std::filesystem::create_directory(_current, ec);
            ec.clear();
            _::copy_level(template_path, _current, ec);
            if(!ec)
              break;
          } while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() < 5);
          if(ec)
            fatalexit();
        }
        // Set the working directory to the newly configured workspace
        std::filesystem::current_path(_current);
        current_test_kernel.working_directory = _current.c_str();
      }
      constexpr impl(impl &&) noexcept = default;
      impl(const impl &) = delete;
      ~impl() noexcept(!is_throwing)
      {
        if(!_current.empty())
        {
          current_test_kernel.working_directory = nullptr;
          std::filesystem::current_path(starting_path());
          _remove_workspace();
        }
      }
    };
    template <bool is_throwing> struct inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const
      {
        return impl<is_throwing, Parent, RetType>(parent, testret, idx, std::filesystem::path(workspacebase) / workspace);
      }
      std::string print(const char *workspace) const { return std::string("precondition ") + workspace; }
    };
  }  // namespace filesystem_setup_impl
  //! The parameters for the filesystem_setup hook
  using filesystem_setup_parameters = parameters<const char *>;
  /*! Kernel test hook setting up a workspace directory for the test to run inside and deleting it after.

  The working directory on first instantiation is assumed to be the correct place to put test workspaces
  each of which will be named after the unique thread id of the calling thread.
  The source of the workspace templates comes from `workspace_template_path()` which in turn derives from
  `library_directory()`.
  \tparam is_throwing If true, throw exceptions for any errors encountered,
  else print a useful message to KERNELTEST_CERR() and terminate the
  process.
  \return A type which when called configures the workspace and changes the working directory to that
  workspace, and on destruction deletes the workspace and changes the working directory back to `starting_path()`.
  `current_test_kernel.working_directory` is also set to the working directory.
  \param workspacebase A path fragment inside `test/tests` of the base of the workspaces to choose from.
  */
  template <bool is_throwing = false> constexpr inline auto filesystem_setup(const char *workspacebase = current_test_kernel.test)
  {
    return filesystem_setup_impl::inst<is_throwing>{workspacebase};
  }

  namespace filesystem_comparison_impl
  {
    //! Walk a directory hierarchy, depth first. f(directory_entry) can return something to early exit.
    template <class U> inline auto depth_first_walk(std::filesystem::path path, U &&f) -> decltype(f(std::declval<std::filesystem::directory_entry>()))
    {
      if(std::filesystem::exists(path))
      {
        for(std::filesystem::directory_iterator it(path); it != std::filesystem::directory_iterator(); ++it)
        {
          if(!std::filesystem::is_symlink(it->path()) /* work around bug in libstdc++ */ && std::filesystem::is_directory(it->status()))
          {
            auto ret(depth_first_walk(it->path(), std::forward<U>(f)));
            if(ret)
              return ret;
          }
        }
        for(std::filesystem::directory_iterator it(path); it != std::filesystem::directory_iterator(); ++it)
        {
          if(!std::filesystem::is_directory(it->status()))
          {
            auto ret(f(*it));
            if(ret)
              return ret;
          }
        }
      }
      // Return default constructed edition of the type returned by the callable
      return decltype(f(std::declval<std::filesystem::directory_entry>()))();
    }
    /*! Records the relative path of every directory reachable under `root`,
    mapped to the target of that entry if it is a symlink and to an empty path
    if it is a real directory (a symlinked directory is recorded but never
    descended into, so a cycle cannot be followed).

    `depth_first_walk()` only ever hands a NON-directory to its callable, which is
    why `compare_directories()` used to be blind to any difference which is a
    directory on its own: a directory present on one side only was invisible
    whenever it was empty, and `filesystem_comparison_structure()` reported two
    plainly different workspaces as identical.

    The value is the link target rather than a bool: recording only
    "is a symlink" made two workspaces whose directory symlinks point
    somewhere completely different compare as identical.
    */
    inline void collect_directories(const std::filesystem::path &root, const std::filesystem::path &rel,
                                    std::map<std::filesystem::path, std::filesystem::path> &out)
    {
      std::filesystem::path here(rel.empty() ? root : root / rel);
      if(!std::filesystem::exists(here))
        return;
      for(std::filesystem::directory_iterator it(here); it != std::filesystem::directory_iterator(); ++it)
      {
        // is_directory(status()) follows symlinks, so a link to a directory counts
        // as a directory here. That matches depth_first_walk()'s descent rule.
        if(!std::filesystem::is_directory(it->status()))
          continue;
        std::filesystem::path child(rel.empty() ? std::filesystem::path(it->path().filename()) : rel / it->path().filename());
        bool islink = std::filesystem::is_symlink(it->path());
        out[child] = islink ? std::filesystem::read_symlink(it->path()) : std::filesystem::path();
        if(!islink)
          collect_directories(root, child, out);
      }
    }
    /*! Compare two directories for equivalence, returning empty result if identical, else
    path of first differing item.
    */
    template <bool compare_contents, bool compare_timestamps>
    std::optional<result<std::filesystem::path>> compare_directories(std::filesystem::path before, std::filesystem::path after) noexcept
    {
      KERNELTEST_EXCEPTION_TRY
      {
        // A workspace which does not exist must never read as "identical". Both walks
        // below are driven off existence - depth_first_walk() returns immediately when
        // its path is absent, and collect_directories() early-returns the same way - so
        // a missing 'after' yields an empty item set on that side and the function used
        // to fall through to `return {};`, i.e. "the workspaces are identical". That is
        // reachable in production: workspace_template_path() deliberately returns a
        // nonexistent leaf when only its parent exists. structure_impl() maps any error
        // here onto filesystem_comparison_internal_failure, so the test goes red rather
        // than silently green.
        {
          std::error_code ec;
          const bool before_isdir = std::filesystem::is_directory(before, ec);
          const bool after_isdir = std::filesystem::is_directory(after, ec);
          if(!before_isdir || !after_isdir)
          {
            KERNELTEST_CERR("WARNING: KernelTest workspace comparison was given a "
                            << (before_isdir ? "model" : "workspace") << " path which is not a directory (" << before << " vs " << after
                            << "), refusing to call them identical" << std::endl);
            return {result<std::filesystem::path>(make_error_code(kerneltest_errc::filesystem_comparison_internal_failure))};
          }
        }
        // Make list of everything in after. std::hash<std::filesystem::path> is
        // guaranteed by C++17, which is why kerneltest's own path_hasher (written
        // for the filesystem TS, whose implementations did not provide one) is gone.
        std::unordered_map<std::filesystem::path, std::filesystem::directory_entry, std::hash<std::filesystem::path>> after_items;
        depth_first_walk(after,
                         [&](std::filesystem::directory_entry dirent) -> int
                         {
                           after_items[dirent.path()] = std::move(dirent);
                           return 0;
                         });

        // We need to remove each item as we check, if anything remains we fail
        std::optional<result<std::filesystem::path>> ret =
        depth_first_walk(before,
                         [&](std::filesystem::directory_entry dirent) -> std::optional<result<std::filesystem::path>>
                         {
                           KERNELTEST_EXCEPTION_TRY
                           {
                             std::filesystem::path leafpath(dirent.path().native().substr(before.native().size() + 1));
                             std::filesystem::path afterpath(after / leafpath);
#ifdef _WIN32
                             if((GetFileAttributesW(dirent.path().c_str()) & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
                             {
                               typedef struct _REPARSE_DATA_BUFFER  // NOLINT
                               {
                                 ULONG ReparseTag;
                                 USHORT ReparseDataLength;
                                 USHORT Reserved;
                                 union
                                 {
                                   struct
                                   {
                                     USHORT SubstituteNameOffset;
                                     USHORT SubstituteNameLength;
                                     USHORT PrintNameOffset;
                                     USHORT PrintNameLength;
                                     ULONG Flags;
                                     WCHAR PathBuffer[1];
                                   } SymbolicLinkReparseBuffer;
                                   struct
                                   {
                                     USHORT SubstituteNameOffset;
                                     USHORT SubstituteNameLength;
                                     USHORT PrintNameOffset;
                                     USHORT PrintNameLength;
                                     WCHAR PathBuffer[1];
                                   } MountPointReparseBuffer;
                                   struct
                                   {
                                     UCHAR DataBuffer[1];
                                   } GenericReparseBuffer;
                                 };
                               } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
                               TCHAR buffer1[32769], buffer2[32769];
                               memset(buffer1, 0, sizeof(buffer1));
                               memset(buffer2, 0, sizeof(buffer2));
                               auto *rpd1 = (REPARSE_DATA_BUFFER *) buffer1;
                               auto *rpd2 = (REPARSE_DATA_BUFFER *) buffer2;
                               DWORD read1 = 0, read2 = 0;
                               HANDLE h = CreateFileW(dirent.path().c_str(), SYNCHRONIZE | FILE_READ_ATTRIBUTES | STANDARD_RIGHTS_READ,
                                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
                               if(h != INVALID_HANDLE_VALUE)
                               {
                                 if(DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, rpd1, (DWORD) sizeof(buffer1), &read1, NULL))
                                 {
                                   CloseHandle(h);
                                 }
                                 else
                                 {
                                   CloseHandle(h);
                                   goto differs;
                                 }
                               }
                               h = CreateFileW(afterpath.c_str(), SYNCHRONIZE | FILE_READ_ATTRIBUTES | STANDARD_RIGHTS_READ,
                                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
                               if(h != INVALID_HANDLE_VALUE)
                               {
                                 if(DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, rpd2, (DWORD) sizeof(buffer2), &read2, NULL))
                                 {
                                   CloseHandle(h);
                                 }
                                 else
                                 {
                                   CloseHandle(h);
                                   goto differs;
                                 }
                               }
                               if(rpd1->ReparseTag != rpd2->ReparseTag)
                                 goto differs;
                               switch(rpd1->ReparseTag)
                               {
                               case IO_REPARSE_TAG_SYMLINK:
                                 if(rpd1->SymbolicLinkReparseBuffer.SubstituteNameLength != rpd2->SymbolicLinkReparseBuffer.SubstituteNameLength)
                                   goto differs;
                                 if(memcmp(rpd1->SymbolicLinkReparseBuffer.PathBuffer + rpd1->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t),
                                           rpd2->SymbolicLinkReparseBuffer.PathBuffer + rpd2->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t),
                                           rpd1->SymbolicLinkReparseBuffer.SubstituteNameLength) != 0)
                                   goto differs;
                                 break;
                               case IO_REPARSE_TAG_MOUNT_POINT:
                                 if(rpd1->MountPointReparseBuffer.SubstituteNameLength != rpd2->MountPointReparseBuffer.SubstituteNameLength)
                                   goto differs;
                                 if(memcmp(rpd1->MountPointReparseBuffer.PathBuffer + rpd1->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t),
                                           rpd2->MountPointReparseBuffer.PathBuffer + rpd2->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t),
                                           rpd1->MountPointReparseBuffer.SubstituteNameLength) != 0)
                                   goto differs;
                                 break;
                               }
                             }
#else
            {
              // Ask about the link itself (symlink_status), not about what it
              // points at (status), and compare BOTH sides: testing only the
              // before side meant a symlink present only in the model was then
              // compared as a regular file and matched on size alone.
              auto before_sym = dirent.symlink_status(), after_sym = std::filesystem::symlink_status(afterpath);
              if(std::filesystem::is_symlink(before_sym) || std::filesystem::is_symlink(after_sym))
              {
                if(std::filesystem::is_symlink(before_sym) != std::filesystem::is_symlink(after_sym))
                  goto differs;
                if(std::filesystem::read_symlink(dirent.path()) != std::filesystem::read_symlink(afterpath))
                  goto differs;
              }
            }
#endif
                             {
                               auto beforestatus = dirent.status(), afterstatus = after_items[afterpath].status();
                               if(std::filesystem::is_directory(beforestatus) != std::filesystem::is_directory(afterstatus))
                                 goto differs;
                               if(std::filesystem::is_regular_file(beforestatus) != std::filesystem::is_regular_file(afterstatus))
                                 goto differs;
                               if(std::filesystem::file_size(dirent.path()) != std::filesystem::file_size(afterpath))
                                 goto differs;
                               if(compare_timestamps)
                               {
                                 if(beforestatus.permissions() != afterstatus.permissions())
                                   goto differs;
                                 if(std::filesystem::last_write_time(dirent.path()) != std::filesystem::last_write_time(afterpath))
                                   goto differs;
                               }
                             }
                             if(compare_contents)
                             {
                               std::ifstream beforeh(dirent.path()), afterh(afterpath);
                               char beforeb[16384] = "", afterb[16384] = "";
                               do
                               {
                                 beforeh.read(beforeb, sizeof(beforeb));
                                 afterh.read(afterb, sizeof(afterb));
                                 if(memcmp(beforeb, afterb, sizeof(afterb)))
                                   goto differs;
                               } while(beforeh.good() && afterh.good());
                             }
                             // This item is identical
                             after_items.erase(afterpath);
                             return {};
                           differs:
                             return {success(leafpath)};
                           }
                           KERNELTEST_EXCEPTION_CATCH_ALL
                           {
                             return {error_from_exception()};
                           }
                         });
        // If anything different, return that
        if(ret)
        {
          if(*ret)
          {
            KERNELTEST_CERR("WARNING: KernelTest workspace comparison saw item differ " << ret->value() << std::endl);
          }
          else
          {
            KERNELTEST_CERR("WARNING: KernelTest workspace comparison saw error " << ret->error() << std::endl);
          }
          // exit(1);
          return ret;
        }
        // If anything in after not in current, return that
        if(!after_items.empty())
        {
          KERNELTEST_CERR("WARNING: KernelTest workspace comparison saw not present item " << after_items.begin()->first << std::endl);
          // exit(1);
          return {success(after_items.begin()->first)};
        }
        // Finally compare the directory ENTRIES themselves, both ways. Both walks
        // above only hand non-directories to their callable, so without this an
        // empty directory present on one side only was never compared at all.
        {
          std::map<std::filesystem::path, std::filesystem::path> before_dirs, after_dirs;
          collect_directories(before, std::filesystem::path(), before_dirs);
          collect_directories(after, std::filesystem::path(), after_dirs);
          for(const auto &i : before_dirs)
          {
            auto j = after_dirs.find(i.first);
            if(after_dirs.end() == j || j->second != i.second)
            {
              KERNELTEST_CERR("WARNING: KernelTest workspace comparison saw a differing directory " << i.first << std::endl);
              return {success(i.first)};
            }
          }
          for(const auto &i : after_dirs)
          {
            if(before_dirs.end() == before_dirs.find(i.first))
            {
              KERNELTEST_CERR("WARNING: KernelTest workspace comparison saw a directory present only in the model " << i.first << std::endl);
              return {success(i.first)};
            }
          }
        }
        // Otherwise both current and after are identical
        return {};
      }
      KERNELTEST_EXCEPTION_CATCH_ALL
      {
        return {error_from_exception()};
      }
    }

    template <class Parent, class RetType> struct structure_impl
    {
      Parent *parent;
      RetType &testret;
      size_t idx;
      std::filesystem::path model_workspace;
      structure_impl(Parent *_parent, RetType &_testret, size_t _idx, const char *workspacebase, const char *workspace)
          : parent(_parent)
          , testret(_testret)
          , idx(_idx)
          , model_workspace(filesystem_setup_impl::workspace_template_path(std::filesystem::path(workspacebase) / workspace))
      {
      }
      structure_impl(structure_impl &&) noexcept = default;
      structure_impl(const structure_impl &) = delete;
      ~structure_impl()
      {
        if(!model_workspace.empty())
        {
          if(!current_test_kernel.working_directory)
          {
            KERNELTEST_CERR("FATAL: There appears to be no hooks::filesystem_setup earlier in the hook sequence, therefore I have no workspace to compare to."
                            << std::endl);
            std::terminate();
          }
          // Only do comparison if test passed
          if(testret)
          {
            // If this is empty, workspaces are identical
            std::optional<result<std::filesystem::path>> workspaces_not_identical =
            compare_directories<false, false>(current_test_kernel.working_directory, model_workspace);
            if(workspaces_not_identical)
            {
              // Propagate any error
              if(workspaces_not_identical->has_error())
              {
                testret = RetType(typename RetType::value_type::error_type(
                make_error_code(kerneltest_errc::filesystem_comparison_internal_failure)));  //, workspaces_not_identical->error().message().c_str(),
                                                                                             // workspaces_not_identical->error().value()
              }
              // Set error with extended message of the path which differs
              else if(workspaces_not_identical->has_value())
                testret = RetType(typename RetType::value_type::error_type(
                make_error_code(kerneltest_errc::filesystem_comparison_failed)));  // , workspaces_not_identical->value().string().c_str()
            }
          }
        }
      }
    };
    struct structure_inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const
      {
        return structure_impl<Parent, RetType>(parent, testret, idx, workspacebase, workspace);
      }
      std::string print(const char *workspace) const { return std::string("postcondition ") + workspace; }
    };
  }  // namespace filesystem_comparison_impl
  //! The parameters for the filesystem_comparison_structure hook
  using filesystem_comparison_structure_parameters = parameters<const char *>;
  /*! Kernel test hook comparing the structure of the test kernel workspace after the test to a workspace template.

  This is the fastest method of filesystem comparison. The following differences are ignored:
   * Timestamps
   * Security and ACLs
   * File contents (but size is not ignored)

  \return A type which when called records the outcome for the test, and on destruction if the outcome
  is not errored compares the test's workspace with a model workspace template. If they do not
  match, the outcome is set to an appropriate errored state.
  \param workspacebase A path fragment inside `test/tests` of the base of the workspaces to choose from.
  */
  constexpr inline auto filesystem_comparison_structure(const char *workspacebase = current_test_kernel.test)
  {
    return filesystem_comparison_impl::structure_inst{workspacebase};
  }
}  // namespace hooks

//! Alias hooks to precondition
namespace precondition = hooks;
//! Alias hooks to postcondition
namespace postcondition = hooks;

KERNELTEST_V1_NAMESPACE_END

#endif
