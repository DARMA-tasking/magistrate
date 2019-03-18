
#include "serdes_common.h"
#include "base_serializer.h"
#include "sizer.h"

namespace serdes {

Sizer::Sizer() : Serializer(ModeType::Sizing) { }

SerialSizeType Sizer::getSize() const {
  return num_bytes_;
}

void Sizer::contiguousBytes(void*, SerialSizeType size, SerialSizeType num_elms) {
  num_bytes_ += size * num_elms;
}

} /* end namespace serdes */
