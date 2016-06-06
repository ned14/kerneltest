/* test_kernel
(C) 2016 Niall Douglas http://www.nedproductions.biz/
File Created: Apr 2016
*/

#include "../boost-lite/include/console_colours.hpp"
#include "config.hpp"
//#define BOOST_CATCH_CUSTOM_MAIN_DEFINED
#include "../boost-lite/include/boost/test/unit_test.hpp"

#ifndef BOOST_KERNELTEST_TEST_KERNEL_HPP
#define BOOST_KERNELTEST_TEST_KERNEL_HPP

/*! \brief Implement a test kernel
\param category This category of test kernel. Typically 'integration'.
\param product Name of the product or library being tested.
\param test The name of this test kernel.
\param name The name of this test.
\param desc A pretty printable description of the test.
\param ... Code implementing the test kernel.
*/
// clang-format off
#define BOOST_KERNELTEST_TEST_KERNEL(category, product, test, name, desc, ...)                                                                                                                                                                                                                                                 \
  \
BOOST_AUTO_TEST_CASE(category/product/test/name, desc)                                                                                                                                                                                                                                                                         \
  {                                                                                                                                                                                                                                                                                                                            \
    \
    using namespace BOOST_KERNELTEST_V1_NAMESPACE;                                                                                                                                                                                                                                                                             \
current_test_kernel.category=#category; \
current_test_kernel.product=#product; \
current_test_kernel.test=#test; \
current_test_kernel.name=#name; \
current_test_kernel.desc=desc; \
    \
std::cout                                                                                                                                                                                                                                                                                                                      \
    << "\n\n"                                                                                                                                                                                                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::blue << __integration_test_kernel_category << "/" << __integration_test_kernel_product << "/" << __integration_test_kernel_test << "/" << __integration_test_kernel_name << ":\n"                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::white << desc << boost_lite::console_colours::normal << std::endl;                                                                                                                                                                                                                        \
    \
__VA_ARGS__;                                                                                                                                                                                                                                                                                                                   \
  }
// clang-format on

#endif  // namespace
