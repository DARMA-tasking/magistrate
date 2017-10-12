
#if ! defined INCLUDED_SERDES_DISPATCH_SERIALIZER
#define INCLUDED_SERDES_DISPATCH_SERIALIZER

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"
#include "dispatch_serializer_byte.h"
#include "dispatch_serializer_nonbyte.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

// First, try to dispatch to byte variant of the dispatcher, which is more
// efficient and non-byte version (the non-byte version will call serialize() on
// each element)
template <typename SerializerT, typename T>
using SerializerDispatch = SerializerDispatchByte<SerializerT, T>;

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER*/
