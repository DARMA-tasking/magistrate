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

namespace checkpoint {
namespace serializerUserTraits {

  template<typename... Traits>
  struct TraitHolder;


  //Check if two traits are the same type when decayed
  template<typename U, typename V>
  struct traits_match : public std::is_same<std::decay_t<U>,std::decay_t<V>>{};
  template<typename U, typename V>
  inline constexpr bool traits_match_v = traits_match<U,V>::value;


  //Merge the traits of two TraitHolders
  template<typename HolderA, typename HolderB>
  struct MergeTraitsImpl;
  template<typename HolderA, typename HolderB>
  using MergeTraits = typename MergeTraitsImpl<HolderA, HolderB>::type;

  template<typename... TraitsA, typename... TraitsB>
  struct MergeTraitsImpl<TraitHolder<TraitsA...>, TraitHolder<TraitsB...>>{
    using type = TraitHolder<TraitsA..., TraitsB...>;
  };


  //The type of the first trait in a TraitHolder
  template<typename Holder>
  struct FirstTraitImpl;
  template<typename Holder>
  using FirstTrait = typename FirstTraitImpl<Holder>::type;

  template<typename Trait, typename... Traits>
  struct FirstTraitImpl<TraitHolder<Trait, Traits...>>{
    using type = Trait;
  };


  //Remove the first trait from a TraitHolder.
  template<typename Holder>
  struct PopTraitImpl;
  template<typename Holder>
  using PopTrait = typename PopTraitImpl<Holder>::type;

  template<typename Trait, typename... Traits>
  struct PopTraitImpl<TraitHolder<Trait, Traits...>>{
    using type = TraitHolder<Traits...>;
  };


  //Remove types T from a TraitHolder. Only (up to) one trait is removed per T
  template<typename Holder, typename... T>
  struct WithoutTraitsImpl;
  template<typename Holder, typename... T>
  using WithoutTraits = typename WithoutTraitsImpl<Holder, T...>::type;

  template<typename Holder>
  struct WithoutTraitsImpl<Holder> {
    using type = Holder;
  };
  template<typename T, typename... U>
  struct WithoutTraitsImpl<TraitHolder<>, T, U...> {
    using type = TraitHolder<>;
  };
  template<typename Holder, typename T, typename... U>
  struct WithoutTraitsImpl<Holder, T, U...> {
    using CheckTrait = FirstTrait<Holder>;
    using Remaining = PopTrait<Holder>;
    using WithoutT = std::conditional_t<
      traits_match_v<T, CheckTrait>,
      Remaining,
      MergeTraits<TraitHolder<CheckTrait>, WithoutTraits<Remaining, T>>
    >;
    using type = WithoutTraits<WithoutT, U...>;
  };


  //Holds traits that should be unwrapped when passed into a TraitHolder
  template<typename Holder>
  struct NestedTraitHolder {
    using Traits = Holder;
  };
  template<typename Holder>
  using CopyTraits = NestedTraitHolder<Holder>;


  //TraitHolder with any NestedTraitHolders unwrapped to their held traits
  template<typename Holder>
  struct UnwrapTraitsImpl;
  template<typename Holder>
  using UnwrapTraits = typename UnwrapTraitsImpl<Holder>::type;

  template<>
  struct UnwrapTraitsImpl<TraitHolder<>>{
    using type = TraitHolder<>;
  };
  template<typename Trait, typename... Traits>
  struct UnwrapTraitsImpl<TraitHolder<Trait, Traits...>>{
    using type = MergeTraits<
      TraitHolder<Trait>,
      UnwrapTraits<TraitHolder<Traits...>>
    >;
  };
  template<typename WrappedTraits, typename... Traits>
  struct UnwrapTraitsImpl<
    TraitHolder<NestedTraitHolder<WrappedTraits>, Traits...>
  > {
    using type = MergeTraits<WrappedTraits,
      UnwrapTraits<TraitHolder<Traits...>>
    >;
  };


  template<typename... Traits>
  struct TraitHolder {
  protected:
    template<typename...>
    friend struct TraitHolder;

    template<typename T>
    using has_trait = std::disjunction<traits_match<T, Traits>...>;

  public:
    template<typename... T>
    using With = TraitHolder<Traits..., T...>;
    //To respect ordering. Could be handy in the future -- disambiguating multiple hooks?
    template<typename... T>
    using WithPre = TraitHolder<T..., Traits...>;

    //Has all types T
    template<typename... T>
    using has = std::conjunction<has_trait<T>...>;
    //Has any types within T
    template<typename... T>
    using has_any = std::disjunction<has_trait<T>...>;

    //Type with each trait T removed once, if present.
    //E.G. TraitHolder<TraitOne, TraitOne, TraitTwo, TraitThree>
    //         ::Without<TraitOne, TraitTwo>
    //       == TraitHolder<TraitOne, TraitThree>;
    template<typename... T>
    using Without = WithoutTraits<TraitHolder<Traits...>, T...>;
  };
} //namespace serializerUserTraits


  template<typename... Traits>
  using UserTraitHolder = serializerUserTraits::UnwrapTraits<
    serializerUserTraits::TraitHolder<Traits...>
  >;
} //namespace checkpoint

#endif /*INCLUDED_SRC_CHECKPOINT_TRAITS_USER_TRAITS_H*/
