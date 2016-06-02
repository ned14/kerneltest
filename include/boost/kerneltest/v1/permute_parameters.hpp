/* permute_parameters
(C) 2016 Niall Douglas http://www.nedprod.com/
File Created: Apr 2016
*/

#include "config.hpp"

#ifndef BOOST_KERNELTEST_PERMUTE_PARAMETERS_HPP
#define BOOST_KERNELTEST_PERMUTE_PARAMETERS_HPP

BOOST_KERNELTEST_V1_NAMESPACE_BEGIN


/*! \brief A parameter permuter instance
\tparam is_mt True if this is a multithreaded parameter permuter
\tparam ParamSequence A sequence of parameter calls
*/
template <bool is_mt, class ParamSequence> class parameter_permuter
{
  ParamSequence params;

public:
  parameter_permuter(ParamSequence &&_params)
      : params(std::move(_params))
  {
  }

  //! Permute the callable f with this parameter permuter
  template <class U> void operator()(U &&f) noexcept
  {
    // Make an array of outcome<decltype(f())>
    // Loop params, instantiating all hooks before each kernel call
    // Trap any exception throws into the outcome
    // If we are multithreaded, use threads :)
  }
};

namespace detail
{
  template <class ParamSequence, class OutcomeType, class... Parameters> struct is_parameters_sequence_type_valid : std::false_type
  {
  };
  template <class OutcomeType, class... Parameters, template <class...> class Container> struct is_parameters_sequence_type_valid<Container<parameters<OutcomeType, Parameters...>>, OutcomeType, Parameters...> : std::true_type
  {
  };
}

/*! \brief Create a multithreaded parameter permuter
\tparam OutcomeType An outcome<T>, result<T> or option<T> for the outcome of the test kernel
\tparam InputTypes The types of the parameters of the test kernel
\tparam Sequence The type of the sequence containing the parameter sets
\tparam Hooks The types of any pretest or posttest hooks
\param seq The sequence of parameter sets
*/
template <class OutcomeType, class... Parameters, class Sequence, class... Hooks, typename = typename std::enable_if<detail::is_parameters_sequence_type_valid<Sequence, OutcomeType, Parameters...>::value>::type> parameter_permuter<true, Sequence> mt_permute_parameters(Sequence &&seq, Hooks &&... hooks)
{
  parameter_permuter<true, Sequence> ret(std::forward<Sequence>(seq));
  return ret;
}
//! \overload
template <class... Parameters, class... Hooks> auto mt_permute_parameters(std::initializer_list<parameters<Parameters...>> seq, Hooks &&... hooks)
{
  parameter_permuter<true, std::initializer_list<parameters<Parameters...>>> ret(std::move(seq));
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
