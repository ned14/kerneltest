/* Routines for handling child processes
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
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

#include "../../child_process.hpp"

#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)  // use of posix apis
#endif

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

namespace child_process
{
  void child_process::_initialise_files() const
  {
    if(_stdin)
      return;
    int ih = _readh.fd, oh = _writeh.fd, eh = _errh.fd;
#ifdef _MSC_VER
    ih = _open_osfhandle((intptr_t) _readh.h, O_APPEND);
    oh = _open_osfhandle((intptr_t) _writeh.h, O_RDONLY);
    if(!_use_parent_errh)
      eh = _open_osfhandle((intptr_t) _errh.h, O_RDONLY);
#endif
    const_cast<child_process *>(this)->_stdin = fdopen(ih, "a");
    const_cast<child_process *>(this)->_stdout = fdopen(oh, "r");
    if(!_use_parent_errh)
      const_cast<child_process *>(this)->_stderr = fdopen(eh, "r");
  }
  void child_process::_deinitialise_files()
  {
    if(!_stdin)
      return;
    fclose(_stdin);
    fclose(_stdout);
    if(!_use_parent_errh)
      fclose(_stderr);
// These are now closed
#ifdef _MSC_VER
    _readh.h = nullptr;
    _writeh.h = nullptr;
    _errh.h = nullptr;
#else
    _readh.fd = 0;
    _writeh.fd = 0;
    _errh.fd = 0;
#endif
  }

  // The following is borrowed from http://www.josuttis.com/cppcode/fdstream.hpp.html

  /************************************************************
  * fdostream
  * - a stream that writes on a file descriptor
  ************************************************************/

  class fdoutbuf : public std::streambuf
  {
  protected:
    int fd;  // file descriptor
  public:
    // constructor
    fdoutbuf(int _fd)
        : fd(_fd)
    {
    }

  protected:
    // write one character
    virtual int_type overflow(int_type c)
    {
      if(c != EOF)
      {
        char z = (char) c;
        if(write(fd, &z, 1) != 1)
        {
          return EOF;
        }
      }
      return c;
    }
    // write multiple characters
    virtual std::streamsize xsputn(const char *s, std::streamsize num) { return write(fd, s, (unsigned) num); }
  };

  class fdostream : public std::ostream
  {
  protected:
    fdoutbuf buf;

  public:
    int fd;
    fdostream(int _fd)
        : std::ostream(0)
        , buf(_fd)
        , fd(_fd)
    {
      rdbuf(&buf);
    }
  };


  /************************************************************
  * fdistream
  * - a stream that reads on a file descriptor
  ************************************************************/

  class fdinbuf : public std::streambuf
  {
  protected:
    int fd;  // file descriptor
  protected:
    /* data buffer:
    * - at most, pbSize characters in putback area plus
    * - at most, bufSize characters in ordinary read buffer
    */
    static const size_t pbSize = 4;      // size of putback area
    static const size_t bufSize = 1024;  // size of the data buffer
    char buffer[bufSize + pbSize];       // data buffer

  public:
    /* constructor
    * - initialize file descriptor
    * - initialize empty data buffer
    * - no putback area
    * => force underflow()
    */
    fdinbuf(int _fd)
        : fd(_fd)
    {
      setg(buffer + pbSize,   // beginning of putback area
           buffer + pbSize,   // read position
           buffer + pbSize);  // end position
    }

  protected:
    // insert new characters into the buffer
    virtual int_type underflow()
    {
#ifndef _MSC_VER
      using std::memmove;
#endif

      // is read position before end of buffer?
      if(gptr() < egptr())
      {
        return traits_type::to_int_type(*gptr());
      }

      /* process size of putback area
      * - use number of characters read
      * - but at most size of putback area
      */
      size_t numPutback;
      numPutback = gptr() - eback();
      if(numPutback > pbSize)
      {
        numPutback = pbSize;
      }

      /* copy up to pbSize characters previously read into
      * the putback area
      */
      memmove(buffer + (pbSize - numPutback), gptr() - numPutback, numPutback);

      // read at most bufSize new characters
      int num;
      num = read(fd, buffer + pbSize, bufSize);
      if(num <= 0)
      {
        // ERROR or EOF
        return EOF;
      }

      // reset buffer pointers
      setg(buffer + (pbSize - numPutback),  // beginning of putback area
           buffer + pbSize,                 // read position
           buffer + pbSize + num);          // end of buffer

      // return next character
      return traits_type::to_int_type(*gptr());
    }
  };

  class fdistream : public std::istream
  {
  protected:
    fdinbuf buf;

  public:
    int fd;
    fdistream(int _fd)
        : std::istream(0)
        , buf(_fd)
        , fd(_fd)
    {
      rdbuf(&buf);
    }
  };

  void child_process::_initialise_streams() const
  {
    int ih = _readh.fd, oh = _writeh.fd, eh = _errh.fd;
#ifdef _MSC_VER
    ih = _open_osfhandle((intptr_t) _readh.h, O_APPEND);
    oh = _open_osfhandle((intptr_t) _writeh.h, O_RDONLY);
    if(!_use_parent_errh)
      eh = _open_osfhandle((intptr_t) _errh.h, O_RDONLY);
#endif
    const_cast<child_process *>(this)->_cin = new fdostream(ih);
    const_cast<child_process *>(this)->_cout = new fdistream(oh);
    if(!_use_parent_errh)
      const_cast<child_process *>(this)->_cerr = new fdistream(eh);
  }
  void child_process::_deinitialise_streams()
  {
#ifdef _MSC_VER
    if(_cin)
    {
      _close(static_cast<fdostream *>(_cin)->fd);
      _close(static_cast<fdistream *>(_cout)->fd);
      if(!_use_parent_errh)
        _close(static_cast<fdistream *>(_cerr)->fd);
    }
#endif
    delete _cin;
    _cin = nullptr;
    delete _cout;
    _cout = nullptr;
    delete _cerr;
    _cerr = nullptr;
  }
}

BOOST_KERNELTEST_V1_NAMESPACE_END

#ifdef _WIN32
#include "windows/child_process.ipp"
#else
#include "posix/child_process.ipp"
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
