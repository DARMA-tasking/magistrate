#include "checkpoint/checkpoint.h"

const struct checkpoint_trait {} CheckpointTrait;
const struct shallow_trait {} ShallowTrait;

namespace test {
  const struct random_trait {} RandomTrait;
  
  struct TestObj {
    int a = 1;

    TestObj() {}
  
    template<typename SerT, typename SerT::has_not_traits_t<shallow_trait>* = nullptr>
    void serialize(SerT& s){
      if (s.hasTraits(CheckpointTrait)){
        if(s.isSizing()) printf("Customizing serialization for checkpoint\n");
        s | a;
      } else {
        if(s.isSizing()) printf("Default serializing testObj\n");
      }

      static_assert(not s.hasTraits(ShallowTrait), "ShallowTrait should have been removed!\n");
    }
  };
}


namespace test {
  template<typename SerT, typename SerT::has_traits_t<random_trait>* = nullptr>
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Inserting random extra object serialization step! ");
    myObj.serialize(s);
  }

  template<typename SerT, typename SerT::has_traits_t<shallow_trait>* = nullptr>
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Removing shallow trait before passing along!\n");
    
    myObj.serialize(s.withoutTraits(ShallowTrait));
  }
}

namespace misc {
  template<typename SerT, typename SerT::has_traits_t<test::random_trait>* = nullptr>
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("Serializers in other namespaces don't usually get found ");
    myObj.serialize(s);
  }


  const struct namespace_trait {} NamespaceTrait;
  template<typename SerT, typename SerT::has_traits_t<namespace_trait>* = nullptr>
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("A misc:: trait means we can serialize from misc:: too: ");
    myObj.serialize(s);
  }

  
  const struct hook_all_trait {} HookAllTrait;
  template<typename SerT, typename T, typename SerT::has_traits_t<hook_all_trait>* = nullptr>
  void serialize(SerT& s, T& myObj){
    if(s.isSizing()) printf("We can even add on a generic pre-serialize hook: ");
    myObj.serialize(s.withoutTraits(HookAllTrait));
  }
}
