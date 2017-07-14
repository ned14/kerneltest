/* Custom test kernel hooks
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: Aug 2016


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

#include "../config.hpp"

#ifndef KERNELTEST_HOOKS_CUSTOM_HPP
#define KERNELTEST_HOOKS_CUSTOM_HPP

KERNELTEST_V1_NAMESPACE_BEGIN

namespace hooks
{
  namespace custom_impl
  {
    template <class U, class V> struct impl
    {
      bool active;
      U onfinish;
      V par;
      impl(U &&_onfinish, V &&_par) noexcept : active(true), onfinish(std::move(_onfinish)), par(std::move(_par)) {}
      impl(impl &&o) noexcept : active(o.active), onfinish(std::move(o.onfinish)), par(std::move(o.par)) { o.active = false; }
      impl(const impl &) = delete;
      ~impl()
      {
        if(active)
          onfinish(par);
      }
    };
    template <class U> struct impl<U, void>
    {
      bool active;
      U onfinish;
      impl(U &&_onfinish) noexcept : active(true), onfinish(std::move(_onfinish)) {}
      impl(impl &&o) noexcept : active(o.active), onfinish(std::move(o.onfinish)) { o.active = false; }
      impl(const impl &) = delete;
      ~impl()
      {
        if(active)
          onfinish();
      }
    };
    template <class state_type> struct make_impl
    {
      template <class U, class V, class Parent, class RetType, class... Args> auto operator()(U &&onbegin, V &&onfinish, Parent &parent, RetType &testret, size_t idx, Args &&... args) const { return impl<V, state_type>(std::forward<V>(onfinish), onbegin(parent, testret, idx, std::forward<Args>(args)...)); }
    };
    template <> struct make_impl<void>
    {
      template <class U, class V, class Parent, class RetType, class... Args> auto operator()(U &&onbegin, V &&onfinish, Parent &parent, RetType &testret, size_t idx, Args &&... args) const
      {
        onbegin(parent, testret, idx, std::forward<Args>(args)...);
        return impl<V, void>(std::forward<V>(onfinish));
      }
    };
    // Instantiated during permuter construction
    template <class U, class V> struct inst
    {
      U onbegin;
      V onfinish;
      std::string description;
      inst(const inst &) = delete;
      inst(inst &&) noexcept = default;

      // Called at the beginning of an individual test. Returns object destroyed at the end of an individual test.
      template <class Parent, class RetType, class... Args> auto operator()(Parent *parent, RetType &testret, size_t idx, Args &&... args) const
      {
        using return_type = decltype(onbegin(*parent, testret, idx, std::forward<Args>(args)...));
        return make_impl<return_type>()(onbegin, std::move(onfinish), *parent, testret, idx, std::forward<Args>(args)...);
      }
      template <class... Args> std::string print(Args &&...) const { return description; }
    };
  }
  //! The parameters for the custom hook
  template <class... Args> using custom_parameters = parameters<Args...>;
  /*! Kernel test hook invoking a lambda before a test and another lambda after a test.
  The first lambda is called with the spec `(parameter_permuter<...> &parent, outcome<T> &testret, size_t, custom_parameters ...)`.
  If the first lambda returns anything, it is passed as the first parameter to the second lambda.
  */
  template <class U, class V> inline auto custom(U &&onbegin, V &&onfinish, std::string description) { return custom_impl::inst<U, V>{std::forward<U>(onbegin), std::forward<V>(onfinish), std::move(description)}; }
}

KERNELTEST_V1_NAMESPACE_END

#endif
