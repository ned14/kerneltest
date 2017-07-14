# Boost.KernelTest
C++14 automated code test infrastructure with permutation, fuzzing, sanitising and edge coverage

Do NOT attempt to use this library in its current state!

## Some notes on what this library is for and will eventually do

KernelTest is a test runner for your choice of unit test framework, so the idea is that instead of
_test cases_, you write instead _test kernels_ which are simply small functions exercising a
**minimal set** of your public API. Here's one from AFIO:

```c++
namespace file_handle_create_close
{
  BOOST_AFIO_TEST_KERNEL_DECL boost::outcome::result<boost::afio::file_handle> test_kernel_file_handle(boost::afio::file_handle::mode m, boost::afio::file_handle::creation c, boost::afio::file_handle::flag f)
  {
    auto h = boost::afio::file_handle::file("testfile.txt", m, c, boost::afio::file_handle::caching::all, f);
    if(h)
      h.get().close();
    return h;
  }
}
```

In the above test kernel only three parameters of the `afio::file_handle::file()` constructor
are exposed. One then has KernelTest call that test kernel many times with a long initialiser
list of parameter permutations and expected outcomes like this:

```c++
//  Outcome (void means no error)    Kernel parameter call set                                                                     Preconditions       Postconditions
{   make_ready_result<void>(),       { file_handle::mode::none,       file_handle::creation::if_needed, file_handle::flag::none }, { "existing1"    }, { "existing1"    }},
{   make_ready_result<void>(),       { file_handle::mode::attr_read,  file_handle::creation::if_needed, file_handle::flag::none }, { "existing1"    }, { "existing1"    }},
{   make_ready_result<void>(),       { file_handle::mode::attr_write, file_handle::creation::if_needed, file_handle::flag::none }, { "existing1"    }, { "existing1"    }},
{   make_ready_result<void>(),       { file_handle::mode::write,      file_handle::creation::if_needed, file_handle::flag::none }, { "non-existing" }, { "existing0"    }},
{   make_ready_result<void>(),       { file_handle::mode::write,      file_handle::creation::if_needed, file_handle::flag::none }, { "existing1"    }, { "existing1"    }},
{   make_ready_result<void>(),       { file_handle::mode::append,     file_handle::creation::if_needed, file_handle::flag::none }, { "non-existing" }, { "existing0"    }},
{   make_ready_result<void>(),       { file_handle::mode::append,     file_handle::creation::if_needed, file_handle::flag::none }, { "existing1"    }, { "existing1"    }},
...
```

It is however tedious and error prone to write out by hand every single **valid** permutation of
even the above three parameter sets. KernelTest was written to automate away almost all of that
tedium.

Very, very recent clangs (>= 4.0) have gained the ability to instrument your code with calls informing
you of why the execution flow changed as execution progresses. You simply must compile your
piece of test code with `-fsanitize-coverage=trace-pc,indirect-calls -fsanitize-coverage=trace-cmp -fsanitize-coverage=trace-div -fsanitize-coverage=trace-gep`
and clang will emit these functions:

`void __sanitizer_cov_trace_pc()`
`void __sanitizer_cov_trace_pc_indirect(void *callee)`

Called per edge executed. Edge can be identified via `__builtin_return_address(0)`.

`void __sanitizer_cov_trace_cmp1(uint8_t Arg1, uint8_t Arg2)`
`void __sanitizer_cov_trace_cmp2(uint16_t Arg1, uint16_t Arg2)`
`void __sanitizer_cov_trace_cmp4(uint32_t Arg1, uint32_t Arg2)`
`void __sanitizer_cov_trace_cmp8(uint64_t Arg1, uint64_t Arg2)`

Called when a comparison is about to be performed which will determine which edge
to execute next.

`void __sanitizer_cov_trace_switch(uint64_t Val, uint64_t *Cases)`

Called when a comparison of a value to a list of values is about to performed,
again determining what the next edge to be executed will be. Note if() statements
comparing a single variable to multiple items are collapsed into one of these.

`void __sanitizer_cov_trace_div4(uint32_t Val)`
`void __sanitizer_cov_trace_div8(uint64_t Val)`

Called when an integer divide is about to happen.

`void __sanitizer_cov_trace_gep(uintptr_t Idx)`

Called when an array index is about to happen.


There is also another sanitiser `-fsanitize=dataflow` which lets you taint each input
parameter and clang will propagate that taint throughout a piece of code. This lets
you determine that variables calculated from input parameters depend on those parameters.


This instrumentation is meant for fuzzers, but it nearly offers enough to do a
poor man's **symbolic execution engine** i.e. to determine what inputs cause what parts
of a program to execute, and thereby automatically generate a minimum set of
parameter call sets for some function which exercise every possible control
flow in the potential execution graph. That minimum set of parameter call sets
can then be automatically inserted by KernelTest into its kernel unit test runner
call lists which can be customised by hand where appropriate. A build tooling pass
or CI pass can also keep those test runner call tests fresh as the code changes.

By "poor man'", as you'll see next I really do mean "nearly offers enough" because this is a
very inefficient way of implementing a unit test case generator as compared to a
proper SMT solving symbolic execution engine like say KLEE (https://klee.github.io/).

Imagine for example this real world function with all its prerequisites minimally
pasted in front:

```c++
/* A small standalone program to test whether the permuter works
*/

#include "../outcome/include/outcome.hpp"

#include "../quickcpplib/include/bitfield.hpp"
#include <filesystem>

namespace minimal_afio
{
  using namespace OUTCOME_V2_NAMESPACE;
  struct native_handle_type
  {
    HANDLE h;
    native_handle_type()
        : h(nullptr)
    {
    }
  };
  struct file_handle
  {
    using path_type = std::experimental::filesystem::path;
    enum class mode : unsigned char
    {
      unchanged = 0,
      none = 2,        //!< No ability to read or write anything, but can synchronise (SYNCHRONIZE or 0)
      attr_read = 4,   //!< Ability to read attributes (FILE_READ_ATTRIBUTES|SYNCHRONIZE or O_RDONLY)
      attr_write = 5,  //!< Ability to read and write attributes (FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES|SYNCHRONIZE or O_RDONLY)
      read = 6,        //!< Ability to read (READ_CONTROL|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|SYNCHRONISE or O_RDONLY)
      write = 7,       //!< Ability to read and write (READ_CONTROL|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|FILE_WRITE_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA|FILE_APPEND_DATA|SYNCHRONISE or O_RDWR)
      append = 9       //!< All mainstream OSs and CIFS guarantee this is atomic with respect to all other appenders (FILE_APPEND_DATA|SYNCHRONISE or O_APPEND)
    };
    enum class creation : unsigned char
    {
      open_existing = 0,
      only_if_not_exist,
      if_needed,
      truncate  //!< Atomically truncate on open, leaving creation date unmodified.
    };
    enum class caching : unsigned char  // bit 0 set means safety fsyncs enabled
    {
      unchanged = 0,
      none = 1,                //!< No caching whatsoever, all reads and writes come from storage (i.e. <tt>O_DIRECT|O_SYNC</tt>). Align all i/o to 4Kb boundaries for this to work. <tt>flag_disable_safety_fsyncs</tt> can be used here.
      only_metadata = 2,       //!< Cache reads and writes of metadata but avoid caching data (<tt>O_DIRECT</tt>), thus i/o here does not affect other cached data for other handles. Align all i/o to 4Kb boundaries for this to work.
      reads = 3,               //!< Cache reads only. Writes of data and metadata do not complete until reaching storage (<tt>O_SYNC</tt>). <tt>flag_disable_safety_fsyncs</tt> can be used here.
      reads_and_metadata = 5,  //!< Cache reads and writes of metadata, but writes of data do not complete until reaching storage (<tt>O_DSYNC</tt>). <tt>flag_disable_safety_fsyncs</tt> can be used here.
      all = 4,                 //!< Cache reads and writes of data and metadata so they complete immediately, sending writes to storage at some point when the kernel decides (this is the default file system caching on a system).
      safety_fsyncs = 7,       //!< Cache reads and writes of data and metadata so they complete immediately, but issue safety fsyncs at certain points. See documentation for <tt>flag_disable_safety_fsyncs</tt>.
      temporary = 6            //!< Cache reads and writes of data and metadata so they complete immediately, only sending any updates to storage on last handle close in the system or if memory becomes tight as this file is expected to be temporary (Windows only).
                               // NOTE: IF UPDATING THIS UPDATE THE std::ostream PRINTER BELOW!!!
    };
    QUICKCPPLIB_BITFIELD_BEGIN(flag)
    {
      none = 0,  //!< No flags
      unlink_on_close = 1 << 0, disable_safety_fsyncs = 1 << 2, disable_safety_unlinks = 1 << 3,

      win_disable_unlink_emulation = 1 << 24,  //!< See the documentation for `unlink_on_close`

      overlapped = 1 << 28,         //!< On Windows, create any new handles with OVERLAPPED semantics
      byte_lock_insanity = 1 << 29  //!< Using insane POSIX byte range locks
    }
    QUICKCPPLIB_BITFIELD_END(flag)
    file_handle(path_type path, caching, flag)
        : _path(std::move(path))

    {
    }
    native_handle_type _v;
    path_type _path;
  };
  using handle = file_handle;
  static inline result<ACCESS_MASK> access_mask_from_handle_mode(handle::mode _mode)
  {
    ACCESS_MASK access = SYNCHRONIZE;
    switch(_mode)
    {
    case handle::mode::unchanged:
      return make_errored_result<ACCESS_MASK>(EINVAL);
    case handle::mode::none:
      break;
    case handle::mode::attr_read:
      access |= FILE_READ_ATTRIBUTES;
      break;
    case handle::mode::attr_write:
      access |= FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
      break;
    case handle::mode::read:
      access |= GENERIC_READ;
      break;
    case handle::mode::write:
      access |= GENERIC_WRITE | GENERIC_READ;
      break;
    case handle::mode::append:
      access |= FILE_APPEND_DATA;
      break;
    }
    return access;
  }
  static inline result<DWORD> attributes_from_handle_caching_and_flags(handle::caching _caching, handle::flag flags)
  {
    DWORD attribs = 0;
    if(flags & handle::flag::overlapped)
    {
      attribs |= FILE_FLAG_OVERLAPPED;
    }
    switch(_caching)
    {
    case handle::caching::unchanged:
      return make_errored_result<DWORD>(EINVAL);
    case handle::caching::none:
      attribs |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
      break;
    case handle::caching::only_metadata:
      attribs |= FILE_FLAG_NO_BUFFERING;
      break;
    case handle::caching::reads:
    case handle::caching::reads_and_metadata:
      attribs |= FILE_FLAG_WRITE_THROUGH;
      break;
    case handle::caching::all:
    case handle::caching::safety_fsyncs:
      break;
    case handle::caching::temporary:
      attribs |= FILE_ATTRIBUTE_TEMPORARY;
      break;
    }
    if(flags & handle::flag::unlink_on_close)
      attribs |= FILE_FLAG_DELETE_ON_CLOSE;
    return attribs;
  }
}

namespace file_create
{
  using namespace minimal_afio;
  using QUICKCPPLIB_NAMESPACE::ringbuffer_log::last190;
  extern BOOST_SYMBOL_EXPORT result<file_handle> test_file_create(file_handle::path_type _path, file_handle::mode _mode = file_handle::mode::read, file_handle::creation _creation = file_handle::creation::open_existing, file_handle::caching _caching = file_handle::caching::all,
                                                                  file_handle::flag flags = file_handle::flag::none) noexcept
  {
    result<file_handle> ret(file_handle(std::move(_path), _caching, flags));
    native_handle_type &nativeh = ret.get()._v;
    BOOST_OUTCOME_FILTER_ERROR(access, access_mask_from_handle_mode(_mode));
    DWORD creation = OPEN_EXISTING;
    switch(_creation)
    {
    case file_handle::creation::open_existing:
      break;
    case file_handle::creation::only_if_not_exist:
      creation = CREATE_NEW;
      break;
    case file_handle::creation::if_needed:
      creation = OPEN_ALWAYS;
      break;
    case file_handle::creation::truncate:
      creation = TRUNCATE_EXISTING;
      break;
    }
    BOOST_OUTCOME_FILTER_ERROR(attribs, attributes_from_handle_caching_and_flags(_caching, flags));
    if(INVALID_HANDLE_VALUE == (nativeh.h = CreateFile(ret.value()._path.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, creation, attribs, NULL)))
    {
      DWORD errcode = GetLastError();
      // assert(false);
      return make_errored_result<file_handle>(errcode, last190(ret.value()._path.u8string()));
    }
    if(flags & file_handle::flag::unlink_on_close)
    {
      // Hide this item
      SetFileAttributes(ret.value()._path.c_str(), FILE_ATTRIBUTE_HIDDEN);
    }
    return ret;
  }
}
```

The above is a standalone program, and the function we want to automatically generate unit tests for is
`test_file_create()`. A future clang AST parser will inspect the function being tested and figure out
from the AST a set of seed value permutations. In the case above, most of the parameters are enums of
some kind, so a complete list of call permutations can be generated just by the AST parser alone. But
let's assume for a minute that at least one of those parameters was just a simple signed integer where
its negativity, zero and the range 1-5 and 6 to `INT_MAX` are the four possible minimum variations which
cause the function to do something different.

You can see in the source code that `test_file_create()`'s parameters are transformed into parameters
with which to call `CreateFile()`. Via the clang instrumentation, we can permute say the mode parameter
and notice that one or many of the `__sanitizer_cov_trace_cmp1()` and/or `__sanitizer_cov_trace_switch()`
calls will always be passed that same input value and/or change when the input changes (e.g. a bitfield
or a value calculated from the input). If the input was compared unmodified, we can then assemble a list
of all the values that that mode parameter is ever compared to on the basis that each of those values,
and those values plus or minus 1 is highly likely to cause new edges to get executed, thus illuminating
more new potential parameter values to test.

This works well for simple integer parameters which represent some unique number meaning something,
and in fact of course mode is actually an enum and therefore only has a small set of valid states
anyway. The clang AST parser should be able to spot enums which are bitfields and tag them appropriately
for permutation, so that covers pretty well the simple stuff.

Things get harder for where calculations are performed onto an input before it is compared. Unless
`-fsanitize=dataflow` is enabled in which case we can taint each input parameter and the taint
will follow onto any calculated variables, we cannot necessarily know that a relationship exists
between a value calculated from input parameters and those input parameters. Even with tainting
on, we may know a relationship exists but have no idea what it is, it could be highly non linear
or any old weird thing. Here simple brute force comes into play, we split the valid input range
into two powers ranges, and if there is change between those we split each range in half again and so
on, effectively performing a binary search for those input values which cause a compared value to change.
_The chances are_ that most calculations from input parameters to compared parameters which don't use
divide nor an array index is going to be linear and so this divide and conquer technique will work,
but I can see some corner cases might slip through. They ought to be fairly rare however.

You can probably see by now that this problem is ultimately recursive, and the total set of
permutations which must be tried rises very quickly when input parameters have any range on them
as total valid permutations is two to the power of the bits of free range. To try and keep down
the bits of free range, one can mark up individual parameters with valid ranges using C++ attributes
and the clang AST parser will ensure that the permuter limits itself to that range.

Where KernelTest doesn't do well is on stuff like string input where the set of unique and potentially
meaningful permutations rises very quickly and this poor man's symbolic execution engine begins
to fall down quite badly. KernelTest takes the view that you the user needs to use a proper fuzzer
with a genetic learning algorithm for string inputs, and so therefore ignores those entirely.

KernelTest isn't capable of performing magic. It can only figure out, via brute force, the total
permutations of a test kernel which you the programmer necessarily will have to carefully write in
a fashion which limits the exercise of the APIs it calls. That will leave unit test coverage gaps as wide
as you are unwilling to introduce mocks for syscalls, or parameter ranges you overconstrain, or
the time you allow the brute force program to run and so on.

Nevertheless, with careful test kernel design KernelTest ought to automate the lion's share of the
tedium of writing unit test code for your APIs and achieving a test coverage much higher than an
equivalent amount of programmer effort without KernelTest would achieve.
