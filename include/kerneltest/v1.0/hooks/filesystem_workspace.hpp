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

#include "../quickcpplib/include/algorithm/string.hpp"
#include "../quickcpplib/include/utils/thread.hpp"

#include <fstream>
#include <mutex>
#include <unordered_map>

KERNELTEST_V1_NAMESPACE_BEGIN

namespace hooks
{
  namespace filesystem_setup_impl
  {
    //! Record the current working directory and store it
    static inline const filesystem::path &starting_path()
    {
      static filesystem::path p = filesystem::current_path();
      return p;
    }
    static inline filesystem::path _has_product(filesystem::path dir, const std::string &product)
    {
      if(filesystem::exists(dir / product))
        return dir / product;
      if(filesystem::exists(dir / ("boost." + product)))
        return dir / ("boost." + product);
      return filesystem::path();
    }

    struct library_directory_storage
    {
      std::unique_lock<std::mutex> lock;
      filesystem::path &path;
      library_directory_storage(std::unique_lock<std::mutex> &&_lock, filesystem::path &_path)
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
      static filesystem::path ret;
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
    template <bool is_throwing = false> inline filesystem::path library_directory(const char *__product = current_test_kernel.product)  // noexcept(!is_throwing)
    {
      try
      {
        static std::string product;
        auto ret = override_library_directory();
        if(__product == product)
          return ret.path;
        filesystem::path library_dir = starting_path();

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
        filesystem::path temp;
        for(;;)
        {
          temp = _has_product(library_dir, _product);
          if(!temp.empty() && filesystem::exists(temp / "test" / "tests"))
          {
            ret.path = temp;
            product = _product;
            return ret.path;
          }
          if(library_dir.native().size() > 3)
            library_dir = filesystem::canonical(library_dir / "..");
          else
            break;
        }
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the product lives");
        else
        {
          KERNELTEST_CERR("FATAL: Couldn't figure out where the product " << _product << " lives. You need a " << _product << " directory somewhere in or above the directory you run the tests from." << std::endl);
          std::terminate();
        }
      }
      catch(...)
      {
        if(!is_throwing)
        {
          KERNELTEST_CERR("library_directory() unexpectedly failed" << std::endl);
          std::terminate();
        }
        else
          throw;
      }
    }

    /*! Figure out an absolute path to the correct test workspace template. Uses
    library_directory() for the base of the product and assumes any test workspace
    templates live in product/test/tests.

    \tparam is_throwing If true, throw exceptions for any errors encountered,
    else print a useful message to KERNELTEST_CERR() and terminate the
    process.
    */
    template <bool is_throwing = false> inline filesystem::path workspace_template_path(const filesystem::path &workspace)  // noexcept(!is_throwing)
    {
      try
      {
        filesystem::path library_dir = library_directory();
        if(filesystem::exists(library_dir / "test" / "tests" / workspace))
        {
          return library_dir / "test" / "tests" / workspace;
        }
        // The final directory is allowed to not exist
        auto workspace2 = workspace.parent_path();
        if(filesystem::exists(library_dir / "test" / "tests" / workspace2))
        {
          return library_dir / "test" / "tests" / workspace;
        }
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the test workspace templates live");
        else
        {
          KERNELTEST_CERR("FATAL: Couldn't figure out where the test workspace templates live for test " << workspace << ". Product source directory is thought to be " << library_dir << std::endl);
          std::terminate();
        }
      }
      catch(...)
      {
        if(!is_throwing)
        {
          KERNELTEST_CERR("workspace_template_path() unexpectedly failed" << std::endl);
          std::terminate();
        }
        else
          throw;
      }
    }

    template <bool is_throwing, class Parent, class RetType> struct impl
    {
      filesystem::path _current;

      void _remove_workspace()  // noexcept(!is_throwing)
      {
        std::error_code ec;
        auto begin = std::chrono::steady_clock::now();
        do
        {
          ec.clear();
          bool exists = filesystem::exists(_current, ec);
          if(!exists && (!ec || ec == std::errc::no_such_file_or_directory))
            return;
          filesystem::remove_all(_current, ec);
        } while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() < 5);
        if(is_throwing)
          throw std::runtime_error("Couldn't delete workspace after five seconds of trying");
        KERNELTEST_CERR("FATAL: Couldn't delete " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl);
        std::terminate();
      }

      impl(Parent *, RetType &, size_t, filesystem::path &&workspace)  // noexcept(!is_throwing)
      {
        auto template_path = workspace_template_path<is_throwing>(workspace);
        // Make the workspace we choose unique to this thread
        _current = starting_path() / ("kerneltest_workspace_" + std::to_string(QUICKCPPLIB_NAMESPACE::utils::thread::this_thread_id()));
        // Clear out any stale workspace with the same name at this path just in case
        _remove_workspace();

        std::error_code ec;
        auto fatalexit = [&] {
          if(is_throwing)
            throw std::system_error(ec);
          KERNELTEST_CERR("FATAL: Couldn't copy " << template_path << " to " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl);
          std::terminate();
        };
        // Is the input workspace no workspace? In which case create an empty directory
        bool exists = filesystem::exists(template_path, ec);
        if(ec && ec != std::errc::no_such_file_or_directory)
          fatalexit();
        if(!exists)
        {
          filesystem::create_directory(_current, ec);
          if(ec)
            fatalexit();
        }
        else
        {
          auto begin = std::chrono::steady_clock::now();
          do
          {
            filesystem::copy(template_path, _current, filesystem::copy_options::recursive, ec);
            if(!ec)
              break;
          } while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() < 5);
          if(ec)
            fatalexit();
        }
        // Set the working directory to the newly configured workspace
        filesystem::current_path(_current);
        current_test_kernel.working_directory = _current.c_str();
      }
      constexpr impl(impl &&) noexcept = default;
      impl(const impl &) = delete;
      ~impl() noexcept(!is_throwing)
      {
        if(!_current.empty())
        {
          current_test_kernel.working_directory = nullptr;
          filesystem::current_path(starting_path());
          _remove_workspace();
        }
      }
    };
    template <bool is_throwing> struct inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const { return impl<is_throwing, Parent, RetType>(parent, testret, idx, filesystem::path(workspacebase) / workspace); }
      std::string print(const char *workspace) const { return std::string("precondition ") + workspace; }
    };
  }
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
  template <bool is_throwing = false> constexpr inline auto filesystem_setup(const char *workspacebase = current_test_kernel.test) { return filesystem_setup_impl::inst<is_throwing>{workspacebase}; }

  namespace filesystem_comparison_impl
  {
    //! Walk a directory hierarchy, depth first. f(directory_entry) can return something to early exit.
    template <class U> inline auto depth_first_walk(filesystem::path path, U &&f) -> decltype(f(std::declval<filesystem::directory_entry>()))
    {
      if(filesystem::exists(path))
      {
        for(filesystem::directory_iterator it(path); it != filesystem::directory_iterator(); ++it)
        {
          if(filesystem::is_directory(it->status()))
          {
            auto ret(depth_first_walk(it->path(), std::forward<U>(f)));
            if(ret)
              return ret;
          }
        }
        for(filesystem::directory_iterator it(path); it != filesystem::directory_iterator(); ++it)
        {
          if(!filesystem::is_directory(it->status()))
          {
            auto ret(f(*it));
            if(ret)
              return ret;
          }
        }
      }
      // Return default constructed edition of the type returned by the callable
      return decltype(f(std::declval<filesystem::directory_entry>()))();
    }
    /*! Compare two directories for equivalence, returning empty result if identical, else
    path of first differing item.
    */
    template <bool compare_contents, bool compare_timestamps> optional<result<filesystem::path>> compare_directories(filesystem::path before, filesystem::path after) noexcept
    {
      try
      {
        // Make list of everything in after
        std::unordered_map<filesystem::path, filesystem::directory_entry, path_hasher> after_items;
        depth_first_walk(after, [&](filesystem::directory_entry dirent) -> int {
          after_items[dirent.path()] = std::move(dirent);
          return 0;
        });

        // We need to remove each item as we check, if anything remains we fail
        optional<result<filesystem::path>> ret = depth_first_walk(before, [&](filesystem::directory_entry dirent) -> optional<result<filesystem::path>> {
          try
          {
            filesystem::path leafpath(dirent.path().native().substr(before.native().size() + 1));
            filesystem::path afterpath(after / leafpath);
            if(filesystem::is_symlink(dirent.symlink_status()))
            {
              if(filesystem::is_symlink(dirent.symlink_status()) != filesystem::is_symlink(filesystem::symlink_status(afterpath)))
                goto differs;
              if(filesystem::read_symlink(dirent.path()) != filesystem::read_symlink(afterpath))
                goto differs;
            }
            {
              auto beforestatus = dirent.status(), afterstatus = after_items[afterpath].status();
              if(filesystem::is_directory(beforestatus) != filesystem::is_directory(afterstatus))
                goto differs;
              if(filesystem::is_regular_file(beforestatus) != filesystem::is_regular_file(afterstatus))
                goto differs;
              if(filesystem::file_size(dirent.path()) != filesystem::file_size(afterpath))
                goto differs;
              if(compare_timestamps)
              {
                if(beforestatus.permissions() != afterstatus.permissions())
                  goto differs;
                if(filesystem::last_write_time(dirent.path()) != filesystem::last_write_time(afterpath))
                  goto differs;
              }
            }
            if(compare_contents)
            {
              std::ifstream beforeh(dirent.path()), afterh(afterpath);
              char beforeb[16384], afterb[16384];
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
          catch(...)
          {
            return {error_from_exception()};
          }
        });
        // If anything different, return that
        if(ret)
          return ret;
        // If anything in after not in current, return that
        if(!after_items.empty())
          return {success(after_items.begin()->first)};
        // Otherwise both current and after are identical
        return {};
      }
      catch(...)
      {
        return {error_from_exception()};
      }
    }

    template <class Parent, class RetType> struct structure_impl
    {
      Parent *parent;
      RetType &testret;
      size_t idx;
      filesystem::path model_workspace;
      structure_impl(Parent *_parent, RetType &_testret, size_t _idx, const char *workspacebase, const char *workspace)
          : parent(_parent)
          , testret(_testret)
          , idx(_idx)
          , model_workspace(filesystem_setup_impl::workspace_template_path(filesystem::path(workspacebase) / workspace))
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
            KERNELTEST_CERR("FATAL: There appears to be no hooks::filesystem_setup earlier in the hook sequence, therefore I have no workspace to compare to." << std::endl);
            std::terminate();
          }
          // Only do comparison if test passed
          if(testret)
          {
            // If this is empty, workspaces are identical
            optional<result<filesystem::path>> workspaces_not_identical = compare_directories<false, false>(current_test_kernel.working_directory, model_workspace);
            if(workspaces_not_identical)
            {
              // Propagate any error
              if(workspaces_not_identical->has_error())
                testret = {make_error_code(kerneltest_errc::filesystem_comparison_internal_failure)};  //, workspaces_not_identical->error().message().c_str(), workspaces_not_identical->error().value()
              // Set error with extended message of the path which differs
              else if(workspaces_not_identical->has_value())
                testret = {make_error_code(kerneltest_errc::filesystem_comparison_failed)};  // , workspaces_not_identical->value().string().c_str()
            }
          }
        }
      }
    };
    struct structure_inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const { return structure_impl<Parent, RetType>(parent, testret, idx, workspacebase, workspace); }
      std::string print(const char *workspace) const { return std::string("postcondition ") + workspace; }
    };
  }
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
  constexpr inline auto filesystem_comparison_structure(const char *workspacebase = current_test_kernel.test) { return filesystem_comparison_impl::structure_inst{workspacebase}; }
}

//! Alias hooks to precondition
namespace precondition = hooks;
//! Alias hooks to postcondition
namespace postcondition = hooks;

KERNELTEST_V1_NAMESPACE_END

#endif
