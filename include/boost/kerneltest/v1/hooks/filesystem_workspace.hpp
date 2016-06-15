/* Filesystem workspace test kernel hooks
(C) 2016 Niall Douglas http://www.nedprod.com/
File Created: May 2016
*/

#include "../config.hpp"

#ifndef BOOST_KERNELTEST_HOOKS_FILESYSTEM_WORKSPACE_HPP
#define BOOST_KERNELTEST_HOOKS_FILESYSTEM_WORKSPACE_HPP

#include "../../boost-lite/include/algorithm/string.hpp"
#include "../../boost-lite/include/utils/thread.hpp"

#include <unordered_map>

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

namespace hooks
{
  namespace filesystem_setup_impl
  {
    //! Record the current working directory and store it
    static inline const stl1z::filesystem::path &starting_path()
    {
      static stl1z::filesystem::path p = stl1z::filesystem::current_path();
      return p;
    }
    static inline stl1z::filesystem::path _has_product(stl1z::filesystem::path dir, const std::string &product)
    {
      if(stl1z::filesystem::exists(dir / product))
        return dir / product;
      if(stl1z::filesystem::exists(dir / ("boost." + product)))
        return dir / ("boost." + product);
      return stl1z::filesystem::path();
    }

    struct library_directory_storage
    {
      std::unique_lock<std::mutex> lock;
      stl1z::filesystem::path &path;
      library_directory_storage(std::unique_lock<std::mutex> &&_lock, stl1z::filesystem::path &_path)
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
      static stl1z::filesystem::path ret;
      return library_directory_storage(std::unique_lock<std::mutex>(lock), ret);
    }
    /*! Figure out an absolute path to the base of the product's directory
    and cache it for later fast returns. Changing the product from the
    cached value will recalculate the path.

    The environment variable BOOST_KERNELTEST_product_HOME is first checked,
    only if it doesn't exist the working directory is checked for a directory
    called product and every directory up the hierarchy until the root of the
    drive.
    \tparam is_throwing If true, throw exceptions for any errors encountered,
    else print a useful message to BOOST_KERNELTEST_CERR() and terminate the
    process.
    */
    template <bool is_throwing = false> inline stl1z::filesystem::path library_directory(const char *__product = current_test_kernel.product) noexcept(!is_throwing)
    {
      try
      {
        static std::string product;
        auto ret = override_library_directory();
        if(__product == product)
          return ret.path;
        stl1z::filesystem::path library_dir = starting_path();

        // Is there an environment variable BOOST_KERNELTEST_product_HOME?
        std::string _product(__product);
        std::string envkey("BOOST_KERNELTEST_" + boost_lite::algorithm::string::toupper(_product) + "_HOME");
#ifdef _UNICODE
        std::wstring _envkey;
        for(auto &i : envkey)
          _envkey.push_back(i);
        auto env = _wgetenv(_envkey.c_str());
#else
        auto env = getenv(envkey.c_str());
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
        stl1z::filesystem::path temp;
        do
        {
          temp = _has_product(library_dir, _product);
          if(!temp.empty() && stl1z::filesystem::exists(temp / "test" / "tests"))
          {
            ret.path = temp;
            product = _product;
            return ret.path;
          }
          library_dir = stl1z::filesystem::canonical(library_dir / "..");
        } while(library_dir.native().size() > 3);
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the product lives");
        else
        {
          BOOST_KERNELTEST_CERR("FATAL: Couldn't figure out where the product " << _product << " lives. You need a " << _product << " directory somewhere in or above the directory you run the tests from." << std::endl);
          std::terminate();
        }
      }
      catch(...)
      {
        if(!is_throwing)
        {
          BOOST_KERNELTEST_CERR("library_directory() unexpectedly failed" << std::endl);
          std::terminate();
        }
        throw;
      }
    }

    /*! Figure out an absolute path to the correct test workspace template. Uses
    library_directory() for the base of the product and assumes any test workspace
    templates live in product/test/tests.

    \tparam is_throwing If true, throw exceptions for any errors encountered,
    else print a useful message to BOOST_KERNELTEST_CERR() and terminate the
    process.
    */
    template <bool is_throwing = false> inline stl1z::filesystem::path workspace_template_path(const stl1z::filesystem::path &workspace) noexcept(!is_throwing)
    {
      try
      {
        stl1z::filesystem::path library_dir = library_directory();
        if(stl1z::filesystem::exists(library_dir / "test" / "tests" / workspace))
        {
          return library_dir / "test" / "tests" / workspace;
        }
        if(is_throwing)
          throw std::runtime_error("Couldn't figure out where the test workspace templates live");
        else
        {
          BOOST_KERNELTEST_CERR("FATAL: Couldn't figure out where the test workspace templates live for test " << workspace << ". Product source directory is thought to be " << library_dir << std::endl);
          std::terminate();
        }
      }
      catch(...)
      {
        if(!is_throwing)
        {
          BOOST_KERNELTEST_CERR("workspace_template_path() unexpectedly failed" << std::endl);
          std::terminate();
        }
        throw;
      }
    }

    template <bool is_throwing, class Parent, class RetType> struct impl
    {
      stl1z::filesystem::path _current;

      void _remove_workspace() noexcept(!is_throwing)
      {
        stl1z::fs_error_code ec;
        auto begin = stl11::chrono::steady_clock::now();
        do
        {
          bool exists = stl1z::filesystem::exists(_current, ec);
          if(!ec && !exists)
            return;
          stl1z::filesystem::remove_all(_current, ec);
        } while(stl11::chrono::duration_cast<stl11::chrono::seconds>(stl11::chrono::steady_clock::now() - begin).count() < 5);
        if(is_throwing)
          throw std::runtime_error("Couldn't delete workspace after five seconds of trying");
        BOOST_KERNELTEST_CERR("FATAL: Couldn't delete " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl);
        std::terminate();
      }

      impl(Parent *parent, RetType &testret, size_t idx, stl1z::filesystem::path &&workspace) noexcept(!is_throwing)
      {
        auto template_path = workspace_template_path<is_throwing>(workspace);
        // Make the workspace we choose unique to this thread
        _current = starting_path() / ("kerneltest_workspace_" + std::to_string(boost_lite::utils::thread::this_thread_id()));
        // Clear out any stale workspace with the same name at this path just in case
        _remove_workspace();

        stl1z::fs_error_code ec;
        auto fatalexit = [&] {
          if(is_throwing)
            throw stl1z::fs_system_error(ec);
          BOOST_KERNELTEST_CERR("FATAL: Couldn't copy " << template_path << " to " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl);
          std::terminate();
        };
        // Is the input workspace no workspace? In which case create an empty directory
        bool exists = stl1z::filesystem::exists(template_path, ec);
        if(ec)
          fatalexit();
        if(!exists)
        {
          stl1z::filesystem::create_directory(_current, ec);
          if(ec)
            fatalexit();
        }
        else
        {
          auto begin = stl11::chrono::steady_clock::now();
          do
          {
            stl1z::filesystem::copy(template_path, _current, stl1z::filesystem::copy_options::recursive, ec);
            if(!ec)
              break;
          } while(stl11::chrono::duration_cast<stl11::chrono::seconds>(stl11::chrono::steady_clock::now() - begin).count() < 5);
          if(ec)
            fatalexit();
        }
        // Set the working directory to the newly configured workspace
        stl1z::filesystem::current_path(_current);
        current_test_kernel.working_directory = &_current;
      }
      ~impl() noexcept(!is_throwing)
      {
        current_test_kernel.working_directory = nullptr;
        stl1z::filesystem::current_path(starting_path());
        _remove_workspace();
      }
    };
    template <bool is_throwing> struct inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const { return impl<is_throwing, Parent, RetType>(parent, testret, idx, stl1z::filesystem::path(workspacebase) / workspace); }
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
  else print a useful message to BOOST_KERNELTEST_CERR() and terminate the
  process.
  \return A type which when called configures the workspace and changes the working directory to that
  workspace, and on destruction deletes the workspace and changes the working directory back to `starting_path()`.
  `current_test_kernel.working_directory` is also set to the working directory.
  \param workspacebase A path fragment inside `test/tests` of the base of the workspaces to choose from.
  */
  template <bool is_throwing = false> constexpr inline auto filesystem_setup(const char *workspacebase) { return filesystem_setup_impl::inst<is_throwing>{workspacebase}; }

  namespace filesystem_comparison_impl
  {
    //! Walk a directory hierarchy, depth first. f(directory_entry) can return something to early exit.
    template <class U> inline auto depth_first_walk(stl1z::filesystem::path path, U &&f) -> decltype(f(std::declval<stl1z::filesystem::directory_entry>()))
    {
      for(stl1z::filesystem::directory_iterator it(path); it != stl1z::filesystem::directory_iterator(); ++it)
      {
        if(stl1z::filesystem::is_directory(it->status()))
        {
          auto ret(depth_first_walk(it->path(), std::forward<U>(f)));
          if(ret)
            return ret;
        }
      }
      for(stl1z::filesystem::directory_iterator it(path); it != stl1z::filesystem::directory_iterator(); ++it)
      {
        if(!stl1z::filesystem::is_directory(it->status()))
        {
          auto ret(f(*it));
          if(ret)
            return ret;
        }
      }
      // Return default constructed edition of the type returned by the callable
      return decltype(f(std::declval<stl1z::filesystem::directory_entry>()))();
    }
    /*! Compare two directories for equivalence, returning empty result if identical, else
    path of first differing item.
    */
    template <bool compare_contents, bool compare_timestamps> result<stl1z::filesystem::path> compare_directories(stl1z::filesystem::path before, stl1z::filesystem::path after) noexcept
    {
      try
      {
        // Make list of everything in after
        std::unordered_map<stl1z::filesystem::path, stl1z::filesystem::directory_entry, path_hasher> after_items;
        depth_first_walk(after, [&](stl1z::filesystem::directory_entry dirent) -> int {
          after_items[dirent.path()] = std::move(dirent);
          return 0;
        });

        // We need to remove each item as we check, if anything remains we fail
        result<stl1z::filesystem::path> ret = depth_first_walk(before, [&](stl1z::filesystem::directory_entry dirent) -> result<stl1z::filesystem::path> {
          try
          {
            stl1z::filesystem::path leafpath(dirent.path().native().substr(before.native().size() + 1));
            stl1z::filesystem::path afterpath(after / leafpath);
            if(stl1z::filesystem::is_symlink(dirent.symlink_status()))
            {
              if(stl1z::filesystem::is_symlink(dirent.symlink_status()) != stl1z::filesystem::is_symlink(stl1z::filesystem::symlink_status(afterpath)))
                goto differs;
              if(stl1z::filesystem::read_symlink(dirent.path()) != stl1z::filesystem::read_symlink(afterpath))
                goto differs;
            }
            {
              auto beforestatus = dirent.status(), afterstatus = after_items[afterpath].status();
              if(stl1z::filesystem::is_directory(beforestatus) != stl1z::filesystem::is_directory(afterstatus))
                goto differs;
              if(stl1z::filesystem::is_regular_file(beforestatus) != stl1z::filesystem::is_regular_file(afterstatus))
                goto differs;
              if(stl1z::filesystem::file_size(dirent.path()) != stl1z::filesystem::file_size(afterpath))
                goto differs;
              if(compare_timestamps)
              {
                if(beforestatus.permissions() != afterstatus.permissions())
                  goto differs;
                if(stl1z::filesystem::last_write_time(dirent.path()) != stl1z::filesystem::last_write_time(afterpath))
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
            return make_empty_result<stl1z::filesystem::path>();
          differs:
            return leafpath;
          }
          BOOST_OUTCOME_CATCH_EXCEPTION_TO_RESULT(stl1z::filesystem::path)
        });
        // If anything different, return that
        if(ret)
          return ret;
        // If anything in after not in current, return that
        if(!after_items.empty())
          return after_items.begin()->first;
        // Otherwise both current and after are identical
        return make_empty_result<stl1z::filesystem::path>();
      }
      BOOST_OUTCOME_CATCH_EXCEPTION_TO_RESULT(stl1z::filesystem::path)
    }

    template <class Parent, class RetType> struct structure_impl
    {
      Parent *parent;
      RetType &testret;
      size_t idx;
      stl1z::filesystem::path model_workspace;
      ~structure_impl()
      {
        if(!current_test_kernel.working_directory)
        {
          BOOST_KERNELTEST_CERR("FATAL: There appears to be no hooks::filesystem_setup earlier in the hook sequence, therefore I have no workspace to compare to." << std::endl);
          std::terminate();
        }
        // Only do comparison if test passed
        if(testret)
        {
          // If this is empty, workspaces are identical
          result<stl1z::filesystem::path> workspaces_not_identical = compare_directories<false, false>(*current_test_kernel.working_directory, model_workspace);
          // Propagate any error
          if(workspaces_not_identical.has_error())
            testret = error_code_extended(make_error_code(kerneltest_errc::filesystem_comparison_internal_failure), workspaces_not_identical.get_error().message().c_str(), workspaces_not_identical.get_error().value());
          // Set error with extended message of the path which differs
          else if(workspaces_not_identical.has_value())
            testret = error_code_extended(make_error_code(kerneltest_errc::filesystem_comparison_failed), workspaces_not_identical.get().string().c_str());
        }
      }
    };
    struct structure_inst
    {
      const char *workspacebase;
      template <class Parent, class RetType> auto operator()(Parent *parent, RetType &testret, size_t idx, const char *workspace) const { return structure_impl<Parent, RetType>{parent, testret, idx, stl1z::filesystem::path(workspacebase) / workspace}; }
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
  constexpr inline auto filesystem_comparison_structure(const char *workspacebase) { return filesystem_comparison_impl::structure_inst{workspacebase}; }

#if 0

// TODO: Outcome's config.hpp should bind this
BOOST_OUTCOME_V1_NAMESPACE_BEGIN
namespace stl1z
{
  namespace filesystem = std::experimental::filesystem;
  struct path_hasher
  {
    size_t operator()(const filesystem::path &p) const { return hash_value(p.native()); }
  };
}
BOOST_OUTCOME_V1_NAMESPACE_END

BOOST_OUTCOME_V1_NAMESPACE_BEGIN
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
namespace integration_test
{
  template <class T> inline void print_result(bool v, const T &result)
  {
    using namespace console_colours;
    if(v)
      std::cout << bold << green << result << normal << std::endl;
    else
      std::cout << bold << red << "FAILED" << normal << std::endl;
  }
}
BOOST_OUTCOME_V1_NAMESPACE_END


#define BOOST_OUTCOME_INTEGRATION_TEST_KERNEL(suite, name, desc, ...)                                                                                                                                                                                                                                                          \
  \
BOOST_AUTO_TEST_CASE(name, desc)                                                                                                                                                                                                                                                                                               \
  {                                                                                                                                                                                                                                                                                                                            \
    \
static constexpr const char __integration_test_kernel_suite[] = #suite;                                                                                                                                                                                                                                                        \
    \
static constexpr const char __integration_test_kernel_name[] = #name;                                                                                                                                                                                                                                                          \
    \
static constexpr const char __integration_test_kernel_description[] = desc;                                                                                                                                                                                                                                                    \
    using namespace BOOST_OUTCOME_V1_NAMESPACE;                                                                                                                                                                                                                                                                                \
    \
std::cout                                                                                                                                                                                                                                                                                                                      \
    << "\n\n"                                                                                                                                                                                                                                                                                                                  \
    << console_colours::bold << console_colours::blue << __integration_test_kernel_suite << " / " << __integration_test_kernel_name << ":\n"                                                                                                                                                                                   \
    << console_colours::bold << console_colours::white << desc << console_colours::normal << std::endl;                                                                                                                                                                                                                        \
    \
__VA_ARGS__;                                                                                                                                                                                                                                                                                                                   \
  }

BOOST_OUTCOME_V1_NAMESPACE_BEGIN
namespace integration_test
{
  template <class T, class Outcome> struct kernel_parameter_to_filesystem
  {
    T parameter_value;
    stl1z::filesystem::path before;
    Outcome result;
    stl1z::filesystem::path after;
    kernel_parameter_to_filesystem(T v, stl1z::filesystem::path b, Outcome o, stl1z::filesystem::path a)
        : parameter_value(std::move(v))
        , before(std::move(b))
        , result(std::move(o))
        , after(std::move(a))
    {
    }
  };
  template <class T, class Outcome> using parameters_type = std::initializer_list<kernel_parameter_to_filesystem<T, Outcome>>;

  /* Sets up a workspace directory for the test to run inside and checks it is correct after
  The working directory on process start is assumed to be the correct place to put test workspaces.
  */
  class filesystem_workspace
  {
    stl1z::filesystem::path _before, _after, _current;

  public:
    // Record the current working directory and store it
    static const stl1z::filesystem::path &starting_path()
    {
      static stl1z::filesystem::path p = stl1z::filesystem::current_path();
      return p;
    }

  private:
    stl1z::filesystem::path _has_afio(stl1z::filesystem::path dir)
    {
      if(stl1z::filesystem::exists(dir / "boost.afio"))
        return dir / "boost.afio";
      if(stl1z::filesystem::exists(dir / "afio"))
        return dir / "afio";
      return stl1z::filesystem::path();
    }

  public:
    // Figure out an absolute path to the correct test workspace templates
    stl1z::filesystem::path workspace_template_path(const char *test_name)
    {
      // Layout is <boost.afio>/test/tests/<test_name>/<workspace_templates>
      stl1z::filesystem::path afiodir = starting_path(), temp;
      do
      {
        temp = _has_afio(afiodir);
        if(!temp.empty() && stl1z::filesystem::exists(temp / "test" / "tests" / test_name))
        {
          return temp / "test" / "tests" / test_name;
        }
        afiodir = stl1z::filesystem::canonical(afiodir / "..");
      } while(afiodir.native().size() > 3);
      std::cerr << "FATAL: Couldn't figure out where the test workspace templates live for test " << test_name << ". You need a boost.afio or afio directory somewhere in or above the directory you run the tests from." << std::endl;
      std::terminate();
    }

  private:
    void _remove_workspace() noexcept
    {
      stl11::error_code ec;
      auto begin = stl11::chrono::steady_clock::now();
      do
      {
        bool exists = stl1z::filesystem::exists(_current, ec);
        if(!ec && !exists)
          return;
        stl1z::filesystem::remove_all(_current, ec);
      } while(stl11::chrono::duration_cast<stl11::chrono::seconds>(stl11::chrono::steady_clock::now() - begin).count() < 5);
      std::cerr << "FATAL: Couldn't delete " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl;
      std::terminate();
    }
    void _setup_workspace() noexcept
    {
      stl11::error_code ec;
      // Is the input workspace no workspace?
      bool exists = stl1z::filesystem::exists(_before, ec);
      if(ec)
        goto fatalexit;
      if(!exists)
      {
        stl1z::filesystem::create_directory(_current, ec);
        if(ec)
          goto fatalexit;
        return;
      }
      {
        auto begin = stl11::chrono::steady_clock::now();
        do
        {
          stl1z::filesystem::copy(_before, _current, stl1z::filesystem::copy_options::recursive, ec);
          if(!ec)
            return;
        } while(stl11::chrono::duration_cast<stl11::chrono::seconds>(stl11::chrono::steady_clock::now() - begin).count() < 5);
      }
    fatalexit:
      std::cerr << "FATAL: Couldn't copy " << _before << " to " << _current << " due to " << ec.message() << " after five seconds of trying." << std::endl;
      std::terminate();
    }
    // We use a depth first strategy. f(directory_entry) can return something to early exit.
    template <class U> static auto _walk(stl1z::filesystem::path path, U &&f) -> decltype(f(std::declval<stl1z::filesystem::directory_entry>()))
    {
      for(stl1z::filesystem::directory_iterator it(path); it != stl1z::filesystem::directory_iterator(); ++it)
      {
        if(stl1z::filesystem::is_directory(it->status()))
        {
          auto ret(_walk(it->path(), std::forward<U>(f)));
          if(ret)
            return ret;
        }
      }
      for(stl1z::filesystem::directory_iterator it(path); it != stl1z::filesystem::directory_iterator(); ++it)
      {
        if(!stl1z::filesystem::is_directory(it->status()))
        {
          auto ret(f(*it));
          if(ret)
            return ret;
        }
      }
      // Return default constructed edition of the type returned by the callable
      return decltype(f(std::declval<stl1z::filesystem::directory_entry>()))();
    }
    // We only compare location, names and sizes. Other metadata like timestamps or perms not compared.
    // Returns empty result if identical, else path of first differing item
    result<stl1z::filesystem::path> _compare_workspace() const noexcept
    {
      // Make list of everything in _after
      std::unordered_map<stl1z::filesystem::path, stl1z::filesystem::directory_entry, stl1z::path_hasher> _after_items;
      _walk(_after, [&](stl1z::filesystem::directory_entry dirent) -> int {
        _after_items[dirent.path()] = std::move(dirent);
        return 0;
      });

      // We need to remove each item as we check, if anything remains we fail
      result<stl1z::filesystem::path> ret = _walk(_current, [&](stl1z::filesystem::directory_entry dirent) -> result<stl1z::filesystem::path> {
        stl1z::filesystem::path leafpath(dirent.path().native().substr(_current.native().size() + 1));
        stl1z::filesystem::path afterpath(_after / leafpath);
        if(stl1z::filesystem::is_symlink(dirent.symlink_status()) != stl1z::filesystem::is_symlink(stl1z::filesystem::symlink_status(afterpath)))
          goto differs;
        {
          auto beforestatus = dirent.status(), afterstatus = _after_items[afterpath].status();
          if(stl1z::filesystem::is_directory(beforestatus) != stl1z::filesystem::is_directory(afterstatus))
            goto differs;
          if(stl1z::filesystem::is_regular_file(beforestatus) != stl1z::filesystem::is_regular_file(afterstatus))
            goto differs;
        }
        // This item is identical
        _after_items.erase(afterpath);
        return make_empty_result<stl1z::filesystem::path>();
      differs:
        return leafpath;
      });
      // If anything different, return that
      if(ret)
        return ret;
      // If anything in after not in current, return that
      if(!_after_items.empty())
        return _after_items.begin()->first;
      // Otherwise both current and after are identical
      return make_empty_result<stl1z::filesystem::path>();
    }

  public:
    template <class ParamType> filesystem_workspace(const char *test_name, const ParamType &param, size_t no, size_t total)
    {
      auto template_path = workspace_template_path(test_name);
      _before = template_path / param.before;
      _after = template_path / param.after;
      _current = starting_path() / "workspace";
      _remove_workspace();
      _setup_workspace();
      stl1z::filesystem::current_path(_current);
      using namespace console_colours;
      std::cout << std::endl
                << yellow << (no + 1) << "/" << total << ":" << normal << " Running filesystem integration test kernel "       //
                << magenta << test_name << normal << " to see if input parameter " << cyan << param.parameter_value << normal  //
                << " causes input workspace " << bold << white << param.before << normal << " to become output workspace "     //
                << bold << white << param.after << normal << std::endl
                << "         Test kernel execution: " << std::flush;
    }

    ~filesystem_workspace()
    {
      std::cout << "   Test file system comparison: " << std::flush;
      result<stl1z::filesystem::path> workspaces_not_identical = _compare_workspace();
      print_result(!workspaces_not_identical, "MATCHES");
      BOOST_CHECK(!workspaces_not_identical);
      if(workspaces_not_identical.has_error())
        std::cout << "NOTE: Filesystem workspace comparison failed due to " << workspaces_not_identical.get_error().message() << std::endl;
      if(workspaces_not_identical.has_value())
        std::cout << "NOTE: Filesystem workspace comparison failed because item " << workspaces_not_identical.get() << " is not identical" << std::endl;
      stl1z::filesystem::current_path(starting_path());
      _remove_workspace();
    }
  };
}
BOOST_OUTCOME_V1_NAMESPACE_END

#define BOOST_OUTCOME_INTEGRATION_TEST_REMOVE_BRACKETS(...) __VA_ARGS__

// outcomes has format { parvalue, dirbefore, outcome, dirafter }
#define BOOST_OUTCOME_INTEGRATION_TEST_ST_KERNEL_PARAMETER_TO_FILESYSTEM(__outcometype, __param, __testdir, __outcomes_initialiser, ...)                                                                                                                                                                                       \
  \
static const BOOST_OUTCOME_V1_NAMESPACE::integration_test::parameters_type<decltype(__param), BOOST_OUTCOME_INTEGRATION_TEST_REMOVE_BRACKETS __outcometype>                                                                                                                                                                    \
  __outcomes = BOOST_OUTCOME_INTEGRATION_TEST_REMOVE_BRACKETS __outcomes_initialiser;                                                                                                                                                                                                                                          \
  \
size_t __no = 0;                                                                                                                                                                                                                                                                                                               \
  \
for(const auto &__outcome                                                                                                                                                                                                                                                                                                      \
    : __outcomes)                                                                                                                                                                                                                                                                                                              \
  \
{                                                                                                                                                                                                                                                                                                                         \
    \
BOOST_OUTCOME_V1_NAMESPACE::integration_test::filesystem_workspace __workspace((__testdir), __outcome, __no++, __outcomes.size());                                                                                                                                                                                             \
    \
(__param) = __outcome.parameter_value;                                                                                                                                                                                                                                                                                         \
    \
__VA_ARGS__                                                                                                                                                                                                                                                                                                             \
  \
}

#define BOOST_OUTCOME_INTEGRATION_TEST_MT_KERNEL_PARAMETER_TO_FILESYSTEM(__outcometype, __param, __testdir, __outcomes_initialiser, ...) BOOST_OUTCOME_INTEGRATION_TEST_ST_KERNEL_PARAMETER_TO_FILESYSTEM(__outcometype, __param, __testdir, __outcomes_initialiser, __VA_ARGS__)

BOOST_OUTCOME_V1_NAMESPACE_BEGIN namespace integration_test
{
  template <class T> void check_result(const outcome<T> &kernel_outcome, const outcome<T> &shouldbe)
  {
    print_result(kernel_outcome == shouldbe, kernel_outcome);
    BOOST_CHECK(kernel_outcome == shouldbe);
  };
  template <class T> void check_result(const result<T> &kernel_outcome, const result<T> &shouldbe)
  {
    print_result(kernel_outcome == shouldbe, kernel_outcome);
    BOOST_CHECK(kernel_outcome == shouldbe);
  };
  template <class T> void check_result(const option<T> &kernel_outcome, const option<T> &shouldbe)
  {
    print_result(kernel_outcome == shouldbe, kernel_outcome);
    BOOST_CHECK(kernel_outcome == shouldbe);
  };

  // If should be has type void, we only care kernel_outcome has a value
  template <class T> void check_result(const outcome<T> &kernel_outcome, const outcome<void> &shouldbe)
  {
    if(kernel_outcome.has_value() && shouldbe.has_value())
    {
      print_result(kernel_outcome.has_value() == shouldbe.has_value(), kernel_outcome);
      BOOST_CHECK(kernel_outcome.has_value() == shouldbe.has_value());
    }
    else
    {
      print_result(kernel_outcome == shouldbe, kernel_outcome);
      BOOST_CHECK(kernel_outcome == shouldbe);
    }
  };
  template <class T> void check_result(const result<T> &kernel_outcome, const result<void> &shouldbe)
  {
    if(kernel_outcome.has_value() && shouldbe.has_value())
    {
      print_result(kernel_outcome.has_value() == shouldbe.has_value(), kernel_outcome);
      BOOST_CHECK(kernel_outcome.has_value() == shouldbe.has_value());
    }
    else
    {
      print_result(kernel_outcome == shouldbe, kernel_outcome);
      BOOST_CHECK(kernel_outcome == shouldbe);
    }
  };
  template <class T> void check_result(const option<T> &kernel_outcome, const option<void> &shouldbe)
  {
    if(kernel_outcome.has_value() && shouldbe.has_value())
    {
      print_result(kernel_outcome.has_value() == shouldbe.has_value(), kernel_outcome);
      BOOST_CHECK(kernel_outcome.has_value() == shouldbe.has_value());
    }
    else
    {
      print_result(kernel_outcome == shouldbe, kernel_outcome);
      BOOST_CHECK(kernel_outcome == shouldbe);
    }
  };
}
BOOST_OUTCOME_V1_NAMESPACE_END

// If __outcome.outcome has type void, it means we don't care what the value is in the non-errored outcome
#define BOOST_OUTCOME_INTEGRATION_TEST_KERNEL_RESULT(value) BOOST_OUTCOME_V1_NAMESPACE::integration_test::check_result(value, __outcome.result);

#include "../include/boost/afio.hpp"

#endif
}
BOOST_KERNELTEST_V1_NAMESPACE_END

#endif
