/* test_kernel
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: Apr 2016


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

#include "config.hpp"

#ifndef BOOST_KERNELTEST_TEST_KERNEL_HPP
#define BOOST_KERNELTEST_TEST_KERNEL_HPP

//#define BOOST_CATCH_CUSTOM_MAIN_DEFINED
#include "../boost-lite/include/boost/test/unit_test.hpp"
#include "../boost-lite/include/console_colours.hpp"

#define BOOST_KERNELTEST_TEST_UNIQUE_NAME2(a, b) a##b
#define BOOST_KERNELTEST_TEST_UNIQUE_NAME1(a, b) BOOST_KERNELTEST_TEST_UNIQUE_NAME2(a, b)
//! \brief A macro expanding into a unique identifier (for this compilation unit)
#define BOOST_KERNELTEST_TEST_UNIQUE_NAME(prefix) BOOST_KERNELTEST_TEST_UNIQUE_NAME1(prefix, __COUNTER__)

/*! \brief Implement a test kernel
\param category This category of test kernel. Typically 'unit' or 'integration'.
\param product Name of the product or library being tested.
\param test The name of this test kernel.
\param name The name of this test.
\param desc A pretty printable description of the test.
\param ... Code implementing the test kernel.
*/
// clang-format off
#define BOOST_KERNELTEST_TEST_KERNEL(_category, _product, _test, _name, _desc, ...)                                                                                                                                                                                                                                                 \
  \
BOOST_AUTO_TEST_CASE(_category/_product/_test/_name, _desc)                                                                                                                                                                                                                                                                         \
  {                                                                                                                                                                                                                                                                                                                            \
    \
    using namespace BOOST_KERNELTEST_V1_NAMESPACE;                                                                                                                                                                                                                                                                             \
current_test_kernel.category=#_category; \
current_test_kernel.product=#_product; \
current_test_kernel.test=#_test; \
current_test_kernel.name=#_name; \
current_test_kernel.description=_desc; \
    \
BOOST_KERNELTEST_COUT("\n\n"                                                                                                                                                                                                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::blue << #_category "/" #_product "/" #_test "/" #_name ":\n"                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::white << _desc << boost_lite::console_colours::normal << std::endl);                                                                                                                                                                                                                        \
    \
__VA_ARGS__;                                                                                                                                                                                                                                                                                                                   \
current_test_kernel.category=nullptr; \
current_test_kernel.product=nullptr; \
current_test_kernel.test=nullptr; \
current_test_kernel.name=nullptr; \
current_test_kernel.description=nullptr; \
  }
// clang-format on

#endif  // namespace
