
#include "serdes_common.h"
#include "serdes.h"
#include "sizer.h"

namespace serdes {

Sizer::Sizer() : Serializer(ModeType::Sizing) { }

SizeType Sizer::getSize() const {
  return num_bytes_;
}

void Sizer::contiguousBytes(void*, SizeType size, SizeType num_elms) {
  num_bytes_ += size * num_elms;
}

} /* end namespace serdes */
