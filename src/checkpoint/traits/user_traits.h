/*
//@HEADER
// *****************************************************************************
//
//                                user_traits.h
//                 DARMA/magistrate => Serialization Library
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#if !defined INCLUDED_SRC_CHECKPOINT_TRAITS_USER_TRAITS_H
#define INCLUDED_SRC_CHECKPOINT_TRAITS_USER_TRAITS_H

namespace {
  struct NoTrait;

  template<typename Traits, typename T, typename... U>
  struct without_helper {
    using type = typename without_helper<typename Traits::template _without_trait<T>, U...>::type;
  };
  template<typename Traits, typename T>
  struct without_helper<Traits, T> {
    using type = typename Traits::template _without_trait<T>;
  };
}

namespace checkpoint {
namespace SerializerUserTraits {

  template<typename U, typename V>
  struct is_base_or_derived
    : std::disjunction<std::is_base_of<U,V>, std::is_base_of<V,U>> {};

  template<typename U, typename V>
  struct is_same_template
    : std::false_type {};
  template<template <typename...> typename Templ, typename... U, typename... V>
  struct is_same_template<Templ<U...>, Templ<V...>>
    : std::true_type {};

  template<typename U, typename V>
  struct traits_match
    : std::disjunction<
#ifdef CHECKPOINT_INHERITED_USER_TRAITS
        is_base_or_derived<std::decay_t<U>,std::decay_t<V>>,
#endif
#ifdef CHECKPOINT_TEMPLATED_USER_TRAITS
        is_same_template<std::decay_t<U>,std::decay_t<V>>,
#endif
        std::is_same<std::decay_t<U>,std::decay_t<V>>
      > {};
  template<typename U, typename V>
  inline constexpr bool traits_match_v = traits_match<U,V>::value;



  template<typename Trait = NoTrait, typename... Traits>
  struct TraitHolder {
  protected:
    template<typename, typename...>
    friend struct TraitHolder;

    template<typename T>
    using has_trait = std::disjunction<traits_match<T, Trait>, traits_match<T, Traits>...>;

  public:

    //Remove one at a time, so we can allow multiple trait copies to be removed independently.
    template<typename T>
    using _without_trait = std::conditional_t<
                            traits_match_v<T, Trait>,
                            TraitHolder<Traits...>,
                            typename TraitHolder<Traits...>::template _without_trait<T>::template with_pre<Trait>
                          >;
    template<typename... T>
    using with = TraitHolder<Trait, Traits..., T...>;
    //To respect ordering. Could be handy in the future -- disambiguating multiple hooks?
    template<typename... T>
    using with_pre = TraitHolder<T..., Trait, Traits...>;

    //Flush out any nested traits (TraitHolder<..., TraitHolder<...>, ...>)
    //Remove any NoTraits, unless that's the only trait.
    using BaseTraits = typename TraitHolder<Traits...>::BaseTraits::template with_pre<Trait>;

    template<typename... T>
    using has = std::conjunction<has_trait<T>...>;
    template<typename... T>
    using has_any = std::disjunction<has_trait<T>...>;

    template<typename... T>
    static inline constexpr bool has_v = has<T...>::value;
    template<typename... T>
    static inline constexpr bool has_any_v = has_any<T...>::value;

    template<typename T, typename... U>
    using without = typename without_helper<BaseTraits, T, U...>::type;
  };

  //Empty specialization w/ shortcuts.
  template<>
  struct TraitHolder<NoTrait> {
    using BaseTraits = TraitHolder<NoTrait>;

    template<typename... T>
    using has = std::false_type;
    template<typename... T>
    using has_any = std::false_type;

    template<typename... T>
    static constexpr bool has_v = false;
    template<typename... T>
    static constexpr bool has_any_v = false;

    template<typename... T>
    using with = TraitHolder<T...>;
    template<typename... T>
    using with_pre = TraitHolder<T...>;

    template<typename... T>
    using without = TraitHolder<NoTrait>;

    template<typename T>
    using _without_trait = TraitHolder<NoTrait>;

  protected:
    template<typename T>
    using has_trait = std::false_type;

    template<typename, typename...>
    friend struct TraitHolder;
    template<typename...>
    friend struct without_helper;
  };

  //Specialization to remove nested traits.
  template<typename... Traits, typename... NestedTraits>
  struct TraitHolder<TraitHolder<NestedTraits...>, Traits...>
        : public TraitHolder<NestedTraits..., Traits...> {};
  //Ignore nested NoTrait
  template<typename... Traits>
  struct TraitHolder<TraitHolder<NoTrait>, Traits...>
        : public TraitHolder<Traits...> {};



  //Helpers for reinterpret_casting a serializer to add/remove traits.
  template<typename... ToAdd, template <typename...> typename SerTempl, typename... SerParams, typename SerTraits>
  auto& getWithTraits(SerTempl<SerParams..., SerTraits>& s){
    return *reinterpret_cast<SerTempl<SerParams..., typename SerTraits::template with<ToAdd...>>*>(&s);
  }

  template<typename... ToRem, template <typename...> typename SerTempl, typename... SerParams, typename SerTraits>
  auto& getWithoutTraits(SerTempl<SerParams..., SerTraits>& s){
    return *reinterpret_cast<SerTempl<SerParams..., typename SerTraits::template without<ToRem...>>*>(&s);
  }
} //namespace SerializerUserTraits


  template<typename... Traits>
  using UserTraitHolder = typename SerializerUserTraits::template TraitHolder<Traits...>::BaseTraits;
} //namespace checkpoint

#endif /*INCLUDED_SRC_CHECKPOINT_TRAITS_USER_TRAITS_H*/
