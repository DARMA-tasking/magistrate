
#if !defined INCLUDED_SERDES_INTERFACE
#define INCLUDED_SERDES_INTERFACE

#include <functional>

namespace serialize { namespace interface {

using SizeType = size_t;
using SerialByteType = char;
using BufferCallbackType = std::function<SerialByteType*(SizeType size)>;

template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn = nullptr);

template <typename T>
T* deserialize(SerialByteType* buf, SizeType size, T* user_buf = nullptr);

}} /* end namespace serialize::interface */

#endif /*INCLUDED_SERDES_INTERFACE*/
