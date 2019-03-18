
#if !defined INCLUDED_SERDES_SIZER
#define INCLUDED_SERDES_SIZER

#include "serdes_common.h"
#include "base_serializer.h"

namespace serdes {

struct Sizer : Serializer {
  Sizer();

  SerialSizeType getSize() const;
  void contiguousBytes(void*, SerialSizeType size, SerialSizeType num_elms);

private:
  SerialSizeType num_bytes_ = 0;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_SIZER*/
