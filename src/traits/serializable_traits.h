
#if ! defined INCLUDED_SERDES_SERIALIZABILITY_TRAITS
#define INCLUDED_SERDES_SERIALIZABILITY_TRAITS

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <cstdint>
#include <cassert>

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif  /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

/*
 * Start the traits class with all the archetypes that can be detected for
 * serialization
 */

namespace serdes {

struct SerdesByteCopy {
  using isByteCopyable = std::true_type;
};

template <typename T>
struct SerializableTraits {
  template <typename U>
  using serialize_t = decltype(
    std::declval<U>().serialize(std::declval<Serializer&>())
  );
  using has_serialize = detection::is_detected<serialize_t, T>;

  template <typename U>
  using serializeParent_t =
    decltype(std::declval<U>().serializeParent(std::declval<Serializer&>()));
  using has_serializeParent = detection::is_detected<serializeParent_t, T>;

  template <typename U>
  using serializeThis_t =
    decltype(std::declval<U>().serializeThis(std::declval<Serializer&>()));
  using has_serializeThis = detection::is_detected<serializeThis_t, T>;

  template <typename U>
  using parserdes_t =
    decltype(std::declval<U>().parserdes(std::declval<Serializer&>()));
  using has_intrusive_parserdes = detection::is_detected<parserdes_t, T>;

  template <typename U>
  using nonintrustive_parserdes_t = decltype(parserdes(
    std::declval<Serializer&>(), std::declval<U&>()
  ));
  using has_nonintrustive_parserdes =
    detection::is_detected<nonintrustive_parserdes_t, T>;

  template <typename U>
  using byteCopyTrait_t = typename U::isByteCopyable;
  using has_byteCopyTraitTrue =
    detection::is_detected_convertible<std::true_type, byteCopyTrait_t, T>;

  template <typename U>
  using has_isArith = std::is_arithmetic<U>;

  template <typename U>
  using nonintrustive_serialize_t = decltype(serialize(
    std::declval<Serializer&>(), std::declval<U&>()
  ));
  using has_nonintrustive_serialize
  = detection::is_detected<nonintrustive_serialize_t, T>;

  template <typename U>
  using constructor_t = decltype(U());
  using has_default_constructor = detection::is_detected<constructor_t, T>;

  template <typename U>
  using reconstruct_t = decltype(U::reconstruct(std::declval<void*>()));
  using has_reconstruct =
    detection::is_detected_convertible<T&, reconstruct_t, T>;

  template <typename U>
  using nonintrustive_reconstruct_t = decltype(
    reconstruct(
      std::declval<Serializer&>(),
      std::declval<U*&>(),
      std::declval<void*>()
    )
  );
  //

  using has_nonintrusive_reconstruct =
    detection::is_detected<nonintrustive_reconstruct_t, T>;

  // Partial serializability (intrusive)
  static constexpr auto const has_int_parserdes = has_intrusive_parserdes::value;
  // Partial serializability (non-intrusive)
  static constexpr auto const has_nonint_parserdes =
    has_nonintrustive_parserdes::value;

  static constexpr auto const has_parserdes =
    has_nonint_parserdes or has_int_parserdes;

  // This defines what it means to have parent serializability
  static constexpr auto const has_parent_serialize = has_serializeParent::value;

  // This defines what it means to have this serializability
  static constexpr auto const has_this_serialize = has_serializeThis::value;

  // This defines what it means to have this serializability
  static constexpr auto const has_split_serialize =
    has_parent_serialize and has_this_serialize;

  // This defines what it means to be reconstructible
  static constexpr auto const is_bytecopyable =
    has_byteCopyTraitTrue::value or has_isArith<T>::value;

  // This defines what it means to be reconstructible
  static constexpr auto const is_reconstructible =
    has_reconstruct::value and not has_default_constructor::value;

  // This defines what it means to be non-intrusively reconstructible
  static constexpr auto const is_nonintrusive_reconstructible =
    has_nonintrusive_reconstruct::value and
    not has_default_constructor::value and
    not has_reconstruct::value;

  // This defines what it means to be default constructible
  static constexpr auto const is_default_constructible =
    has_default_constructor::value;

  static constexpr auto const has_serialize_instrusive =
    has_serialize::value;
  static constexpr auto const has_serialize_noninstrusive =
    has_nonintrustive_serialize::value;

  // This defines what it means to have a serialize function
  static constexpr auto const has_serialize_function =
    has_serialize_instrusive or has_serialize_noninstrusive;

  // This defines what it means to be serializable
  static constexpr auto const is_serializable =
    has_serialize_function and (is_default_constructible or is_reconstructible);

  static constexpr auto const is_parserdes =
    has_parserdes and not has_serialize_function;
};

}  // end namespace serdes

#endif  /*HAS_DETECTION_COMPONENT*/

#endif  /*INCLUDED_SERDES_SERIALIZABILITY_TRAITS*/
