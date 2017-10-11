
#if ! defined INCLUDED_SERDES_SERIALIZER_TRAITS
#define INCLUDED_SERDES_SERIALIZER_TRAITS

#include <cstdint>

#include "serdes_common.h"

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif  /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

namespace serdes {

template <typename T>
struct SerializerTraits {
  template <typename U>
  using contiguousBytes_t = decltype(std::declval<U>().contiguousBytes(
    std::declval<void*>(), std::declval<SizeType>(), std::declval<SizeType>()
  ));
  using has_contiguousBytes = detection::is_detected<contiguousBytes_t, T>;

  template <typename U>
  using isSizing_t = decltype(std::declval<U>().isSizing());
  using has_isSizing = detection::is_detected_convertible<bool, isSizing_t, T>;

  template <typename U>
  using isPacking_t = decltype(std::declval<U>().isPacking());
  using has_isPacking = detection::is_detected_convertible<bool, isPacking_t, T>;

  template <typename U>
  using isUnpacking_t = decltype(std::declval<U>().isUnpacking());
  using has_isUnpacking = detection::is_detected_convertible<bool, isUnpacking_t, T>;

  // This defines what it means to be reconstructible
  static constexpr auto const is_valid_serializer =
    has_contiguousBytes::value and has_isSizing::value
    and has_isPacking::value and has_isUnpacking::value;
};

}  // end namespace serdes

#endif  /*HAS_DETECTION_COMPONENT*/

#endif  /*INCLUDED_SERDES_SERIALIZER_TRAITS*/
