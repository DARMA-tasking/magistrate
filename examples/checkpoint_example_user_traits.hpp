#include "checkpoint/checkpoint.h"

struct checkpoint_trait {} CheckpointTrait;
struct shallow_trait {} ShallowTrait;

namespace test {
  struct random_trait {} RandomTrait;

  struct TestObj {
    int a = 1;

    TestObj() {}

    template<typename SerT, typename SerT::template has_not_traits<shallow_trait>::type = nullptr>
    void serialize(SerT& s){
      if constexpr(SerT::template has_traits<checkpoint_trait>::value){
        if(s.isSizing()) printf("Customizing serialization for checkpoint\n");
        s | a;
      } else {
        if(s.isSizing()) printf("Default serializing testObj\n");
      }

      static_assert(SerT::template has_not_traits<shallow_trait>::value, "ShallowTrait should have been removed!\n");
    }
  };
}

namespace test {
  template<typename SerT, typename SerT::template has_traits<random_trait>::type = nullptr>
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Inserting random extra object serialization step! ");
    myObj.serialize(s);
  }

  template<typename SerT, typename SerT::template has_traits<shallow_trait>::type = nullptr>
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Removing shallow trait before passing along!\n");
    auto newS = s.template withoutTraits<shallow_trait>();
    myObj.serialize(newS);
  }
}

namespace misc {
  template<typename SerT, typename SerT::template has_traits<test::random_trait>::type = nullptr>
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("Serializers in other namespaces don't usually get found ");
    myObj.serialize(s);
  }


  const struct namespace_trait {} NamespaceTrait;
  template<typename SerT, typename SerT::template has_traits<namespace_trait>::type = nullptr>
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("A misc:: trait means we can serialize from misc:: too: ");
    myObj.serialize(s);
  }


  const struct hook_all_trait {} HookAllTrait;
  template<typename SerT, typename T, typename SerT::template has_traits<hook_all_trait>::type = nullptr>
  void serialize(SerT& s, T& myObj){
    if(s.isSizing()) printf("We can even add on a generic pre-serialize hook: ");
    auto newS = s.template withoutTraits<hook_all_trait>();
    myObj.serialize(newS);
  }
}
