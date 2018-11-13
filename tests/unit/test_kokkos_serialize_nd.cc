#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"

// N-D dimension comparison
template <typename ViewT>
static void compareND(ViewT const& k1, ViewT const& k2) {
  serdes::ViewEquality<ViewT>::template compare<GTestEquality>(k1,k2);
}

template <typename LayoutT>
inline LayoutT layout4d(lsType d1, lsType d2, lsType d3, lsType d4) {
  return LayoutT{d1,d2,d3,d4};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout4d(lsType d1,lsType d2,lsType d3,lsType d4) {
  return Kokkos::LayoutStride{d1,1,d2,d1,d3,d1*d2,d4,d1*d2*d3};
}

#endif
