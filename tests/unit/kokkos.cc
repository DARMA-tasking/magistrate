#include "test_harness.h"
#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_Serial.hpp>
#include <container/array_serialize.h>
#include <container/view_serialize.h>
#include <container/string_serialize.h>
#include <container/vector_serialize.h>
#include <container/tuple_serialize.h>
#include <serializers/packer.h>
#include <serializers/unpacker.h>
#include <serdes_headers.h>

template<class ViewTypeA, class ViewTypeB>
constexpr bool isSameMemoryLayout (const ViewTypeA&, const ViewTypeB&) {
  typedef typename ViewTypeA::array_layout array_layoutA;
  typedef typename ViewTypeB::array_layout array_layoutB;
  return std::is_same<array_layoutA, array_layoutB>::value;
}

template<typename T>
void compareKokkosView(const T& k1, const T& k2, int dim){
  EXPECT_EQ(k1.label(), k2.label());
  EXPECT_EQ(k1.size(), k2.size());
  EXPECT_EQ(k1.is_contiguous(), k2.is_contiguous());
  EXPECT_EQ(k1.use_count(), k2.use_count());
  EXPECT_EQ(k1.span(), k2.span());

  EXPECT_EQ(isSameMemoryLayout(k1, k2), true);

  //Extent
  for(int i = 0; i < dim; ++i)
  {
    EXPECT_EQ(k1.extent(i), k2.extent(i));
  }

  //Data
  const auto k1Data = k1.data();
  const auto k2Data = k2.data();
  for(int i = 0; i < k1.size(); ++i)
  {
    //    std::cout << "data : " << i << " : " << k1Data[i]<< " ==? " <<k2Data[i] <<std::endl;
    EXPECT_EQ(k1Data[i] == k2Data[i], true);
  }

}

TEST(KOKKOS, stringTest)
{
  std::string str{"hello"};
  serdes::Packer s(1000);
  serdes::serialize(s, str);

  std::string outstr;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outstr);

  EXPECT_EQ(str, outstr);
}

TEST(KOKKOS, arrayOfStringSizeT)
{
  std::array<size_t, 2> array{3,7};

  serdes::Packer s(1000);
  serdes::serialize(s, array);

  std::array<size_t, 2> outarray;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outarray);
  EXPECT_EQ(array.size(), outarray.size());

  for(auto i = 0; i < array.size(); ++i)
  {
    //    std::cout <<" TEST arrayOfStringSizeT " << array.at(i) << " : "<< outarray.at(i)<< std::endl;
    EXPECT_EQ(array.at(i), outarray.at(i));
  }
}

TEST(KOKKOS, vectorOfStringTest)
{
  std::vector<std::string> vec;
  vec.emplace_back("hello");
  vec.emplace_back("world");

  serdes::Packer s(1000);
  serdes::serialize(s, vec);

  std::vector<std::string> outvec;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outvec);
  EXPECT_EQ(vec.size(), outvec.size());

  for(auto i = 0; i < vec.size(); ++i)
  {
    EXPECT_EQ(vec.at(i), outvec.at(i));
  }
}

class KokkosViewTest : public ::testing::Test {
protected:
  void SetUp() override {
    Kokkos::initialize();
  }

  void TearDown() override {
    Kokkos::finalize();
  }
};

TEST_F(KokkosViewTest, kokkosViewTest)
{
  const size_t N = 10;
  Kokkos::View<double*[2]> k("A", N);

  Kokkos::parallel_for(10, KOKKOS_LAMBDA(int i)
  {
                         // Read and write access to data comes via operator()
                         k(i ,0) = 1.0*i ;
                         k(i ,1) = 1.0*i*i ;
                         k(i ,2) = 1.0*i*i*i ;
                       });

  serdes::Packer s(1000);
  serdes::serialize(s, k);

  Kokkos::View<double*[2]> outk;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outk);

  compareKokkosView(k, outk, 2);
}

//TEST_F(KokkosViewTest, kokkosViewTest2)
//{
//  const size_t N = 10;
//  Kokkos::View<double*[2], Kokkos::LayoutStride> k("A", N);
//  std::cout << "size : " << k.size() <<std::endl;

//  Kokkos::parallel_for(10, KOKKOS_LAMBDA(int i)
//  {
//                         // Read and write access to data comes via operator()
//                         k(i ,0) = 1.0*i ;
//                         k(i ,1) = 1.0*i*i ;
//                         k(i ,2) = 1.0*i*i*i ;
//                       });
//  std::cout << "// for OK " <<std::endl;

//  serdes::Packer s(1000);
//  serdes::serialize(s, k);

//  Kokkos::View<double*[2], Kokkos::LayoutStride> outk;
//  serdes::Unpacker us(s.getBuffer(),1000);
//  serdes::serialize(us, outk);

//  compareKokkosView(k, outk, 2);
//}

