
#if !defined INCLUDED_SERDES_DISPATCH
#define INCLUDED_SERDES_DISPATCH

#include "serdes_common.h"
#include "buffer/buffer.h"
#include "dispatch_common.h"
#include "dispatch_serializer.h"
#include "dispatch_deserializer.h"
#include "dispatch_byte_macro.h"

#include <tuple>

namespace serdes {

struct InPlaceTag { };

template <typename T>
struct Dispatch {
  static SerialSizeType sizeType(T& to_size);
  static BufferPtrType packType(
    T& to_pack, SerialSizeType const& size, SerialByteType* buf
  );
  template <typename PackerT>
  static BufferPtrType packTypeWithPacker(
    PackerT& packer, T& to_pack, SerialSizeType const& size
  );
  static T& unpackType(
    SerialByteType* buf, SerialByteType* data, SerialSizeType const& size,
    bool in_place = false
  );

  static SerialSizeType sizeTypePartial(T& to_size);
  static BufferPtrType packTypePartial(
    T& to_pack, SerialSizeType const& size, SerialByteType* buf
  );
  template <typename PackerT>
  static BufferPtrType packTypeWithPackerPartial(
    PackerT& packer, T& to_pack, SerialSizeType const& size
  );
  static T& unpackTypePartial(
    SerialByteType* buf, SerialByteType* data, SerialSizeType const& size
  );
};

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target);

template <typename Serializer, typename T>
inline Serializer& operator&(Serializer& s, T& target);

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const num_elms);

template <typename Serializer, typename T>
inline void parserdesArray(Serializer& s, T* array, SerialSizeType const num_elms);

template <typename T>
SerializedReturnType serializeType(
  T& to_serialize, BufferObtainFnType fn = nullptr
);

template <typename T>
T* deserializeType(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf = nullptr
);

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, SerialSizeType sz, T* t);

template <typename T>
SerializedReturnType serializeTypePartial(
  T& to_serialize, BufferObtainFnType fn = nullptr
);

template <typename T>
T* deserializeTypePartial(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf = nullptr
);

template <typename T>
std::size_t sizeType(T& t);

} /* end namespace serdes */

#include "dispatch.impl.h"

#endif /*INCLUDED_SERDES_DISPATCH*/
