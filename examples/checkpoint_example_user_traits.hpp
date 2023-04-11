#include "checkpoint/checkpoint.h"

struct CheckpointingTrait {};

struct ShallowTrait {};

namespace test {
  struct RandomTrait {};
  
  struct TestObj {
    int a = 1;

    TestObj() {}
  
    template<typename SerializerT>
    void serialize(SerializerT& s){
      if (checkpoint::hasTrait<CheckpointingTrait>(s)){
        if(s.isSizing()) printf("Customizing serialization for checkpoint\n");
        s | a;
      } else {
        if(s.isSizing()) printf("Default serializing testObj\n");
      }

      static_assert(not checkpoint::has_trait_v<SerializerT, ShallowTrait>, "ShallowTrait should have been removed!\n");
    }
  };
}


namespace test {
  template<typename SerializerT, std::enable_if_t<checkpoint::has_trait_v<SerializerT, RandomTrait> >* = nullptr>
  void serialize(SerializerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Inserting random extra object serialization step! ");
    myObj.serialize(s);
  }

  template<typename SerializerT, std::enable_if_t< checkpoint::has_trait_v<SerializerT, ShallowTrait>>* = nullptr>
  void serialize(SerializerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Removing shallow trait before passing along!\n");
    
    auto& newS = checkpoint::withoutTraits<ShallowTrait>(s);
    
    myObj.serialize(newS);
  }
}

namespace misc {
  template<typename SerializerT, std::enable_if_t<checkpoint::has_trait_v<SerializerT, test::RandomTrait> >* = nullptr>
  void serialize(SerializerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("Serializers in other namespaces don't usually get found ");
    myObj.serialize(s);
  }


  struct NamespaceTrait {};
  template<typename SerializerT, std::enable_if_t<checkpoint::has_trait_v<SerializerT, NamespaceTrait> >* = nullptr>
  void serialize(SerializerT& s, test::TestObj& myObj){
    if(s.isSizing()) printf("A misc:: trait means we can serialize from misc:: too: ");
    myObj.serialize(s);
  }

  
  struct HookAllTrait {};
  template<typename SerializerT, typename T, std::enable_if_t<checkpoint::has_trait_v<SerializerT, HookAllTrait> >* = nullptr>
  void serialize(SerializerT& s, T& myObj){
    if(s.isSizing()) printf("We can even add on a generic pre-serialize hook: ");
    myObj.serialize(s);
  }
}
