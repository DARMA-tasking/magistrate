
#if !defined INCLUDED_SERDES_DISPATCH_BYTES_MACRO
#define INCLUDED_SERDES_DISPATCH_BYTES_MACRO

#include "serdes_common.h"

#define SERDES_BYTES(TYPE)                                \
  namespace serdes {                                      \
    template <typename SerializerT>                       \
    void serialize(SerializerT& s, TYPE& t) {             \
      SerializerT::contiguousTyped(s, &t, 1);             \
    }                                                     \
  }

#endif /*INCLUDED_SERDES_DISPATCH_BYTES_MACRO*/
