/*
//@HEADER
// *****************************************************************************
//
//                      test_kokkos_integration_commons.h
//                 DARMA/magistrate => Serialization Library
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#if !defined INCLUDED_TESTS_UNIT_TEST_KOKKOS_INTEGRATION_COMMONS_H
#define INCLUDED_TESTS_UNIT_TEST_KOKKOS_INTEGRATION_COMMONS_H

#include "test_harness.h"
#include "test_commons.h"

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View Integration Tests with other elements
///////////////////////////////////////////////////////////////////////////////

struct BaseData {
  int a = 10;

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | a;
  }
};

static struct DataConsTagType { } DataConsTag { };

static constexpr int const gold_val1 = 10;
static constexpr int const gold_val2 = 20;
static constexpr int const gold_val3 = 29;

static constexpr std::size_t const d1_a = 100;
static constexpr std::size_t const d2_a = 8, d2_b = 7;
static constexpr std::size_t const d3_a = 2, d3_b = 7, d3_c = 4;
static constexpr std::size_t const d4_a = 2;

struct Data : BaseData {
  using AtomicTrait      = ::Kokkos::MemoryTraits<Kokkos::Atomic>;
  using Kokkos_ViewType0 = ::Kokkos::View<int,      AtomicTrait>;
  using Kokkos_ViewType1 = ::Kokkos::View<int*,     Kokkos::LayoutLeft>;
  using Kokkos_ViewType2 = ::Kokkos::View<double**, Kokkos::LayoutRight>;
  using Kokkos_ViewType3 = ::Kokkos::View<float***, AtomicTrait>;
  using Kokkos_ViewType4 = ::Kokkos::View<int*[2]>;
  #if MAGISTRATE_KOKKOS_KERNELS_ENABLED
  using Kokkos_CrsType = ::KokkosSparse::StaticCrsGraph<double, Kokkos::DefaultExecutionSpace>;
  using Kokkos_HostCrsGraphType = Kokkos_CrsType::host_mirror_type;
  using Kokkos_CrsMatrix = ::KokkosSparse::CrsMatrix<double, int, Kokkos::DefaultExecutionSpace>;
  using Kokkos_HostCrsMatrix = Kokkos_CrsMatrix::host_mirror_type;
  #endif
  using DimType          = typename Kokkos_ViewType1::size_type;

  Data() = default;

  explicit Data(DataConsTagType)
    : vec({gold_val3,gold_val3+1,gold_val3+2}), val1(gold_val1), val2(gold_val2)
  {
    Kokkos_ViewType0 v0_tmp("v0_tmp");
    Kokkos_ViewType1 v1_tmp("v1_tmp",d1_a);
    Kokkos_ViewType2 v2_tmp("v2_tmp",d2_a,d2_b);
    Kokkos_ViewType3 v3_tmp("v3_tmp",d3_a,d3_b,d3_c);
    Kokkos_ViewType4 v4_tmp("v4_tmp",d4_a);

    this->init_data_on_device(v0_tmp,v1_tmp,v2_tmp,v3_tmp,v4_tmp);
    v0 = v0_tmp;
    v1 = v1_tmp;
    v2 = v2_tmp;
    v3 = v3_tmp;
    v4 = v4_tmp;

    #if MAGISTRATE_KOKKOS_KERNELS_ENABLED
    std::vector< std::vector< int > > graph( d1_a );

    for ( size_t i = 0 ; i < d1_a; ++i ) {
       graph[i].reserve(8);
       for ( size_t j = 0 ; j < 8 ; ++j ) {
         graph[i].push_back( i + j * 3 );
       }
    }

    crs = KokkosSparse::create_staticcrsgraph<Kokkos_CrsType>( "crs_type" , graph );

    int nrow = 5, ncol = 7;
    size_t nnz = 11;
    std::vector<int> rows{0, 2, 4, 5, 7, 11};
    std::vector<int> cols{0, 1, 1, 3, 2, 0, 3, 0, 2, 4, 6};
    std::vector<double> values{1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1, 10.1, 11.1};
    crs_mat = KokkosSparse::CrsMatrix<double, int, Kokkos::DefaultExecutionSpace>("crs_matrix", nrow, ncol, nnz, &values[0], &rows[0], &cols[0]);

    //--- Check that the input matrix is stored as expected
    EXPECT_EQ( nrow, (int) crs_mat.numRows());
    EXPECT_EQ( ncol, (int) crs_mat.numCols());
    EXPECT_EQ( nnz, (size_t) crs_mat.nnz());

    Kokkos_HostCrsMatrix host_crs_mat("host_crs_matrix",crs_mat);

    for (int ir = 0; ir < std::min<int>(nrow, host_crs_mat.numRows()); ++ir) {
      auto myRow = host_crs_mat.rowConst(ir);
      for (int jc = 0; jc < myRow.length; ++jc) {
        EXPECT_EQ( cols[rows[ir] + jc], myRow.colidx(jc) );
        EXPECT_EQ( values[rows[ir] + jc], myRow.value(jc) );
      }
    }
    #endif
  }

  void init_data_on_device(const Kokkos_ViewType0& v0_tmp,
                           const Kokkos_ViewType1& v1_tmp,
                           const Kokkos_ViewType2& v2_tmp,
                           const Kokkos_ViewType3& v3_tmp,
                           const Kokkos_ViewType4& v4_tmp)
  {
    Kokkos::parallel_for("v0_tmp init",1,KOKKOS_LAMBDA(size_t ){
      v0_tmp.operator()() = v0val();
    });

    Kokkos::parallel_for("v1_tmp init",d1_a,KOKKOS_LAMBDA(size_t i){
      v1_tmp.operator()(i) = v1val(i);
    });

    Kokkos::parallel_for("v2_tmp init",d2_a,KOKKOS_LAMBDA(size_t i){
      for (DimType j = 0; j < d2_b; j++) {
        v2_tmp.operator()(i,j) = v2val(i,j);
      }
    });

    Kokkos::parallel_for("v3_tmp init",d3_a,KOKKOS_LAMBDA(size_t i){
      for (DimType j = 0; j < d3_b; j++) {
        for (DimType k = 0; k < d3_c; k++) {
          v3_tmp.operator()(i,j,k) = v3val(i,j,k);//d3_a*d3_b*i + j*d3_a + k;
        }
      }
    });

    Kokkos::parallel_for("v4_tmp init",d4_a,KOKKOS_LAMBDA(size_t i){
      v4_tmp.operator()(i,0) = v4val(i,0);//d4_a*i*2 + 1;
      v4_tmp.operator()(i,1) = v4val(i,1);//d4_a*i*2 + 2;
    });

    Kokkos::fence();
  }

  /* Generators for creating expected data values */
  KOKKOS_FUNCTION
  static int v0val() {
    return d1_a * 29;
  }
  KOKKOS_FUNCTION
  static int v1val(DimType i) {
    return d1_a * i;
  }
  KOKKOS_FUNCTION
  static double v2val(DimType i, DimType j) {
    return d2_a*i + j;
  }
  KOKKOS_FUNCTION
  static float v3val(DimType i, DimType j, DimType k) {
    return d3_a*d3_b*i + j*d3_a + k;
  }
  KOKKOS_FUNCTION
  static int v4val(DimType i, DimType j) {
    return j == 0 ? d4_a*i*2 + 1 : d4_a*i*2 + 2;
  }

  /* Check that all values are golden and match with expected generators */
  static void checkIsGolden(Data const& in) {
    EXPECT_EQ(in.val1,gold_val1);
    EXPECT_EQ(in.val2,gold_val2);
    EXPECT_EQ(in.vec.size(),3UL);
    EXPECT_EQ(in.vec[0],gold_val3+0);
    EXPECT_EQ(in.vec[1],gold_val3+1);
    EXPECT_EQ(in.vec[2],gold_val3+2);
    EXPECT_EQ(in.v0.size(),1UL);
    EXPECT_EQ(in.v1.size(),d1_a);
    EXPECT_EQ(in.v2.size(),d2_a*d2_b);
    EXPECT_EQ(in.v3.size(),d3_a*d3_b*d3_c);
    EXPECT_EQ(in.v4.size(),d4_a*2);

    auto host_v0 = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.v0);
    auto host_v1 = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.v1);
    auto host_v2 = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.v2);
    auto host_v3 = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.v3);
    auto host_v4 = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.v4);

    EXPECT_EQ(host_v0.operator()(), v0val());
    for (DimType i = 0; i < d1_a; i++) {
      EXPECT_EQ(host_v1.operator()(i), v1val(i));
    }
    for (DimType i = 0; i < d2_a; i++) {
      for (DimType j = 0; j < d2_b; j++) {
        EXPECT_EQ(host_v2.operator()(i,j), v2val(i,j));
      }
    }
    for (DimType i = 0; i < d3_a; i++) {
      for (DimType j = 0; j < d3_b; j++) {
        for (DimType k = 0; k < d3_c; k++) {
          EXPECT_EQ(host_v3.operator()(i,j,k), v3val(i,j,k));
        }
      }
    }
    for (DimType i = 0; i < d4_a; i++) {
      EXPECT_EQ(host_v4.operator()(i,0), v4val(i,0));
      EXPECT_EQ(host_v4.operator()(i,1), v4val(i,1));
    }

    #if MAGISTRATE_KOKKOS_KERNELS_ENABLED
    std::vector< std::vector< int > > graph( d1_a );

    for ( size_t i = 0 ; i < d1_a; ++i ) {
       graph[i].reserve(8);
       for ( size_t j = 0 ; j < 8 ; ++j ) {
         graph[i].push_back( i + j * 3 );
       }
    }

    // build a graph on the host
    //
    auto host_entries = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.crs.entries);
    auto host_row_map = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),in.crs.row_map);
    Data::Kokkos_HostCrsGraphType host_crs(host_entries,host_row_map);

    for ( size_t i = 0 ; i < d1_a ; ++i ) {
      const size_t begin = host_crs.row_map[i];
      const size_t n = host_crs.row_map[i+1] - begin ;
      EXPECT_EQ( n , graph[i].size() );
      for ( size_t j = 0 ; j < n ; ++j ) {
        EXPECT_EQ( (int) host_crs.entries( j + begin ) , graph[i][j] );
      }
    }
    //--- Check that the output matrix is stored as expected
    int nrow = 5, ncol = 7, nnz = 11;
    std::vector<int> rows{0, 2, 4, 5, 7, 11};
    std::vector<int> cols{0, 1, 1, 3, 2, 0, 3, 0, 2, 4, 6};
    std::vector<double> values{1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1, 10.1, 11.1};
    //
    EXPECT_EQ( nrow, (int) in.crs_mat.numRows());
    EXPECT_EQ( ncol, (int) in.crs_mat.numCols());
    EXPECT_EQ( nnz, (int) in.crs_mat.nnz());
    //

    Data::Kokkos_HostCrsMatrix host_crs_mat("host_crs_mat",in.crs_mat);

    for (int ir = 0; ir < host_crs_mat.numRows(); ++ir) {
      auto myRow = host_crs_mat.row(ir);
      for (int jc = 0; jc < myRow.length; ++jc) {
        EXPECT_EQ( cols[rows[ir] + jc], myRow.colidx(jc) );
        EXPECT_EQ( values[rows[ir] + jc], myRow.value(jc) );
      }
    }
    #endif
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    BaseData::serialize<SerializerT>(s);
    s | vec;
    s | val1 | val2;
    s | v0 | v1 | v2 | v3 | v4;
    #if MAGISTRATE_KOKKOS_KERNELS_ENABLED
    s | crs;
    s | crs_mat;
    #endif
  }

public:
  std::vector<int> vec = {};
  int val1 = 1, val2 = 2;
  Kokkos_ViewType0 v0;
  Kokkos_ViewType1 v1;
  Kokkos_ViewType2 v2;
  Kokkos_ViewType3 v3;
  Kokkos_ViewType4 v4;
  #if MAGISTRATE_KOKKOS_KERNELS_ENABLED
  Kokkos_CrsType crs;
  Kokkos_CrsMatrix crs_mat;
  #endif
};

struct KokkosBaseTest : virtual testing::Test { };

#endif /*INCLUDED_TESTS_UNIT_TEST_KOKKOS_INTEGRATION_COMMONS_H*/
