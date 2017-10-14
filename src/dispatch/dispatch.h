
#if !defined INCLUDED_SERDES_DISPATCH
#define INCLUDED_SERDES_DISPATCH

#include "serdes_common.h"
#include "buffer/buffer.h"
#include "dispatch_common.h"
#include "dispatch_serializer.h"
#include "dispatch_deserializer.h"

#include <tuple>

namespace serdes {

template <typename T>
struct Dispatch {
  static SizeType sizeType(T& to_size);
  static BufferPtrType packType(
    T& to_pack, SizeType const& size, SerialByteType* buf
  );
  template <typename PackerT>
  static BufferPtrType packTypeWithPacker(
    PackerT& packer, T& to_pack, SizeType const& size
  );
  static T& unpackType(
    SerialByteType* buf, SerialByteType* data, SizeType const& size
  );
};

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target);

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SizeType const num_elms);

template <typename T>
SerializedReturnType serializeType(
  T& to_serialize, BufferObtainFnType fn = nullptr
);

template <typename T>
T* deserializeType(
  SerialByteType* data, SizeType const& size, T* allocBuf = nullptr
);

} /* end namespace serdes */

#include "dispatch.impl.h"

#endif /*INCLUDED_SERDES_DISPATCH*/
