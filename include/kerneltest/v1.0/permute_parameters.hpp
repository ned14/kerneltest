/* permute_parameters
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (10 commits)
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

#ifndef KERNELTEST_PERMUTE_PARAMETERS_HPP
#define KERNELTEST_PERMUTE_PARAMETERS_HPP

#include "quickcpplib/console_colours.hpp"
#include "quickcpplib/type_traits.hpp"

#include <array>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6326)  // comparison of constants
#endif

KERNELTEST_V1_NAMESPACE_BEGIN

namespace detail
{
  template <class T> struct has_constant_size
  {
    static constexpr bool value = false;
    static constexpr size_t size = 0;
  };
  template <class T, size_t N> struct has_constant_size<std::array<T, N>>
  {
    static constexpr bool value = true;
    static constexpr size_t size = N;
  };
  template <class ParamSequence, bool = has_constant_size<ParamSequence>::value> struct permutation_results_type
  {
    template <class T> using type = std::vector<optional<T>>;
    constexpr ParamSequence operator()(size_t no) const { return ParamSequence(no); }
  };
  template <class ParamSequence> struct permutation_results_type<ParamSequence, true>
  {
    template <class T> using type = std::array<optional<T>, has_constant_size<ParamSequence>::size>;
    constexpr ParamSequence operator()(size_t) const { return ParamSequence(); }
  };
  template <class T> constexpr T make_permutation_results_type(size_t no) { return permutation_results_type<T>()(no); }

  template <class ParamSequence, class Callable> struct result_of_parameter_permute;
  template <class OutcomeType, class... Types, class... Excess, class Callable> struct result_of_parameter_permute<parameters<OutcomeType, parameters<Types...>, Excess...>, Callable>
  {
    using type = decltype(std::declval<Callable>()(std::declval<Types>()...));
  };

  // Need a tuple whose destruction order is well known. This fellow destructs in
  // reverse order of Ts...
  template <class... Ts> struct hooks_container;
  template <class T, class... Ts> struct hooks_container<T, Ts...>
  {
    T _v;
    hooks_container<Ts...> _vs;
    constexpr hooks_container(T &&v, Ts &&... vs)
        : _v(std::move(v))
        , _vs(std::move(vs)...)
    {
    }
  };
  template <class T> struct hooks_container<T>
  {
    T _v;
    constexpr hooks_container(T &&v)
        : _v(std::move(v))
    {
    }
  };
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unreferenced formal parameter
#endif
  template <class Hook, class Permuter, class Outcome, class... Types, size_t... Idxs> auto instantiate_hook(Hook &&hook, Permuter *parent, Outcome &out, size_t idx, const std::tuple<Types...> &pars, std::index_sequence<Idxs...>) { return hook(parent, out, idx, std::get<Idxs>(pars)...); }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  template <class... Hooks, class Permuter, class Outcome, class ParamSequence, size_t... Idxs> auto instantiate_hooks(const std::tuple<Hooks...> &hooks, Permuter *parent, Outcome &out, size_t idx, const ParamSequence &pars, std::index_sequence<Idxs...>)
  {
    // callspec is (parameter_permuter<...> *parent, outcome<T> &testret, size_t, pars)
    // pars<0> is expected outcome, pars<1> is kernel parameter set. pars<2> onwards are the hook parameters
    //
    // Cannot use tuple because can't guarantee order of destruction, it varies by
    // STL implementation
    return hooks_container<decltype(instantiate_hook(std::get<Idxs>(hooks), parent, out, idx, std::get<2 + Idxs>(pars), std::make_index_sequence<parameters_size<typename parameters_element<2 + Idxs, ParamSequence>::type>::value>()))...>{
    instantiate_hook(std::get<Idxs>(hooks), parent, out, idx, std::get<2 + Idxs>(pars), std::make_index_sequence<parameters_size<typename parameters_element<2 + Idxs, ParamSequence>::type>::value>())...};
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unreferenced formal parameter
#endif
  template <class U, class... Types, size_t... Idxs> auto call_f_with_parameters(U &&f, const parameters<Types...> &params, std::index_sequence<Idxs...>) { return f(std::get<Idxs>(params)...); }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  template <class U, class... Types, size_t... Idxs> auto call_f_with_tuple(U &&f, const std::tuple<Types...> &params, std::index_sequence<Idxs...>) { return f(std::get<Idxs>(params)...); }

  template <class T, class U, class V, class A, class B, class C> bool check_result(const optional<outcome<T, U, V>> &kernel_outcome, const outcome<A, B, C> &shouldbe) { return kernel_outcome.value() == shouldbe; };
  template <class T, class U, class A, class B> bool check_result(const optional<result<T, U>> &kernel_outcome, const result<A, B> &shouldbe) { return kernel_outcome.value() == shouldbe; };

  // If should be has type void, we only care kernel_outcome has a value
  template <class T, class U, class V, class B, class C> bool check_result(const optional<outcome<T, U, V>> &kernel_outcome, const outcome<void, B, C> &shouldbe)
  {
    if(kernel_outcome.value().has_value() && shouldbe.has_value())
      return kernel_outcome.value().has_value() == shouldbe.has_value();
    else if(shouldbe.has_error() && kernel_outcome.value().has_error())
    {
#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
      const auto &kernel_outcome_sc = kernel_outcome.value().error();
      auto shouldbe_sc = shouldbe.error();
      // match errors for semantic equivalence
      if(kernel_outcome_sc == shouldbe_sc)
        return true;
#else
      std::error_code kernel_outcome_ec = OUTCOME_V2_NAMESPACE::policy::error_code(kernel_outcome.value().error());
      std::error_code shouldbe_ec = OUTCOME_V2_NAMESPACE::policy::error_code(shouldbe.error());
      // match errors for semantic equivalence
      if(kernel_outcome_ec.default_error_condition() == shouldbe_ec)
        return true;
#ifndef _WIN32
      // Some POSIX STLs don't map system_category to generic_category, which is stupid
      if(kernel_outcome_ec.category() == std::system_category() && shouldbe_ec.category() == std::generic_category())
        return kernel_outcome_ec.value() == shouldbe_ec.value();
#endif
#endif
      return false;
    }
    else
      return false;
  };
  template <class T, class U, class B> bool check_result(const optional<result<T, U>> &kernel_outcome, const result<void, B> &shouldbe)
  {
    if(kernel_outcome.value().has_value() && shouldbe.has_value())
      return kernel_outcome.value().has_value() == shouldbe.has_value();
    else if(shouldbe.has_error() && kernel_outcome.value().has_error())
    {
#if KERNELTEST_EXPERIMENTAL_STATUS_CODE
      const auto &kernel_outcome_sc = kernel_outcome.value().error();
      auto shouldbe_sc = shouldbe.error();
      // match errors for semantic equivalence
      if(kernel_outcome_sc == shouldbe_sc)
        return true;
#else
      std::error_code kernel_outcome_ec = OUTCOME_V2_NAMESPACE::policy::error_code(kernel_outcome.value().error());
      std::error_code shouldbe_ec = OUTCOME_V2_NAMESPACE::policy::error_code(shouldbe.error());
      // match errors for semantic equivalence
      if(kernel_outcome_ec.default_error_condition() == shouldbe_ec)
        return true;
#ifndef _WIN32
      // Some POSIX STLs don't map system_category to generic_category, which is stupid
      if(kernel_outcome_ec.category() == std::system_category() && shouldbe_ec.category() == std::generic_category())
        return kernel_outcome_ec.value() == shouldbe_ec.value();
#endif
#endif
      return false;
    }
    else
      return false;
  };

  template <class T, class U, class V, class A, class B, class C, typename = decltype(std::declval<outcome<T, U, V>>() == std::declval<outcome<A, B, C>>())> inline bool compare(const outcome<T, U, V> &a, const outcome<A, B, C> &b) { return a == b; }
  template <class T, class U, class V, class B, class C> inline bool compare(const outcome<T, U, V> &a, const outcome<void, B, C> &b)
  {
    if(a.has_value() && b.has_value())
      return true;
    if(a.has_error() && b.has_error())
      return a.error() == b.error();
    if(a.has_exception() && b.has_exception())
      return a.exception() == b.exception();
    return false;
  }
  template <class T, class U, class A, class B, typename = decltype(std::declval<result<T, U>>() == std::declval<result<A, B>>())> inline bool compare(const result<T, U> &a, const result<A, B> &b) { return a == b; }
  template <class T, class U, class B> inline bool compare(const result<T, U> &a, const result<void, B> &b)
  {
    if(a.has_value() && b.has_value())
      return true;
    if(a.has_error() && b.has_error())
      return a.error() == b.error();
    return false;
  }
}

/*! \brief A parameter permuter instance
\tparam is_mt True if this is a multithreaded parameter permuter
\tparam ParamSequence A sequence of parameter calls
*/
template <bool is_mt, class ParamSequence, class... Hooks> class parameter_permuter
{
  ParamSequence _params;
  std::tuple<Hooks...> _hooks;

  // syntax helper for MSVC :)
  using _permutation_results_type = typename detail::permutation_results_type<ParamSequence>;

public:
  //! True if this parameter permuter is multithreaded
  static constexpr bool is_multithreaded = is_mt;
  //! The type of the sequence of parameters
  using parameter_sequence_type = ParamSequence;
  //! True if the sequence of parameters is constant sized
  static constexpr bool parameter_sequence_type_is_constant_sized = detail::has_constant_size<ParamSequence>::value;
  //! Any constant size of the parameter_sequence_type if it is constant sized
  static constexpr size_t parameter_sequence_type_constant_size = detail::has_constant_size<ParamSequence>::size;

  //! The type of an individual parameter set
  using parameter_sequence_value_type = typename parameter_sequence_type::value_type;
  //! The type of the outcome from an individual parameter set
  using outcome_type = typename parameters_element<0, parameter_sequence_value_type>::type;
  //! Accessor for the outcome from an individual parameter set
  static constexpr const outcome_type &outcome_value(const parameter_sequence_value_type &v) { return std::get<0>(v); }

  //! The number of parameters in an individual parameter set
  static constexpr size_t parameters_size = KERNELTEST_V1_NAMESPACE::parameters_size<parameter_sequence_value_type>::value - 1;
  //! The type of the parameter at index N
  template <size_t N> using parameter_type = typename parameters_element<1 + N, parameter_sequence_value_type>::type;
  //! Accessor for the parameter at index N
  template <size_t N> static constexpr const parameter_type<N> &parameter_value(const parameter_sequence_value_type &v) { return std::get<N + 1>(v); }

  //! The type of the sequence of hooks
  using hook_sequence_type = std::tuple<Hooks...>;
  //! The number of hooks
  static constexpr size_t hook_sequence_size = std::tuple_size<hook_sequence_type>::value;
  //! The type of the hook at index N
  template <size_t N> using hook_type = typename std::tuple_element<N, hook_sequence_type>::type;
  //! Accessor for the hook at index N
  template <size_t N> static constexpr const hook_type<N> &hook_value(const hook_sequence_type &v) { return std::get<N>(v); }

  //! The type of the results returned by the call operator. Can be array<> or vector<> depending on ParamSequence
  template <class T> using permutation_results_type = typename _permutation_results_type::template type<T>;
  //! True if permutation_results_type is constant sized
  static constexpr bool permutation_results_type_is_constant_sized = parameter_sequence_type_is_constant_sized;
  //! Any constant size of the permutation_results_type if it is constant sized
  static constexpr size_t permutation_results_type_constant_size = parameter_sequence_type_constant_size;

  //! Constructs an instance. Best to use mt_permute_parameters() or st_permute_parameters() instead.
  constexpr parameter_permuter(ParamSequence &&params, std::tuple<Hooks...> &&hooks)
      : _params(std::move(params))
      , _hooks(std::move(hooks))
  {
  }

  //! Returns the parameter sequence this permuter was constructed with
  const ParamSequence &parameter_sequence() const { return _params; }
  //! Returns the hooks this permuter was constructed with
  const std::tuple<Hooks...> &hooks() const { return _hooks; }
  //! Convenience indexer into parameter sequence
  auto &operator[](size_t idx) { return _params[idx]; }
  //! Convenience indexer into parameter sequence
  const auto &operator[](size_t idx) const { return _params[idx]; }

  /*! Permute the callable f with this parameter permuter, returning a sequence of results.
  \return An array or vector of results (depends on ParamSequence::size() being constexpr).
  \throws bad_alloc Failure to allocate the vector of results if returning a vector.
  \throws anything Any exception thrown by any call of the callable f
  \param f Some callable with callspec result(typename ParamSequence::value_type ...)
  */
  template <class U> auto operator()(U &&f) const
  {
    using return_type = typename detail::result_of_parameter_permute<parameter_sequence_value_type, U>::type;
    using return_type_as_if_void = typename return_type::template rebind<void>;
    static_assert(!std::is_void<typename outcome_type::value_type>::value ? (std::is_constructible<outcome_type, return_type>::value) : (std::is_constructible<outcome_type, return_type_as_if_void>::value), "Return type of callable is not compatible with the parameter outcome type");
    permutation_results_type<return_type> results(detail::make_permutation_results_type<permutation_results_type<return_type>>(_params.size()));
    permutation_results_type<const parameter_sequence_value_type *> params(detail::make_permutation_results_type<permutation_results_type<const parameter_sequence_value_type *>>(_params.size()));
    {
      auto it(params.begin());
      for(auto &i : _params)
        *it++ = &i;
    }
    auto call_f = [&](size_t idx) {
      int stage = 0;
      auto nested_f = [&](size_t idx) {
        using callable_parameters_type = parameter_type<0>;
        const callable_parameters_type &p = parameter_value<0>(**params[idx]);
        try
        {
          // Instantiate the hooks
          auto hooks(detail::instantiate_hooks(_hooks, this, results[idx], idx, **params[idx], std::make_index_sequence<sizeof...(Hooks)>()));
          (void) hooks;
          stage = 1;
          // Call the kernel
          results[idx] = detail::call_f_with_parameters(std::forward<U>(f), p, std::make_index_sequence<KERNELTEST_V1_NAMESPACE::parameters_size<callable_parameters_type>::value>());
          stage = 2;
        }
        catch(...)
        {
          kerneltest_errc code = kerneltest_errc::setup_exception_thrown;
          if(1 == stage)
            code = kerneltest_errc::kernel_exception_thrown;
          else if(2 == stage)
            code = kerneltest_errc::teardown_exception_thrown;
          try
          {
            throw;
          }
          catch(const std::exception &e)
          {
            KERNELTEST_CERR("WARNING: C++ exception thrown '" << e.what() << "'" << std::endl);
          }
          catch(...)
          {
          }
#if 1
          results[idx] = return_type(in_place_type<typename return_type::error_type>, make_error_code(code));
//! \todo If permuter kernel output is an outcome, return a nested exception ptr assuming compilers have caught up by then
#else
          try
          {
            std::throw_with_nested(std::system_error(make_error_code(code)));
          }
          catch(...)
          {
            results[idx].set_exception(std::current_exception());
          }
#endif
        }
      };
//! \todo Need to install signal handlers for each permutation execution thread somehow
#if 0  // def _WIN32
      __try
      {
#endif
      nested_f(idx);
#if 0  // def _WIN32
    }
#if 0  // def _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6320)  // SA warning about catching all exceptions
#endif
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      kerneltest_errc code = kerneltest_errc::setup_seh_exception_thrown;
      if(1 == stage)
        code = kerneltest_errc::kernel_seh_exception_thrown;
      else if(2 == stage)
        code = kerneltest_errc::teardown_seh_exception_thrown;
      results[idx] = {make_error_code(code)};
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif
    };
#ifdef _OPENMP
    if(is_multithreaded)
    {
#pragma omp parallel for
      for(size_t n = 0; n < results.size(); n++)
        call_f(n);
    }
    else
#endif
    {
      for(size_t n = 0; n < results.size(); n++)
        call_f(n);
    }
    return results;
  }

  /*! Checks a sequence of results against what they ought to be, calling the callable f with the results
  \return True if all the results match
  \throws invalid_argument If the results passed is not of the same length as the parameter permute sequence
  \param results A sequence of results to check
  \param fail Some callable with callspec bool(size_t, value, shouldbe) called if the values do not match
  \param pass Some callable with callspec bool(size_t, value, shouldbe) called if the values match
  */
  template <class U, class V, class W, typename std::enable_if<QUICKCPPLIB_NAMESPACE::type_traits::is_sequence<U>::value, bool>::type = true> bool check(U &&sequence, V &&fail, W &&pass) const
  {
    if(sequence.size() != _params.size())
      throw std::invalid_argument("sequence to check does not have same length as parameter permute sequence");
    bool ret = true;
    auto it(sequence.cbegin());
    size_t idx = 0;
    for(auto &i : _params)
    {
      const outcome_type &shouldbe = outcome_value(i);
      const auto &outcome = *it;
      if(detail::check_result(outcome, shouldbe))
      {
        if(!pass(idx, outcome, shouldbe))
          ret = false;
      }
      else
      {
        if(!fail(idx, outcome, shouldbe))
          ret = false;
      }
      ++it;
      ++idx;
    }
    return ret;
  }
  //! \overload
  template <class U, class V> bool check(U &&sequence, V &&fail) const
  {
    return check(std::forward<U>(sequence), std::forward<V>(fail), [](size_t, const auto &, const auto &) { return true; });
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
  template <class T, size_t N, size_t... Idxs> auto array_from_Carray(const T (&seq)[N], std::index_sequence<Idxs...>) { return std::array<T, N>{{seq[Idxs]...}}; }
}

/*! \brief Create a single threaded parameter permuter
\tparam OutcomeType An outcome<T>, result<T> or option<T> for the outcome of the test kernel
\tparam InputTypes The types of the parameters of the test kernel
\tparam Sequence The type of the sequence containing the parameter sets
\tparam Hooks The types of any pretest or posttest hooks
\param seq The sequence of parameter sets
*/
template <class OutcomeType, class... Parameters, class Sequence, class... Hooks, typename = typename std::enable_if<detail::is_parameters_sequence_type_valid<Sequence, OutcomeType, Parameters...>::value>::type> constexpr parameter_permuter<true, Sequence> st_permute_parameters(Sequence &&seq, Hooks &&... hooks)
{
  return parameter_permuter<false, Sequence, Hooks...>(std::forward<Sequence>(seq), std::tuple<Hooks...>(std::forward<Hooks>(hooks)...));
}
//! \overload
template <class... Parameters, size_t N, class... Hooks> constexpr auto st_permute_parameters(const parameters<Parameters...> (&seq)[N], Hooks &&... hooks)
{
  // Convert C type arrays into std::array
  return parameter_permuter<false, std::array<parameters<Parameters...>, N>, Hooks...>(detail::array_from_Carray<parameters<Parameters...>, N>(seq, std::make_index_sequence<N>()), std::tuple<Hooks...>(std::forward<Hooks>(hooks)...));
}

/*! \brief Create a multithreaded parameter permuter
\tparam OutcomeType An outcome<T>, result<T> or option<T> for the outcome of the test kernel
\tparam InputTypes The types of the parameters of the test kernel
\tparam Sequence The type of the sequence containing the parameter sets
\tparam Hooks The types of any pretest or posttest hooks
\param seq The sequence of parameter sets
*/
template <class OutcomeType, class... Parameters, class Sequence, class... Hooks, typename = typename std::enable_if<detail::is_parameters_sequence_type_valid<Sequence, OutcomeType, Parameters...>::value>::type> constexpr parameter_permuter<true, Sequence> mt_permute_parameters(Sequence &&seq, Hooks &&... hooks)
{
  return parameter_permuter<true, Sequence, Hooks...>(std::forward<Sequence>(seq), std::tuple<Hooks...>(std::forward<Hooks>(hooks)...));
}
//! \overload
template <class... Parameters, size_t N, class... Hooks> constexpr auto mt_permute_parameters(const parameters<Parameters...> (&seq)[N], Hooks &&... hooks)
{
  // Convert C type arrays into std::array
  return parameter_permuter<true, std::array<parameters<Parameters...>, N>, Hooks...>(detail::array_from_Carray<parameters<Parameters...>, N>(seq, std::make_index_sequence<N>()), std::tuple<Hooks...>(std::forward<Hooks>(hooks)...));
}

namespace detail
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)  // conditional expression is constant
#endif
  class _print_params
  {
    template <bool first> static void _do() {}
    template <bool first, class T, class... Types> static void _do(T &&v, Types &&... vs)
    {
      if(!first)
        KERNELTEST_COUT(", ");
      KERNELTEST_COUT(v);
      _do<false>(std::forward<Types>(vs)...);
    };

  public:
    template <class... Types> void operator()(Types &&... vs) const { _do<true>(std::forward<Types>(vs)...); }
  };
  template <class Permuter> class _print_hook
  {
    const typename Permuter::parameter_sequence_value_type &_v;
    template <size_t Idx> void _do() const {}
    template <size_t Idx, class T, class... Types> void _do(T &&v, Types &&... vs) const
    {
      if(Idx > 0)
        KERNELTEST_COUT(", ");
      // Fetch the hook parameter set for this hook
      using hook_pars_type = typename Permuter::template parameter_type<1 + Idx>;
      //#ifdef __c2__  // c2 be buggy
      //      const auto &hook_pars = std::get<2 + Idx>(_v);
      //#else
      const auto &hook_pars = Permuter::template parameter_value<1 + Idx>(_v);
      //#endif
      // Each hook instantiator exposes a member function print(...) which takes
      // the same args as the hook instance
      detail::call_f_with_parameters([&v](const auto &... vs) { KERNELTEST_COUT(v.print(vs...)); }, hook_pars, std::make_index_sequence<parameters_size<hook_pars_type>::value>());
      _do<Idx + 1>(std::forward<Types>(vs)...);
    };

  public:
    _print_hook(const typename Permuter::parameter_sequence_value_type &v)
        : _v(v)
    {
    }
    template <class... Types> void operator()(Types &&... vs) const { _do<0>(std::forward<Types>(vs)...); }
  };
  template <class Permuter> void pretty_print_preamble(const Permuter &_permuter, size_t idx)
  {
    using namespace QUICKCPPLIB_NAMESPACE::console_colours;
    KERNELTEST_COUT("  " << yellow << (idx + 1) << "/" << _permuter.parameter_sequence().size() << ": " << normal);
    auto parameter_sequence_item_it = _permuter.parameter_sequence().cbegin();
    std::advance(parameter_sequence_item_it, idx);
    // Print kernel parameters we called the kernel with
    {
      KERNELTEST_COUT("kernel(");
      const auto &pars = std::get<1>(*parameter_sequence_item_it);
      using pars_type = typename std::decay<decltype(pars)>::type;
      detail::call_f_with_parameters(_print_params(), pars, std::make_index_sequence<parameters_size<pars_type>::value>());
      KERNELTEST_COUT(")");
    }
    // If there are any hooks, print those
    if(Permuter::hook_sequence_size > 0)
    {
      KERNELTEST_COUT(" with ");
      const auto &hooks = _permuter.hooks();
      detail::call_f_with_tuple(_print_hook<Permuter>(*parameter_sequence_item_it), hooks, std::make_index_sequence<Permuter::hook_sequence_size>());
    }
    KERNELTEST_COUT("\n");
  }

  template <class Permuter, class U> class pretty_print_failure_impl
  {
    const Permuter &_permuter;
    U _f;

  public:
    template <class V>
    pretty_print_failure_impl(const Permuter &permuter, V &&f)
        : _permuter(permuter)
        , _f(std::forward<V>(f))
    {
    }
    template <class _T, class _U> bool operator()(size_t idx, const _T &result, const _U &shouldbe) const
    {
      using namespace QUICKCPPLIB_NAMESPACE::console_colours;
      pretty_print_preamble(_permuter, idx);
      KERNELTEST_COUT("    " << bold << red << "FAILED" << normal << " (should be " << bold << print(shouldbe) << normal << ", was " << bold << print(result.value()) << normal << ")" << std::endl);
      _f(result, shouldbe);
      return false;
    }
  };
  template <class Permuter, class U> class pretty_print_success_impl
  {
    const Permuter &_permuter;
    U _f;

  public:
    template <class V>
    pretty_print_success_impl(const Permuter &permuter, V &&f)
        : _permuter(permuter)
        , _f(std::forward<V>(f))
    {
    }
    template <class _T, class _U> bool operator()(size_t idx, const _T &result, const _U &shouldbe) const
    {
      using namespace QUICKCPPLIB_NAMESPACE::console_colours;
      pretty_print_preamble(_permuter, idx);
      KERNELTEST_COUT("    " << bold << green << "PASSED " << normal << print(result.value()) << std::endl);
      _f(result, shouldbe);
      return true;
    }
  };
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}
//! Colourfully prints a failed result
template <class Permuter, class U> detail::pretty_print_failure_impl<Permuter, U> pretty_print_failure(const Permuter &s, U &&f)
{
  return detail::pretty_print_failure_impl<Permuter, U>(s, std::forward<U>(f));
}
//! \overload
template <class Permuter> auto pretty_print_failure(const Permuter &s)
{
  return pretty_print_failure(s, [](const auto &, const auto &) {});
}
//! Colourfully prints a successful result
template <class Permuter, class U> detail::pretty_print_success_impl<Permuter, U> pretty_print_success(const Permuter &s, U &&f)
{
  return detail::pretty_print_success_impl<Permuter, U>(s, std::forward<U>(f));
}
//! \overload
template <class Permuter> auto pretty_print_success(const Permuter &s)
{
  return pretty_print_success(s, [](const auto &, const auto &) {});
}


//! Do a normal permuter.check, but also call BOOST_CHECK() on every single result
template <class Permuter, class Results> inline void check_results_with_boost_test(const Permuter &permuter, const Results &results)
{
  // Note that we accumulate failures into the checks vector for later processing
  std::vector<std::function<void()>> checks;
  bool all_passed = permuter.check(results, pretty_print_failure(permuter, [&checks](const auto &result, const auto &shouldbe) { checks.push_back([&] { BOOST_CHECK(detail::compare(result.value(), shouldbe)); }); }), pretty_print_success(permuter));
  BOOST_CHECK(all_passed);
  // The pretty printing gets messed up by the unit test output, so defer telling it
  // about failures until now
  for(auto &i : checks)
    i();
}

/*! If expr is false, sets testreturn to an error_code_extended of `kerneltest_errc::check_failed` with an extended message of the expr which failed
*/
#define KERNELTEST_CHECK(testreturn, expr)                                                                                                                                                                                                                                                                                     \
  if(!(expr))                                                                                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                                                                                            \
    \
(testreturn) = typename std::decay_t<decltype(testreturn)>::error_type(make_error_code(kerneltest_errc::check_failed));                                                                                                                                                                                                        \
  \
}


KERNELTEST_V1_NAMESPACE_END

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // namespace
