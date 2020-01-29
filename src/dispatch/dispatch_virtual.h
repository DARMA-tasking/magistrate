/*
//@HEADER
// *****************************************************************************
//
//                             dispatch_virtual.h
//                           DARMA Toolkit v. 1.0.0
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

#if !defined(CHECKPOINT_DISPATCH_VIRTUAL_H)
#define CHECKPOINT_DISPATCH_VIRTUAL_H

#include "dispatch.h"

#include <vector>
#include <tuple>
#include <functional>

namespace serdes {
  using AutoHandlerType = int;

  struct SERIALIZE_CONSTRUCT_TAG {};

  template <typename T>
  using RegistryType = std::vector<
    std::tuple<
      int,
      std::function<T*(void)>
      >
    >;

  template <typename ObjT>
  struct Registrar {
    Registrar();
    AutoHandlerType index;
  };

  template <typename ObjT>
  struct Type {
    static AutoHandlerType const idx;
  };

  template <typename T>
  inline RegistryType<T>& getRegistry() {
    static RegistryType<T> reg;
    return reg;
  }

  template <typename ObjT>
  Registrar<ObjT>::Registrar() {
    using BaseType = typename ObjT::SerDerBaseType;

    auto& reg = getRegistry<BaseType>();
    index = reg.size();

    debug_serdes("registrar: %ld, %s\n", reg.size(), typeid(ObjT).name());

    reg.emplace_back(
                     std::make_tuple(
                                     index,
                                     []() -> BaseType* { return new ObjT(SERIALIZE_CONSTRUCT_TAG{}); }
                                     )
                     );
  }

  template <typename ObjT>
  AutoHandlerType const Type<ObjT>::idx = Registrar<ObjT>().index;

  template <typename T>
  inline auto getObjIdx(AutoHandlerType han) {
    debug_serdes("getObjIdx: han=%d, size=%ld\n", han, getRegistry<T>().size());
    return getRegistry<T>().at(han);
  }

  template <typename ObjT>
  inline AutoHandlerType makeObjIdx() {
    return Type<ObjT>::idx;
  }

  template <typename DerivedT, typename BaseT>
  struct SerializableDerived : BaseT {
    using SerDerBaseType = BaseT;

    template <typename... Args>
    SerializableDerived(Args&&... args)
      : BaseT(std::forward<Args>(args)...)
    { }

    SerializableDerived() {
    }

    template <typename SerializerT>
    void serialize(SerializerT& s) {
      debug_serdes("SerializableDerived:: serialize\n");
      BaseT::serialize(s);
    }

    void doSerialize(serdes::Serializer* s) override {
      AutoHandlerType entry = makeObjIdx<DerivedT>();

      if (s->isSizing()) {
        auto& ss = *static_cast<serdes::Sizer*>(s);
        ss | entry;
        ss | *this;
        ss | (*static_cast<DerivedT*>(this));
      } else if (s->isPacking()) {
        auto& sp = *static_cast<serdes::Packer*>(s);
        sp | entry;
        sp | *this;
        sp | (*static_cast<DerivedT*>(this));
      } else if (s->isUnpacking()) {
        auto& su = *static_cast<serdes::Unpacker*>(s);
        // entry was read out in virtualSerialize to reconstruct the object
        su | *this;
        su | (*static_cast<DerivedT*>(this));
      }
    }
  };

  template <typename BaseT>
  struct SerializableBase {
    virtual void doSerialize(serdes::Serializer*)  = 0;
  };

  template <typename BaseT, typename SerializerT>
  void virtualSerialize(BaseT*& base, SerializerT& s) {
    if (s.isUnpacking()) {
      AutoHandlerType entry = -1;
      // Peek to see the type of the next element, get the right constructor
      s | entry;
      debug_serdes("entry=%d\n", entry);
      auto lam = getObjIdx<BaseT>(entry);
      auto ptr = std::get<1>(lam)();
      base = ptr;
    }
    base->doSerialize(&s);
  }

} // namespace serdes

#endif
