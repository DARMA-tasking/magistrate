
#if !defined INCLUDED_SERDES_INTERFACE
#define INCLUDED_SERDES_INTERFACE

#include <cstdlib>
#include <functional>
#include <memory>

namespace serialization { namespace interface {

using SizeType = size_t;
using SerialByteType = char;

using BufferCallbackType = std::function<SerialByteType*(SizeType size)>;

struct SerializedInfo {
  virtual SizeType getSize() const = 0;
  virtual SerialByteType* getBuffer() const = 0;
  virtual ~SerializedInfo() { }
};

using SerializedInfoPtrType = std::unique_ptr<SerializedInfo>;
using SerializedReturnType = SerializedInfoPtrType;

template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn = nullptr);

template <typename T>
T* deserialize(SerialByteType* buf, SizeType size, T* user_buf = nullptr);

template <typename T>
T* deserialize(SerializedReturnType&& in);

}} /* end namespace serialization::interface */

#endif /*INCLUDED_SERDES_INTERFACE*/
