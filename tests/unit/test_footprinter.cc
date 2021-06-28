/*
//@HEADER
// *****************************************************************************
//
//                             test_footprinter.cc
//                 DARMA/checkpoint => Serialization Library
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

#include <gtest/gtest.h>

#include "test_harness.h"

#include <atomic>
#include <thread>
#include <checkpoint/checkpoint.h>
#include <checkpoint/traits/serializable_traits.h>

struct ompi_communicator_t;
struct ompi_group_t;
struct ompi_request_t;
struct ompi_win_t;

namespace checkpoint { namespace tests { namespace unit {

struct TestFootprinter : TestHarness { };

const std::size_t sample_size = 5;

struct Test1 {
  double d;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | d;

    s.addBytes(sample_size);
  }
};

struct Test2 {
  float f;
};

template <typename Serializer>
void serialize(Serializer& s, Test2 t) {
  s | t.f;
}

struct TestBase {

  checkpoint_virtual_serialize_root()

  virtual ~TestBase() = default;

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | a;
  }

private:
  int a = 0;
};

struct TestDerived2 : TestBase {
  explicit TestDerived2(int i) {}
  explicit TestDerived2(SERIALIZE_CONSTRUCT_TAG) {}

  checkpoint_virtual_serialize_derived_from(TestBase)

  template <
    typename SerializerT,
    typename = std::enable_if_t<
      std::is_same<
        SerializerT,
        checkpoint::Footprinter
      >::value
    >
  >
  void serialize(SerializerT& s) {
    s | raw_pointer;
    s | shared_pointer;
  }

private:
  std::shared_ptr<int> shared_pointer = std::make_shared<int>(5);
  int* raw_pointer = nullptr;
};


TEST_F(TestFootprinter, test_fundamental_types) {
  int i;
  std::size_t size = 7;
  EXPECT_EQ(checkpoint::getMemoryFootprint(i), sizeof(i));
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(i, size),
    sizeof(i) + size
  );

  double d;
  EXPECT_EQ(checkpoint::getMemoryFootprint(d), sizeof(d));
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(d, size),
    sizeof(d) + size
  );

  auto str = "12345";
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(str),
    sizeof(str) + sizeof(*str)
    // actually: sizeof(str) + 6 * sizeof(str[0])
  );

  {
    int* ptr = nullptr;
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr));
  }

  {
    int* ptr = new int();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr) + sizeof(*ptr)
    );
    delete ptr;
  }

  {
    int j = 3;
    void* ptr = static_cast<void*>(&j);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr)
    );
  }

  {
    int *arr = new int[5];
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(arr),
      sizeof(arr) + sizeof(*arr)
      // actually: sizeof(arr) + 5 * sizeof(arr[0])
    );
    delete [] arr;
  }

  {
    int arr[] = {1, 2, 3, 4, 5};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(arr),
      sizeof(arr)
      // actually: sizeof(&arr) + sizeof(arr)
    );
  }
}

TEST_F(TestFootprinter, test_array) {
  std::array<int, 3> a = {1, 2, 3};
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(a),
    a.size() * sizeof(a[0])
  );
}

TEST_F(TestFootprinter, test_atomic) {
  std::atomic<bool> atomic_bool{false};
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(atomic_bool),
    sizeof(bool)
  );
}

TEST_F(TestFootprinter, test_chrono_duration) {
  std::chrono::seconds sec(1);
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(sec),
    sizeof(sec)
  );

  {
    std::chrono::milliseconds milisec{300};
    auto ms_ret = checkpoint::serialize(milisec);
    auto deserialized =
      checkpoint::deserialize<std::chrono::milliseconds>(ms_ret->getBuffer());
    EXPECT_EQ(milisec, *deserialized);
  }

  {
    std::chrono::hours hrs{24};
    auto hrs_ret = checkpoint::serialize(hrs);
    auto deserialized =
      checkpoint::deserialize<std::chrono::hours>(hrs_ret->getBuffer());
    EXPECT_EQ(hrs, *deserialized);
  }
}

TEST_F(TestFootprinter, test_deque) {
  std::deque<int> d = {1, 2, 3};
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(d),
    sizeof(d) + d.size() * sizeof(d.front())
  );
}

TEST_F(TestFootprinter, test_enum) {
  {
    enum color
    {
      red,
      green,
      blue
    };
    color col = red;

    EXPECT_EQ(checkpoint::getMemoryFootprint(col), sizeof(col));
  }

  {
    enum struct color : char
    {
      red = 'r',
      green = 'g',
      blue = 'b'
    };
    color col = color::green;

    EXPECT_EQ(checkpoint::getMemoryFootprint(col), sizeof(char));
  }
}

void fn() {
  std::cout << 1;
}

TEST_F(TestFootprinter, test_function) {
  std::function<void()> f = fn;
  auto ptr = f.target<void(*)()>();
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(f),
    sizeof(f)
  );
}

TEST_F(TestFootprinter, test_list) {
  std::list<int> l = {1, 2, 3};
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(l),
    sizeof(l) + l.size() * sizeof(l.front())
  );

  auto it = l.begin();
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(it),
    sizeof(it)
  );
}

TEST_F(TestFootprinter, test_map) {
  {
    std::map<int, char> m = {{1, 'a'}, {2, 'b'}, {3, 'c'}};
    auto p = *m.begin();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(m),
      sizeof(m) + m.size() * (sizeof(p) + sizeof(p.first) + sizeof(p.second))
    );
  }

  {
    std::unordered_map<int, char> m = {{1, 'a'}, {2, 'b'}, {3, 'c'}};
    auto p = *m.begin();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(m),
      sizeof(m) + m.size() * (sizeof(p) + sizeof(p.first) + sizeof(p.second))
    );
  }

  {
    std::multimap<int, char> m = {{1, 'a'}, {1, 'b'}, {1, 'c'}};
    auto p = *m.begin();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(m),
      sizeof(m) + m.size() * (sizeof(p) + sizeof(p.first) + sizeof(p.second))
    );
  }

  {
    std::unordered_multimap<int, char> m = {{1, 'a'}, {1, 'b'}, {1, 'c'}};
    auto p = *m.begin();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(m),
      sizeof(m) + m.size() * (sizeof(p) + sizeof(p.first) + sizeof(p.second))
    );
  }

  {
    std::unordered_map<int, std::unique_ptr<double>> m;
    m[1] = std::make_unique<double>(10.0);
    m[2] = std::make_unique<double>(20.0);
    m[3] = std::make_unique<double>(30.0);

    auto& elm = *m.begin();
    auto elm_size = sizeof(elm)                    // std::pair
      + sizeof(elm.first)                          // int
      + sizeof(elm.second) + sizeof(*elm.second);  // std::unique_ptr

    EXPECT_EQ(
      checkpoint::getMemoryFootprint(m),
      sizeof(m) + m.size() * elm_size
    );
  }
}

TEST_F(TestFootprinter, test_shared_ptr) {
  {
    std::shared_ptr<Test1> ptr;
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr));
  }

  {
    auto ptr = std::make_shared<Test2>();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr) + sizeof(*ptr)
    );
  }
}

TEST_F(TestFootprinter, test_queue) {
  {
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(q),
      sizeof(q) + q.size() * sizeof(q.front())
    );
    EXPECT_EQ(q.size(), std::size_t{3});
  }

  {
    std::priority_queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(q),
      sizeof(q) + q.size() * sizeof(q.top())
    );
    EXPECT_EQ(q.size(), std::size_t{3});
  }
}

TEST_F(TestFootprinter, test_set) {
  {
    std::set<int> s = {1, 2, 3};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(s),
      sizeof(s) + s.size() * sizeof(*s.begin())
    );
  }

  {
    std::unordered_set<int> s = {1, 2, 3};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(s),
      sizeof(s) + s.size() * sizeof(*s.begin())
    );
  }

  {
    std::multiset<int> s = {1, 1, 1};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(s),
      sizeof(s) + s.size() * sizeof(*s.begin())
    );
  }

  {
    std::unordered_multiset<int> s = {1, 1, 1};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(s),
      sizeof(s) + s.size() * sizeof(*s.begin())
    );
  }

  {
    std::set<std::unique_ptr<int>> s;
    s.insert(std::make_unique<int>(100));
    s.insert(std::make_unique<int>(200));
    s.insert(std::make_unique<int>(300));

    auto& elm = *s.begin();
    auto elm_size = sizeof(elm) + sizeof(*elm);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(s),
      sizeof(s) + s.size() * elm_size
    );
  }
}

TEST_F(TestFootprinter, test_stack) {
  {
    std::stack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(stack),
      sizeof(stack) + stack.size() * sizeof(stack.top())
    );
    EXPECT_EQ(stack.size(), std::size_t{3});
  }
}

// does not account for small string optimisation
TEST_F(TestFootprinter, test_string) {
  std::string s = "123456789";
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(s),
    sizeof(s) + s.capacity() * sizeof(s[0])
  );
}

TEST_F(TestFootprinter, test_thread) {
  std::thread t(fn);
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(t),
    sizeof(t)
  );
  t.detach();
}

// naive approach, just sum memory usage of all elements
TEST_F(TestFootprinter, test_tuple) {
  int i         = 1;
  double d      = 3.8;
  std::string s = "123456789";
  auto t = std::make_tuple(i, d, s);
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(t),
    sizeof(i) + sizeof(d) + sizeof(s) + s.capacity() * sizeof(s[0])
  );
}

TEST_F(TestFootprinter, test_unique_ptr) {
  {
    std::unique_ptr<Test1> ptr;
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr));
  }

  {
    auto ptr = std::make_unique<Test1>();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr) + sizeof(*ptr) + sample_size
    );
  }

  {
    auto ptr = std::make_unique<Test2>();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr) + sizeof(*ptr)
    );
  }
}

TEST_F(TestFootprinter, test_vector) {
  {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(v[0])
    );
  }

  {
    std::vector<Test1*> v = { new Test1(), nullptr };
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(v[0]) + sizeof(*v[0]) + sample_size
    );
    delete v[0];
  }

  {
    std::vector<bool> v = { false, true, false, true, true };
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v)
    );
  }

  {
    std::vector<TestDerived2> v;
    EXPECT_GE(
      checkpoint::getMemoryFootprint(v),
      sizeof(v)
    );

    v.reserve(3);
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(TestDerived2)
    );
  }
}

TEST_F(TestFootprinter, test_virtual_serialize) {
  {
    std::unique_ptr<TestBase> ptr = std::make_unique<TestDerived2>(0);
    checkpoint::getMemoryFootprint(ptr);
  }

  {
    std::shared_ptr<TestBase> ptr = std::make_shared<TestDerived2>(0);
    checkpoint::getMemoryFootprint(ptr);
  }
}

TEST_F(TestFootprinter, test_ompi) {
  {
    std::vector<ompi_communicator_t*> v(3);

    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(ompi_communicator_t*)
    );
  }

  {
    std::vector<ompi_group_t*> v(5);

    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(ompi_group_t*)
    );
  }

  {
    std::vector<ompi_request_t*> v(7);

    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(ompi_request_t*)
    );
  }

  {
    std::vector<ompi_win_t*> v(9);

    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(ompi_win_t*)
    );
  }
}

struct TestNoSerialize {
  double d;
  int i;
};

static_assert(
  not SerializableTraits<TestNoSerialize>::is_traversable,
  "TestNoSerialize has no serializer defined"
);

TEST_F(TestFootprinter, test_no_serialize) {
  std::vector<TestNoSerialize> v(7);

  EXPECT_EQ(
    checkpoint::getMemoryFootprint(v),
    sizeof(v) + v.capacity() * sizeof(TestNoSerialize)
  );

  std::unordered_map<int, TestNoSerialize> m;
  m[1] = TestNoSerialize();
  m[2] = TestNoSerialize();
  m[3] = TestNoSerialize();
  auto p = *m.begin();

  EXPECT_EQ(
    checkpoint::getMemoryFootprint(m),
    sizeof(m) + m.size() * (sizeof(p) + sizeof(p.first) + sizeof(p.second))
  );
}
}}} // end namespace checkpoint::tests::unit
