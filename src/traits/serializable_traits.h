
#if ! defined INCLUDED_SERDES_SERIALIZABILITY_TRAITS
#define INCLUDED_SERDES_SERIALIZABILITY_TRAITS

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <cstdint>

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif  /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

namespace serdes {

struct SerdesByteCopy {
  using isByteCopyable = std::true_type;
};

template <typename T>
struct SerializableTraits {
  template <typename U>
  using serialize_t = decltype(std::declval<U>().serialize(std::declval<Serializer&>()));
  using has_serialize = detection::is_detected<serialize_t, T>;

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
  using has_nonintrustive_serialize = detection::is_detected<nonintrustive_serialize_t, T>;

  template <typename U>
  using constructor_t = decltype(U());
  using has_default_constructor = detection::is_detected<constructor_t, T>;

  template <typename U>
  using reconstruct_t = decltype(U::reconstruct(std::declval<void*>()));
  using has_reconstruct = detection::is_detected_convertible<T&, reconstruct_t, T>;

  // This defines what it means to be reconstructible
  static constexpr auto const is_bytecopyable =
    has_byteCopyTraitTrue::value or has_isArith<T>::value;

  // This defines what it means to be reconstructible
  static constexpr auto const is_reconstructible =
    has_reconstruct::value and not has_default_constructor::value;

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
;
};

}  // end namespace serdes

#endif  /*HAS_DETECTION_COMPONENT*/

#endif  /*INCLUDED_SERDES_SERIALIZABILITY_TRAITS*/
