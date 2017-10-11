
#if !defined INCLUDED_SERDES_SIZER
#define INCLUDED_SERDES_SIZER

#include "serdes_common.h"
#include "serdes.h"

namespace serdes {

struct Sizer : Serializer {
  Sizer();

  SizeType getSize() const;
  void contiguousBytes(void*, SizeType size, SizeType num_elms);

private:
  SizeType num_bytes_ = 0;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_SIZER*/
