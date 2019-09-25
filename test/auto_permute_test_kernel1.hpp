/* A small standalone program to test whether the permuter works
*/

#include "outcome/outcome.hpp"
#include <fcntl.h>
#include <memory>
#include <string>
#include <unistd.h>

namespace file_create
{
  using namespace OUTCOME_V2_NAMESPACE;
  struct handle_type
  {
    int fd;
    handle_type(int _fd)
        : fd(_fd)
    {
    }
    ~handle_type() { ::close(fd); }
  };
  extern BOOST_SYMBOL_EXPORT outcome<std::shared_ptr<handle_type>> openfile(std::string path) noexcept
  {
    try
    {
      int fd;
      while(-1 == (fd = ::open(path.c_str(), 0)) && EINTR == errno)
        ;
      if(-1 == fd)
      {
        int code = errno;
        // If a temporary failure, this is an expected unexpected outcome
        if(EBUSY == code || EISDIR == code || ELOOP == code || ENOENT == code || ENOTDIR == code || EPERM == code || EACCES == code)
          return error_code_extended(code, std::generic_category());

        // If a non-temporary failure, this is an unexpected outcome
        return std::make_exception_ptr(std::system_error(code, std::generic_category(), strerror(code)));
      }
      return std::make_shared<handle_type>(fd);
    }
    catch(...)
    {
      // Any exception thrown is truly unexpected
      return std::current_exception();
    }
  }
}
