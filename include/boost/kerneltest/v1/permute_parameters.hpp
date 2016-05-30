/* permute_parameters
(C) 2016 Niall Douglas http://www.nedprod.com/
File Created: Apr 2016
*/

#include "config.hpp"

#ifndef BOOST_KERNELTEST_PERMUTE_PARAMETERS_HPP
#define BOOST_KERNELTEST_PERMUTE_PARAMETERS_HPP

// We need an aggregate initialisable collection of heterogeneous types
#if __cplusplus >= 20170000L
#include <tuple>
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
template <class... Types> using parameters = std::tuple<Types...>;
BOOST_KERNELTEST_V1_NAMESPACE_END
#else
#include "../boost-lite/include/atuple.hpp"
BOOST_KERNELTEST_V1_NAMESPACE_BEGIN
template <class... Types> using parameters = boost_lite::aggregate_tuple::atuple<Types...>;
BOOST_KERNELTEST_V1_NAMESPACE_END
#endif

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN

template <class OutcomeType, class... InputTypes, class Container, class... Hooks> parameter_permuter<OutcomeType, InputTypes...> mt_permute_parameters(Container &&cont, Hooks &&... hooks);

/*! \brief A parameter permuter instance
\tparam is_mt True if this is a multithreaded parameter permuter
\tparam OutcomeType An outcome<T>, result<T> or option<T> for the outcome of the test kernel
\tparam InputTypes The types of the parameters of the test kernel
\tparam Container The type of the container containing the parameter sets
\tparam Hooks The types of any pretest or posttest hooks
*/
template <bool is_mt, class Sequence> class parameter_permuter
{
  template <class OutcomeType, class... InputTypes, template <class...> class Sequence, class... Hooks> friend auto mt_permute_parameters(Sequence<parameters<OutcomeType, InputTypes...>> &&seq, Hooks &&... hooks);
  Sequence params;

  parameter_permuter(Sequence &&_params)
      : params(std::move(_params))
  {
  }

public:
  //! Permute the callable f with this parameter permuter
  template <class U> void operator()(U &&f) noexcept {}
};

/*! \brief Create a multithreaded parameter permuter
\tparam OutcomeType An outcome<T>, result<T> or option<T> for the outcome of the test kernel
\tparam InputTypes The types of the parameters of the test kernel
\tparam Sequence The type of the sequence containing the parameter sets
\tparam Hooks The types of any pretest or posttest hooks
\param seq The sequence of parameter sets
\param hooks Any pretest or posttest hooks
*/
template <class OutcomeType, class... InputTypes, template <class...> class Sequence, class... Hooks> auto mt_permute_parameters(Sequence<parameters<OutcomeType, InputTypes...>> &&seq, Hooks &&... hooks)
{
  using ContainerType = typename std::decay<decltype(seq)>::type;
  parameter_permuter<true, ContainerType> ret(std::move(seq));
  hooks(ret)...;
  return ret;
}

BOOST_KERNELTEST_V1_NAMESPACE_END

#if 0
\
static const BOOST_OUTCOME_V1_NAMESPACE::integration_test::parameters_type<decltype(__param), BOOST_OUTCOME_INTEGRATION_TEST_REMOVE_BRACKETS __outcometype>
__outcomes = BOOST_OUTCOME_INTEGRATION_TEST_REMOVE_BRACKETS __outcomes_initialiser;
\
size_t __no = 0;
\
for(const auto &__outcome
    : __outcomes)
\
{
  \
BOOST_OUTCOME_V1_NAMESPACE::integration_test::filesystem_workspace __workspace((__testdir), __outcome, __no++, __outcomes.size());
  \
(__param) = __outcome.parameter_value;
  \
__VA_ARGS__
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

#endif  // temporary

#endif  // namespace
