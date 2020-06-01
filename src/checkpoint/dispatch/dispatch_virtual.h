/*
//@HEADER
// *****************************************************************************
//
//                              dispatch_virtual.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H
#define INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H

#include "checkpoint/dispatch/dispatch.h"
#include "checkpoint/dispatch/vrt/virtual_serialize.h"

/**
 * ---------------------------------------------------------------------------
 *
 * Common header file for serializing virtual hierarchies of classes. Currently,
 * only linear, single inheritance patterns are supported in the implementation.
 *
 * ---------------------------------------------------------------------------
 * ----------------------------- API usage: ----------------------------------
 * ---------------------------------------------------------------------------
 *
 * === Option 1 ===
 *
 *   - Make your virtual class hierarchy you want to serialize all inherit from
 *     \c SerializableBase<T> and \c SerializableDervived<T,BaseT> in the whole
 *     hierarchy.
 *
 * === Option 2 ===
 *
 *   - Insert checkpoint macros in your virtual class hierarchy for derived and
 *     base classes with the corresponding macros:
 *     - \c checkpoint_virtual_serialize_base(T)
 *     - \c checkpoint_virtual_serialize_dervied(T,BaseT)
 *
 * Invoking the virtual serializer:
 *
 *   - If you have a \c std::unique_ptr<T>, where T is virtually serializable (by
 *     using the macros or inheriting from \c SerializableBase<T> and
 *     \c SerializableDervived<T,BaseT> ), they will automatically be virtually
 *     serialized.
 *
 *   - If you have a raw pointer, \c Teuchos::RCP, or \c std::shared_ptr<T>,
 *     you must invoke: \c checkpoint::allocateConstructForPointer<SerializerT,T>
 *
 *     Example with raw pointer:
 *
 *       template <typename T>
 *       struct MyObjectWithRawPointer {
 *         T* raw_ptr;
 *
 *         template <typename SerializerT>
 *         void serialize(SerializerT& s) {
 *           bool is_null = raw_ptr == nullptr;
 *           s | is_null; // serialize whether we have a null pointer not
 *           if (!is_null) {
 *             // During size/pack, save the actual derived type of raw_ptr;
 *             // During unpack, allocate/construct raw_ptr with correct virtual type
 *             checkpoint::allocateConstructForPointer(s, raw_ptr);
 *             s | *raw_ptr;
 *           }
 *         }
 *       };
 *
 * ---------------------------------------------------------------------------
 * --------------------- Overview of implementation: -------------------------
 * ---------------------------------------------------------------------------
 *
 * When packing we need to traverse the whole hierarchy of virtual classes to
 * serialize all the user's content. Thus, \c SerializableBase<T> and
 * \c SerializableDervived<T,BaseT> insert a shim layer between all layers of
 * the virtual hierarchy and below the base class. The alternative macros,
 * instead of inserting a shim layer, just add the required virtual methods for
 * recursively serializing the hierarchy.
 *
 * The macros or shim classes insert a virtual method
 * \c _checkpointDynamicSerialize which is called during sizing, packing, and
 * unpacking (all traversals). \c _checkpointDynamicSerialize calls the
 * serializer on the base class recursively to get the whole hierarchy
 * serialized.
 *
 * To actually dispatch the to the object's serialize method, there is a problem
 * that the serializer type is templated (and is non-virtual). Since we can't
 * mix virtual and templates, the serializer is carried through
 * \c _checkpointDynamicSerialize as a \c void* but then dispatched using a
 * type registry for the serializers. The type registry, templated on the object
 * type, type-erases the templated serializer type (see serializer_registry.h).
 * To ensure that all serializers are instantiated properly,
 * \c _checkpointDynamicSerialize calls
 * \c checkpoint::instantiateObjSerializer<T,checkpoint_serializer_variadic_args()>
 * where \c checkpoint_serializer_variadic_args() is a macro that contains a
 * pack of all existing serializers that ship with checkpoint.
 *
 * *Important*: if you define a custom traversal, you need to manually
 * instantiate it for all virtual objects.
 *
 * When a serializer is looked up in the registry, it gets an index for a given
 * \c T -- where \c T is a user's virtually serialized object type. However, we
 * save the registry index for the base class in the hierarchy. Because the
 * serializer registry is templated on \c T (i.e., there is a separate registry
 * for each \c T ) we must match up the entry across registries for a derived
 * type when we invoke the serialize from that registry entry on it. Thus, we
 * use \c linkDerivedToBase<SerializerT,DerivedT,BaseT>() to link the
 * entries---store the base index in the derived index's entry. Using this, when
 * picking the right serializer for the derived, we check the dervied entries
 * for the one that matches the base index. This ensures that no matter what
 * order the static serializer registries are created in we get the right
 * serializer across multiple registries.
 *
 * During unpacking a user's \c T -- which is virtually serialized -- we have to
 * construct the correct derived type based on what was actually instantiated
 * when \c T was live. Thus, we have a second registry for all objects that
 * generate a unique index for each object type. The generated integer
 * representing the type is serialized during sizing and packing and then used
 * during unpacking to invoke the correct allocation and object
 * construction. Refer to object_registry.h for more details.
 *
 * The virtual method \c _checkpointDynamicTypeIndex gives us the correct
 * registered index for a virtual hierarchy for the derived class that is given
 * to us to serialize during sizing/packing.
 *
 */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H*/
