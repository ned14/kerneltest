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
static constexpr const char __integration_test_kernel_category[] = #category;                                                                                                                                                                                                                                                  \
    \
static constexpr const char __integration_test_kernel_product[] = #product;                                                                                                                                                                                                                                                    \
    \
static constexpr const char __integration_test_kernel_test[] = #test;                                                                                                                                                                                                                                                          \
    \
static constexpr const char __integration_test_kernel_name[] = #name;                                                                                                                                                                                                                                                          \
    \
static constexpr const char __integration_test_kernel_description[] = desc;                                                                                                                                                                                                                                                    \
    using namespace BOOST_KERNELTEST_V1_NAMESPACE;                                                                                                                                                                                                                                                                             \
    \
std::cout                                                                                                                                                                                                                                                                                                                      \
    << "\n\n"                                                                                                                                                                                                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::blue << __integration_test_kernel_category << "/" << __integration_test_kernel_product << "/" << __integration_test_kernel_test << "/" << __integration_test_kernel_name << ":\n"                                                                                                                                  \
    << boost_lite::console_colours::bold << boost_lite::console_colours::white << desc << boost_lite::console_colours::normal << std::endl;                                                                                                                                                                                                                        \
    \
__VA_ARGS__;                                                                                                                                                                                                                                                                                                                   \
  }
// clang-format on


BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
template <class T> inline void print_result(bool v, const T &result)
{
  using namespace boost_lite::console_colours;
  if(v)
    std::cout << bold << green << result << normal << std::endl;
  else
    std::cout << bold << red << "FAILED" << normal << std::endl;
}
BOOST_KERNELTEST_V1_NAMESPACE_END

#define BOOST_KERNELTEST_CHECK_RESULT(r) BOOST_CHECK(r.has_value())

#endif  // namespace
