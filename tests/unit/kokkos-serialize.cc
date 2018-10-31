#if KOKKOS_ENABLED_SERDES

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

//TEST_F(KokkosViewTest, kokkosViewTest)
//{
//  const size_t N = 10;
//  Kokkos::View<double*[3]> k("A", N);

//  Kokkos::parallel_for(10, KOKKOS_LAMBDA(int i)
//  {
//                         // Read and write access to data comes via operator()
//                         k(i ,0) = 1.0*i ;
//                         k(i ,1) = 1.0*i*i ;
//                         k(i ,2) = 1.0*i*i*i ;
//                       });

//  serdes::Packer s(1000);
//  serdes::serialize(s, k);

//  Kokkos::View<double*[3]> outk;
//  serdes::Unpacker us(s.getBuffer(),1000);
//  serdes::serialize(us, outk);

//  compareKokkosView(k, outk, 2);
//}

//TEST_F(KokkosViewTest, kokkosViewTestLeftLayout)
//{
//  const size_t N = 10;
//  Kokkos::View<double*[3], Kokkos::LayoutLeft> k("A", N);

//  Kokkos::parallel_for(10, KOKKOS_LAMBDA(int i)
//  {
//                         // Read and write access to data comes via operator()
//                         k(0 ,i) = 1.0*i ;
//                         k(1 ,i) = 1.0*i*i ;
//                         k(2 ,i) = 1.0*i*i*i ;
//                       });

//  serdes::Packer s(1000);
//  serdes::serialize(s, k);

//  Kokkos::View<double*[3], Kokkos::LayoutLeft> outk;
//  serdes::Unpacker us(s.getBuffer(),1000);
//  serdes::serialize(us, outk);

//  compareKokkosView(k, outk, 2);
//}

#if 0

TEST_F(KokkosViewTest, kokkosViewTestStrideLayout)
{
  const size_t N =7;
  const size_t M =3;
  Kokkos::View<double**, Kokkos::LayoutStride> view("test", 8, 9);
  Kokkos::View<double**, Kokkos::LayoutStride > k("A", N, 1,N, 1);
  std::cout <<" size of view just after it declaration " << k.size() << std::endl;
  size_t strides[3];
  k.stride(strides); // fill 'strides' with strides

  Kokkos::parallel_for(N, KOKKOS_LAMBDA(int i)
  {
                         // Read and write access to data comes via operator()
                         for(auto j = 0; j< N; ++j)
                         {
                           k(i,1, j,1) = 1.0* pow(i+ 1,j + 1);
                           std::cout <<" size " << i * N + j << " : " << k(i, 1, j, 1) << std::endl;

                         }

                       });
  std::cout <<" Strides " << strides[0] << " "<< strides[1] << " " << std::endl;
  std::cout <<" extents " << k.extent(0) << " "<< k.extent(1) << " "<< k.extent(2) << " "<< k.extent(3) << " "<< k.extent(4) << " "<< k.extent(5) << " "<< k.extent(6) << " "<< k.extent(7) << " "<< k.extent(8) << " " << std::endl;
  std::cout <<" size " << k.size() << std::endl;

  std::cout <<" Init of View with Stride Layout OK " << std::endl;
  serdes::Packer s(1000);
  serdes::serialize(s, k);

  Kokkos::View<double**, Kokkos::LayoutStride> outk;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outk);

  compareKokkosView(k, outk, 2);
}

#endif

#endif
