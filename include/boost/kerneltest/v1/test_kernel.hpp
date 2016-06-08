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
