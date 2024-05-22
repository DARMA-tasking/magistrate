/*
//@HEADER
// *****************************************************************************
//
//                            view_traverse_manual.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_MANUAL_H
#define INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_MANUAL_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/container/view_traits_extract.h"

#if MAGISTRATE_ENABLED_KOKKOS

#include <Kokkos_Core.hpp>
#include <Kokkos_DynamicView.hpp>

#include <utility>
#include <tuple>
#include <type_traits>

namespace checkpoint {


/*
 *                     === Manual Traversal ===
 *
 * This is left here to test the automatic code.
 *
 * Serialization of data backing a view by manually traversing the data using
 * the Kokkos::View::operator()(...) or
 * Kokkos::DynamicView::operator()(...). The access operator is guaranteed to
 * correctly accesses the data regardless of the layout.
 */

template <typename ViewT, typename = void>
struct SerializeMemory;

template <typename ViewT>
using KokkosAtomicArchetype = typename std::is_same<
  typename ViewT::traits::memory_traits, ::Kokkos::MemoryTraits<Kokkos::Atomic>
>;

template <typename ViewT>
struct SerializeMemory<
  ViewT,
  std::enable_if_t<KokkosAtomicArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT const& v) {
    using DataType = typename ViewGetType<ViewT>::DataType;
    if (s.isUnpacking()) {
      DataType val;
      s | val;
      v.operator()().operator=(val);
    } else {
      auto const& val = v.operator()().operator const DataType();
      //DataType const& val = v.operator()();
      s | val;
    }
  }
};

template <typename ViewT>
struct SerializeMemory<
  ViewT,
  std::enable_if_t<!KokkosAtomicArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT const& v) {
    s | v.operator()();
  }
};

template <typename SerializerT, typename ViewType, std::size_t dims>
struct TraverseManual;

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,0> {
  static void apply(SerializerT& s, ViewType const& v) {
    SerializeMemory<ViewType>::apply(s,v);

    //
    // This code works for both atomic and non-atomic views but I've specially
    // above for more clarity.
    //
    // if (s.isUnpacking()) {
    //   BaseType val{};
    //   s | val;
    //   v.operator()() = val;
    // } else {
    //   /*
    //    * Be very careful here, do not use "auto const& val". If the view is
    //    * atomic we need to get the conversion operator to `BaseType`. In full
    //    * glory, we are invoking the following implicitly:
    //    *
    //    *   auto const& val = v.operator()().operator const BaseType();
    //    */
    //   BaseType const& val = v.operator()();
    //   s | val;
    // }
    //
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,1> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      if (s.isUnpacking()) {
        BaseType val{};
        s | val;
        v.operator()(i) = val;
      } else {
        BaseType const& val = v.operator()(i);
        s | val;
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,2> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        if (s.isUnpacking()) {
          BaseType val{};
          s | val;
          v.operator()(i,j) = val;
        } else {
          BaseType const& val = v.operator()(i,j);
          s | val;
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,3> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          if (s.isUnpacking()) {
            BaseType val{};
            s | val;
            v.operator()(i,j,k) = val;
          } else {
            BaseType const& val = v.operator()(i,j,k);
            s | val;
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,4> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            if (s.isUnpacking()) {
              BaseType val{};
              s | val;
              v.operator()(i,j,k,l) = val;
            } else {
              BaseType const& val = v.operator()(i,j,k,l);
              s | val;
            }
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,5> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            for (SizeType m = 0; m < v.extent(4); m++) {
              if (s.isUnpacking()) {
                BaseType val{};
                s | val;
                v.operator()(i,j,k,l,m) = val;
              } else {
                BaseType const& val = v.operator()(i,j,k,l,m);
                s | val;
              }
            }
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,6> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            for (SizeType m = 0; m < v.extent(4); m++) {
              for (SizeType n = 0; n < v.extent(5); n++) {
                if (s.isUnpacking()) {
                  BaseType val{};
                  s | val;
                  v.operator()(i,j,k,l,m,n) = val;
                } else {
                  BaseType const& val = v.operator()(i,j,k,l,m,n);
                  s | val;
                }
              }
            }
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,7> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            for (SizeType m = 0; m < v.extent(4); m++) {
              for (SizeType n = 0; n < v.extent(5); n++) {
                for (SizeType o = 0; o < v.extent(6); o++) {
                  if (s.isUnpacking()) {
                    BaseType val{};
                    s | val;
                    v.operator()(i,j,k,l,m,n,o) = val;
                  } else {
                    BaseType const& val = v.operator()(i,j,k,l,m,n,o);
                    s | val;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,8> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            for (SizeType m = 0; m < v.extent(4); m++) {
              for (SizeType n = 0; n < v.extent(5); n++) {
                for (SizeType o = 0; o < v.extent(6); o++) {
                  for (SizeType p = 0; p < v.extent(7); p++) {
                    if (s.isUnpacking()) {
                      BaseType val{};
                      s | val;
                      v.operator()(i,j,k,l,m,n,o,p) = val;
                    } else {
                      BaseType const& val = v.operator()(i,j,k,l,m,n,o,p);
                      s | val;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
};

} /* end namespace checkpoint */

#endif /*MAGISTRATE_ENABLED_KOKKOS*/

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_MANUAL_H*/
