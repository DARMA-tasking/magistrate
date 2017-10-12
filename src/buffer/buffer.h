
#if !defined INCLUDED_SERDES_BUFFER
#define INCLUDED_SERDES_BUFFER

#include "serdes_common.h"

#include <memory>

namespace serdes {

struct Buffer {
  virtual SerialByteType* getBuffer() const = 0;
  virtual ~Buffer() { }
};

using SerializedType = std::tuple<SerialByteType*, SizeType>;
using BufferPtrType = std::unique_ptr<Buffer>;
using SerializedReturnType = std::tuple<BufferPtrType, SizeType>;

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_BUFFER*/
