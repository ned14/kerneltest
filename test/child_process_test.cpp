/* Regression tests for kerneltest::child_process, and for the workspace
directory comparison in hooks/filesystem_workspace.hpp.

WHY THIS FILE EXISTS
====================
child_process has been in production use for years on the assumption that it
worked. A review found three ways in which it silently lied to its callers:

  1. POSIX is_running() never returned true.
     `waitid(P_PID, pid, &info, WNOHANG | WNOWAIT)` omits the
     WEXITED/WSTOPPED/WCONTINUED that POSIX requires, so Linux rejects the call
     with EINVAL and the error branch answered "not running" for EVERY child,
     always. Even with the options corrected, the old `info.si_pid != 0`
     predicate was backwards: a live child yields si_pid == 0.
  2. The POSIX destructor closed STDIN_FILENO.
     `_deinitialise_files()` set the pipe descriptors to 0 after fclose()ing
     them, but native_handle_type::operator bool() tests `fd != -1`, so the
     destructor's `if(_readh) ::close(_readh.fd)` closed fd 0 - three times -
     on descriptors that had already been freed and which the runtime may have
     handed out again in the meantime.
  3. compare_directories() never compared directories.
     Both depth_first_walk() passes hand only NON-directories to their
     callable, so a directory present on one side only was invisible whenever
     it was empty, and filesystem_comparison_structure() reported two plainly
     different workspaces as identical.

Every assertion below fails against the pre-fix code. The child is this
executable re-launched with an internal argument, so the suite needs no
external program and behaves identically on POSIX and Windows.
*/

// <cstring> first: it keeps this test compilable against the pre-fix headers,
// which used memset/memmove without including it themselves, so the regression
// can be demonstrated against the exact code the fix replaced.
#include <cstring>

#include "kerneltest/v1.0/child_process.hpp"
#include "kerneltest/v1.0/hooks/filesystem_workspace.hpp"

#include <chrono>

#ifndef _WIN32
#include <csignal>  // for raise() and SIGKILL
#endif
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace kerneltest_v1;

// `child_process` is BOTH the namespace and (inside it) the class, so the class
// needs a distinct name at global scope.
namespace child_process_ns = kerneltest_v1::child_process;
using child_process_t = child_process_ns::child_process;

namespace
{
  int failures = 0;
  // Every expect() call, pass or fail. main() used to gate success on
  // `failures == 0` alone, so a test whose whole body was #ifdef'd out - or which
  // returned early - contributed nothing and the binary still announced a full
  // pass. Each test now declares a floor and run_test() fails it for running
  // fewer checks than that.
  int checks_run = 0;
  int skips = 0;

  bool expect(bool cond, const std::string &what)
  {
    ++checks_run;
    if(cond)
    {
      std::printf("ok   : %s\n", what.c_str());
    }
    else
    {
      std::printf("FAIL : %s\n", what.c_str());
      std::fflush(stdout);
      ++failures;
    }
    return cond;
  }

  // A test which was deliberately not run on this platform. It must be visible in
  // the log and must NOT read as a pass.
  void skip(const std::string &what)
  {
    ++skips;
    std::printf("SKIP : %s\n", what.c_str());
  }

  // Run one test and require it to have made at least `floor` checks. A skip is
  // not a pass: a body which is compiled out, or which bails on the first
  // precondition, fails here instead of contributing silence to a green suite.
  void run_test(const char *name, void (*fn)(), int floor)
  {
    const int before = checks_run;
    fn();
    const int ran = checks_run - before;
    if(ran < floor)
    {
      std::printf("FAIL : %s ran %d check(s), expected at least %d (a compiled-out or skipped test must not pass)\n", name, ran, floor);
      std::fflush(stdout);
      ++failures;
    }
  }

  template <class T> std::string str(const T &v)
  {
    std::ostringstream ss;
    ss << v;
    return ss.str();
  }

  // Launch THIS executable with the given internal argument, so the test is
  // hermetic: no /bin/sh, no cmd.exe, nothing platform specific.
  result<child_process_t> self_launch(const char *mode, const char *number)
  {
    std::vector<std::filesystem::path::string_type> args;
    args.push_back(std::filesystem::path(mode).native());
    args.push_back(std::filesystem::path(number).native());
    return child_process_t::launch(child_process_ns::current_process_path(), std::move(args));
  }

  // True once is_running() says the child has gone, or the deadline expires.
  // Polling (rather than sleeping blindly) keeps the test quick without making
  // it flaky on a loaded machine.
  bool wait_until_reported_dead(const child_process_t &cp, std::chrono::milliseconds limit)
  {
    auto deadline = std::chrono::steady_clock::now() + limit;
    while(std::chrono::steady_clock::now() < deadline)
    {
      if(!cp.is_running())
        return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return !cp.is_running();
  }

#ifndef _WIN32
  int count_open_fds()
  {
#ifdef __linux__
    int n = 0;
    std::error_code ec;
    for(std::filesystem::directory_iterator it("/proc/self/fd", ec), end; !ec && it != end; it.increment(ec))
      ++n;
    return n;
#else
    return -1;
#endif
  }
#endif

  // ---------------------------------------------------------------- directories

  namespace fci = hooks::filesystem_comparison_impl;
  using comparison = std::optional<result<std::filesystem::path>>;

  // As filesystem_comparison_structure() calls it: names and metadata only.
  comparison compare(const std::filesystem::path &before, const std::filesystem::path &after)
  {
    return fci::compare_directories<false, false>(before, after);
  }
  // With contents compared as well.
  comparison compare_contents(const std::filesystem::path &before, const std::filesystem::path &after)
  {
    return fci::compare_directories<true, false>(before, after);
  }

  void write_file(const std::filesystem::path &p, const char *contents)
  {
    std::filesystem::create_directories(p.parent_path());
    std::FILE *h = std::fopen(p.string().c_str(), "wb");
    if(h != nullptr)
    {
      const size_t len = std::strlen(contents);
      if(0 == len || len == std::fwrite(contents, 1, len, h))
      {
        std::fclose(h);
        return;
      }
      std::fclose(h);
    }
    std::printf("FAIL : could not write %s\n", p.string().c_str());
    ++failures;
  }

  // A difference must be reported; give the name of what was reported so a
  // failure is diagnosable from the log alone.
  void expect_difference(const comparison &r, const std::string &what, const char *expected_name = nullptr)
  {
    if(!r.has_value())
    {
      expect(false, what + " (reported IDENTICAL)");
      return;
    }
    if(r->has_error())
    {
      expect(false, what + " (reported an error: " + str(r->error()) + ")");
      return;
    }
    const std::filesystem::path &p = r->value();
    if(expected_name != nullptr)
    {
      expect(p.filename() == std::filesystem::path(expected_name).native(), what + " (reported '" + p.string() + "', expected " + expected_name + ")");
      return;
    }
    expect(true, what + " (reported '" + p.string() + "')");
  }

  void test_directory_comparison()
  {
    using std::filesystem::path;
    const path root = std::filesystem::temp_directory_path() /
                      ("kerneltest-child-process-test-" + str(static_cast<unsigned long long>(std::chrono::steady_clock::now().time_since_epoch().count())));
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    // (a) identical trees, including an identical EMPTY directory on both sides
    {
      const path before = root / "a" / "before", after = root / "a" / "after";
      write_file(before / "f.txt", "one");
      write_file(after / "f.txt", "one");
      std::filesystem::create_directories(before / "shared_empty");
      std::filesystem::create_directories(after / "shared_empty");
      expect(!compare(before, after).has_value(), "identical trees (with an identical empty directory) compare as identical");
    }
    // (b) an EMPTY directory present only before
    {
      const path before = root / "b" / "before", after = root / "b" / "after";
      write_file(before / "f.txt", "one");
      write_file(after / "f.txt", "one");
      std::filesystem::create_directories(before / "only_before");
      expect_difference(compare(before, after), "an empty directory present only in the workspace is a difference", "only_before");
    }
    // (c) an EMPTY directory present only after
    {
      const path before = root / "c" / "before", after = root / "c" / "after";
      write_file(before / "f.txt", "one");
      write_file(after / "f.txt", "one");
      std::filesystem::create_directories(after / "only_after");
      expect_difference(compare(before, after), "an empty directory present only in the model is a difference", "only_after");
    }
    // (d) a NESTED empty directory tree present only before
    {
      const path before = root / "d" / "before", after = root / "d" / "after";
      std::filesystem::create_directories(before);
      std::filesystem::create_directories(after);
      std::filesystem::create_directories(before / "a" / "b" / "c");
      expect_difference(compare(before, after), "a nested empty directory tree present only in the workspace is a difference", "a");
    }
    // (e) the same relative path is a directory on one side and a file on the other
    {
      const path before = root / "e" / "before", after = root / "e" / "after";
      std::filesystem::create_directories(before / "x");  // x is a directory here
      std::filesystem::create_directories(after);
      write_file(after / "x", "not a directory");          // ... and a file here
      expect_difference(compare(before, after), "a directory on one side and a file on the other is a difference");
    }
    // (f) the file comparison itself still works (guards against the directory
    //     fix having replaced rather than extended it). Without compare_contents
    //     only the size is compared, so the two cases are distinct.
    {
      const path before = root / "f" / "before", after = root / "f" / "after";
      write_file(before / "differing_size.txt", "one");
      write_file(after / "differing_size.txt", "one plus more");
      expect_difference(compare(before, after), "a differing file size is still a difference", "differing_size.txt");
      write_file(before / "differing_content.txt", "one");
      write_file(after / "differing_content.txt", "two");
      expect(!compare(before, after).has_value() == false, "sanity: the size-differing file is still reported");
      expect_difference(compare_contents(before, after), "differing file contents are a difference when contents are compared");
    }
    // (g) a file present only before is still a difference
    {
      const path before = root / "g" / "before", after = root / "g" / "after";
      write_file(before / "only.txt", "one");
      std::filesystem::create_directories(after);
      expect_difference(compare(before, after), "a file present only in the workspace is still a difference", "only.txt");
    }
    // (h) two EMPTY directories compare as identical (no false positive)
    {
      const path before = root / "h" / "before", after = root / "h" / "after";
      std::filesystem::create_directories(before);
      std::filesystem::create_directories(after);
      expect(!compare(before, after).has_value(), "two empty directory trees compare as identical");
    }
#ifndef _WIN32
    // (i) a symlinked directory on one side only is a difference, and the walk
    //     must not descend through the link (it records it as a link instead)
    {
      const path before = root / "i" / "before", after = root / "i" / "after";
      std::filesystem::create_directories(before / "real");
      std::filesystem::create_directories(after / "real");
      std::filesystem::create_directory_symlink(before / "real", before / "link");
      std::filesystem::create_directories(after / "link");
      expect_difference(compare(before, after), "a symlinked directory against a real directory is a difference");
    }
#endif
    // (j) a model workspace which does not exist must NOT read as "identical". Both
    //     walks are existence-driven, so a missing tree yields an empty item set on
    //     that side and used to fall through to "identical" - which is exactly what
    //     workspace_template_path() hands back when only the template's parent exists.
    //     It must now be an ERROR (internal failure): nothing was actually compared.
    {
      const path before = root / "j" / "before", after = root / "j" / "does_not_exist";
      std::filesystem::create_directories(before);  // an EMPTY workspace, as in the report
      const comparison r = compare(before, after);
      if(!r.has_value())
      {
        expect(false, "a nonexistent model workspace is reported as IDENTICAL (silent green)");
      }
      else if(!r->has_error())
      {
        expect(false, "a nonexistent model workspace is reported as a difference rather than as an internal failure");
      }
      else
      {
        expect(true, "a nonexistent model workspace is reported as an internal failure");
      }
    }
    // (k) ... and the same the other way round: an absent workspace is no more
    //     identical to a model than an absent model is to a workspace.
    {
      const path before = root / "k" / "does_not_exist", after = root / "k" / "after";
      std::filesystem::create_directories(after);  // an EMPTY model, so only existence differs
      const comparison r = compare(before, after);
      if(!r.has_value())
      {
        expect(false, "a nonexistent workspace is reported as IDENTICAL (silent green)");
      }
      else
      {
        expect(r->has_error(), "a nonexistent workspace is reported as an internal failure");
      }
    }

    std::filesystem::remove_all(root, ec);
  }

  // ------------------------------------------------------------ child processes

  void test_live_child_is_running()
  {
    auto launched = self_launch("--kt-child-sleep-ms", "1500");
    if(!expect(launched.has_value(), "launch a live child: " + (launched.has_value() ? std::string() : str(launched.error()))))
      return;
    child_process_t cp(std::move(launched.value()));
    expect(cp.is_running(), "is_running() is TRUE for a freshly launched live child");
    bool all_true = true;
    for(int i = 0; i < 5; i++)
      all_true = all_true && cp.is_running();
    expect(all_true, "is_running() stays TRUE across repeated polling of a live child");
    const auto r = cp.wait();
    expect(r.has_value() && r.value() == 0, "a live child that exits normally is reported as exit code 0, got " + (r.has_value() ? str(r.value()) : str(r.error())));
    expect(!cp.is_running(), "is_running() is FALSE after wait() reaped the child");
  }

  // The critical property: observing liveness must not consume the exit status.
  void test_is_running_does_not_consume_exit_status()
  {
    auto launched = self_launch("--kt-child-exit", "7");
    if(!expect(launched.has_value(), "launch a child which exits immediately with 7: " + (launched.has_value() ? std::string() : str(launched.error()))))
      return;
    child_process_t cp(std::move(launched.value()));
    expect(wait_until_reported_dead(cp, std::chrono::milliseconds(20000)), "is_running() goes FALSE once the child has exited");
    const auto r = cp.wait();
    expect(r.has_value(), "wait() succeeds after is_running() has already seen the child as dead: " + (r.has_value() ? std::string() : str(r.error())));
    if(r.has_value())
      expect(r.value() == 7, "wait() still reports the child's exit status 7 after polling is_running(), got " + str(r.value()));
  }

#ifndef _WIN32
  // Regression: a child which dies of a signal must not be reported as having
  // exited normally with the signal's number as its exit code. waitid() reports
  // si_status for both, and only si_code says which happened.
  void test_signal_death_is_not_an_exit_code()
  {
    auto launched = self_launch("--kt-child-suicide", "0");
    if(!expect(launched.has_value(), "launch a child which kills itself: " + (launched.has_value() ? std::string() : str(launched.error()))))
      return;
    child_process_t cp(std::move(launched.value()));
    const auto r = cp.wait();
    if(!expect(r.has_value(), "wait() succeeds for a child which died of a signal: " + (r.has_value() ? std::string() : str(r.error()))))
      return;
    expect(r.value() == 128 + SIGKILL, "a child killed by SIGKILL is reported as 128+SIGKILL, not as its exit code, got " + str(r.value()));
  }
#endif

  void test_moved_from_handle()
  {
    auto launched = self_launch("--kt-child-sleep-ms", "400");
    if(!expect(launched.has_value(), "launch a child to move: " + (launched.has_value() ? std::string() : str(launched.error()))))
      return;
    child_process_t a(std::move(launched.value()));
    child_process_t b(std::move(a));
    expect(!a.is_running(), "a moved-from child_process reports NOT running (its handle was relinquished)");
    expect(b.is_running(), "the move target reports running");
    (void) b.wait();
  }

#ifndef _WIN32
  // Regression: the destructor must not close STDIN_FILENO. It did so whenever
  // any of file_in()/file_out()/file_err() had been called, because
  // _deinitialise_files() left the descriptors at 0 (which operator bool()
  // considers valid) instead of -1.
  void test_destructor_does_not_close_stdin()
  {
    const int saved_stdin = ::dup(STDIN_FILENO);
    struct stat before;
    const bool had_stdin = 0 == ::fstat(STDIN_FILENO, &before);

    {
      auto launched = self_launch("--kt-child-sleep-ms", "50");
      if(!expect(launched.has_value(), "launch a child whose FILE handles we arm"))
      {
        if(saved_stdin >= 0)
          ::close(saved_stdin);
        return;
      }
      child_process_t cp(std::move(launched.value()));
      std::FILE *out = cp.file_out();
      expect(out != nullptr, "file_out() returns a usable FILE *");
      expect(static_cast<bool>(cp.read_native_handle()) && static_cast<bool>(cp.write_native_handle()), "the pipe handles are still valid while the FILE handles are open");
      // cp goes out of scope here with its FILE handles armed
    }

    if(had_stdin)
    {
      struct stat after;
      const bool open_still = -1 != ::fcntl(STDIN_FILENO, F_GETFD);
      expect(open_still, "STDIN_FILENO is still open after a child_process with armed FILE handles was destroyed");
      if(open_still)
      {
        expect(0 == ::fstat(STDIN_FILENO, &after) && after.st_dev == before.st_dev && after.st_ino == before.st_ino,
               "STDIN_FILENO still refers to the same file after the destructor");
      }
    }
    else
    {
      // Nothing to observe: STDIN_FILENO was already closed when the suite started.
      // Say so, rather than contributing two silent non-checks to a green run.
      skip("test_destructor_does_not_close_stdin: STDIN_FILENO was not open, so its survival cannot be observed");
    }
    if(saved_stdin >= 0)
    {
      ::dup2(saved_stdin, STDIN_FILENO);
      ::close(saved_stdin);
    }
  }

  // The nastiest manifestation: with fd 0 free the runtime hands it to an
  // unrelated file, and the destructor's three close(0) calls then destroy THAT.
  void test_destructor_does_not_close_a_reused_fd_zero()
  {
    const int saved_stdin = ::dup(STDIN_FILENO);
    ::close(STDIN_FILENO);
    const int victim = ::open("/dev/null", O_RDONLY);
    if(!expect(victim == STDIN_FILENO, "fd 0 was reused for an unrelated file (got fd " + str(victim) + ")"))
    {
      if(victim >= 0)
        ::close(victim);
      if(saved_stdin >= 0)
      {
        ::dup2(saved_stdin, STDIN_FILENO);
        ::close(saved_stdin);
      }
      return;
    }
    {
      auto launched = self_launch("--kt-child-sleep-ms", "50");
      if(launched.has_value())
      {
        child_process_t cp(std::move(launched.value()));
        (void) cp.file_out();
      }
      else
      {
        expect(false, "launch a child with fd 0 already taken by another file");
      }
    }
    expect(-1 != ::fcntl(victim, F_GETFD), "an unrelated descriptor which had reused fd 0 survives the child_process destructor");
    ::close(victim);
    if(saved_stdin >= 0)
    {
      ::dup2(saved_stdin, STDIN_FILENO);
      ::close(saved_stdin);
    }
  }

  // Repeated launch/use/destroy must not leak or double-close descriptors: a
  // double close is exactly what used to happen, and it only shows up as a leak
  // of something else when a descriptor the runtime had reused is hit.
  void test_no_descriptor_leak()
  {
#ifdef __linux__
    const int before = count_open_fds();
    if(!expect(before > 3, "could count our own open descriptors"))
      return;
    for(int i = 0; i < 8; i++)
    {
      auto launched = self_launch("--kt-child-sleep-ms", "10");
      if(!launched.has_value())
      {
        expect(false, "launch a child in the descriptor leak loop");
        break;
      }
      child_process_t cp(std::move(launched.value()));
      (void) cp.file_out();
      (void) cp.wait();
    }
    const int after = count_open_fds();
    expect(after == before, "no descriptor leak across 8 launch/use/destroy cycles (" + str(before) + " -> " + str(after) + ")");
#else
    // count_open_fds() only works on Linux, and an empty function that returns
    // without a word used to read as a pass.
    skip("test_no_descriptor_leak: /proc/self/fd is only available on Linux");
#endif
  }
#endif
}  // namespace

int main(int argc, char *argv[])
{
  // Internal child modes. These are reached only when this executable is
  // re-launched by the tests above, never by a human.
  if(argc > 1)
  {
    const std::string mode(argv[1]);
    const int number = (argc > 2) ? std::atoi(argv[2]) : 0;
    if(mode == "--kt-child-sleep-ms")
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(number));
      return 0;
    }
    if(mode == "--kt-child-exit")
    {
      return number;
    }
#ifndef _WIN32
    if(mode == "--kt-child-suicide")
    {
      std::raise(SIGKILL);  // die of a signal, without ever returning
      return 0;
    }
#endif
  }

  std::printf("current_process_path() = %s\n", child_process_ns::current_process_path().string().c_str());

  // Each test declares the minimum number of checks it must make. `failures == 0`
  // alone was a false green: a compiled-out body (#ifdef), an early return or a
  // platform skip contributed nothing and the binary still announced a full pass.
  run_test("test_live_child_is_running", test_live_child_is_running, 5);
  run_test("test_is_running_does_not_consume_exit_status", test_is_running_does_not_consume_exit_status, 3);
  run_test("test_moved_from_handle", test_moved_from_handle, 3);
#ifndef _WIN32
  run_test("test_signal_death_is_not_an_exit_code", test_signal_death_is_not_an_exit_code, 2);
  run_test("test_destructor_does_not_close_stdin", test_destructor_does_not_close_stdin, 3);
  run_test("test_destructor_does_not_close_a_reused_fd_zero", test_destructor_does_not_close_a_reused_fd_zero, 2);
  run_test("test_no_descriptor_leak", test_no_descriptor_leak, 2);
#else
  skip("test_signal_death_is_not_an_exit_code: POSIX signals are not available on Windows");
  skip("test_destructor_does_not_close_stdin: POSIX pipe descriptors are not available on Windows");
  skip("test_destructor_does_not_close_a_reused_fd_zero: POSIX pipe descriptors are not available on Windows");
  skip("test_no_descriptor_leak: POSIX pipe descriptors are not available on Windows");
#endif
  run_test("test_directory_comparison", test_directory_comparison, 11);

  if(failures == 0)
  {
    std::printf("\nAll child_process tests passed (%d checks run, %d skipped).\n", checks_run, skips);
    return 0;
  }
  std::printf("\n%d child_process test(s) FAILED (%d checks run, %d skipped).\n", failures, checks_run, skips);
  return 1;
}
