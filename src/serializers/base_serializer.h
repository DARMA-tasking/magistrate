
#if !defined INCLUDED_SERDES_BASE_SERIALIZER
#define INCLUDED_SERDES_BASE_SERIALIZER

#include "serdes_common.h"

#include <type_traits>
#include <cstdlib>

namespace serdes {

enum struct eSerializationMode : int8_t {
  None = 0,
  Unpacking = 1,
  Packing = 2,
  Sizing = 3,
  Invalid = -1
};

struct Serializer {
  using ModeType = eSerializationMode;

  explicit Serializer(ModeType const& in_mode) : cur_mode_(in_mode) {}

  ModeType getMode() const { return cur_mode_; }
  bool isSizing() const { return cur_mode_ == ModeType::Sizing; }
  bool isPacking() const { return cur_mode_ == ModeType::Packing; }
  bool isUnpacking() const { return cur_mode_ == ModeType::Unpacking; }

  template <typename SerializerT, typename T>
  static void contiguousTyped(SerializerT& serdes, T* ptr, SizeType num_elms) {
    serdes.contiguousBytes(static_cast<void*>(ptr), sizeof(T), num_elms);
  }

  SerialByteType* getBuffer() const { return nullptr; }
  SerialByteType* getSpotIncrement(SizeType const inc) { return nullptr; }

protected:
  ModeType cur_mode_ = ModeType::Invalid;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_BASE_SERIALIZER*/
