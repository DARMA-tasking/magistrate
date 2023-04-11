/*
//@HEADER
// *****************************************************************************
//
//                             user_traits.h
//                 DARMA/checkpoint => Serialization Library
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

#if !defined INCLUDED_USER_TRAITS_CHECKPOINT_TRAITS_H
#define INCLUDED_USER_TRAITS_CHECKPOINT_TRAITS_H

//"api" pre-declare
namespace checkpoint {
namespace SerializerUserTraits {
  template<typename SerializerT, typename Trait>
  struct has_trait;
  

  template<typename SerializerT, typename Trait>
  struct with_trait;
  template<typename SerializerT, typename Trait, typename... Traits>
  struct with_traits;

  template<typename SerializerT, typename Trait>
  struct without_trait;
  template<typename SerializerT, typename Trait, typename... Traits>
  struct without_traits;
}}  // end namespace checkpoint::SerializerUserTraits



namespace { //anon = filescope

  template<
    typename TraitToRemove,
    template<typename...> typename SerializerT,
    typename UserTraits,
    typename... PassedTraits
  >
  struct without_trait_impl {
    using type = SerializerT<PassedTraits...>;
  };

  template<
    typename ToRemove,
    template<typename...> typename SerializerT,
    typename... Passed,
    typename... Remaining
  >
  struct without_trait_impl<
        ToRemove,
        SerializerT,
        std::tuple<ToRemove, Remaining...>,
        Passed...
  >
  {
    using type = typename without_trait_impl<
                 ToRemove, SerializerT, std::tuple<Remaining...>, Passed...
                 >::type;
  };


  template<
    typename ToRemove,
    template<typename...> typename SerializerT,
    typename... Passed,
    typename ToKeep,
    typename... Remaining
  >
  struct without_trait_impl<
        ToRemove,
        SerializerT,
        std::tuple<ToKeep, Remaining...>,
        Passed...
  >
  {
    using type = typename without_trait_impl<
                 ToRemove, SerializerT, std::tuple<Remaining...>, Passed..., ToKeep
                 >::type;
  };
}

  
  
namespace checkpoint {
namespace SerializerUserTraits {
  template<
    template<typename...> typename SerializerT, 
    typename... UserTraits,
    typename Trait
  >
  struct has_trait<SerializerT<UserTraits...>, Trait> 
      : std::disjunction<std::is_same<Trait, UserTraits>...> { };


  template<
    template<typename...> typename SerializerT, 
    typename... UserTraits,
    typename Trait
  >
  struct with_trait<SerializerT<UserTraits...>, Trait>
      : std::conditional<has_trait<SerializerT<UserTraits...>, Trait>::value,
            SerializerT<UserTraits...>,
            SerializerT<UserTraits..., Trait>
        > { };


  template<typename SerializerT, typename Trait>
  struct with_traits<SerializerT, Trait> 
      : with_trait<SerializerT, Trait> {};
  
  template<
    typename SerializerT,
    typename TraitOne,
    typename TraitTwo,
    typename... Remaining
  >
  struct with_traits<SerializerT, TraitOne, TraitTwo, Remaining...>
      : with_traits<typename with_trait<SerializerT, TraitOne>::type, TraitTwo, Remaining...> {};

    

  template<
    typename SerializerT, 
    typename Trait
  >
  struct without_trait { 
    using type = SerializerT;
  };

  template<
    template<typename...> typename SerializerT, 
    typename... UserTraits,
    typename Trait
  >
  struct without_trait<SerializerT<UserTraits...>, Trait> 
      : without_trait_impl<Trait, SerializerT, std::tuple<UserTraits...>> { };


  template<typename SerializerT, typename Trait>
  struct without_traits<SerializerT, Trait>
      : without_trait<SerializerT, Trait> {};

  template<
    typename SerializerT,
    typename TraitOne,
    typename TraitTwo,
    typename... Remaining
  >
  struct without_traits<SerializerT, TraitOne, TraitTwo, Remaining...>
      : without_traits<typename without_trait<SerializerT, TraitOne>::type, TraitTwo, Remaining...> {};

}}  // end namespace checkpoint::SerializerUserTraits

#endif /*INCLUDED_USER_TRAITS_CHECKPOINT_TRAITS_H*/
