#include "checkpoint/checkpoint.h"

struct CheckpointingTrait {};
struct RandomTrait {};

struct ShallowTrait {};

namespace test {
  struct TestObj {
    int a = 1;

    TestObj() {}
  
    template<typename SerializerT>
    void serialize(SerializerT& s){
      if constexpr (checkpoint::hasTrait<CheckpointingTrait, SerializerT>::value){
        if(s.isSizing()) printf("Customizing serialization for checkpoint\n");
        s | a;
      } else {
        if(s.isSizing()) printf("Default serializing testObj\n");
      }

      static_assert(not checkpoint::hasTrait<ShallowTrait, SerializerT>::value, "ShallowTrait should have been removed!\n");
    }
  };
}


namespace test {
  template<typename SerializerT, std::enable_if_t< checkpoint::hasTrait<RandomTrait, SerializerT>::value >* = nullptr>
  void serialize(SerializerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Inserting random extra object serialization step! ");
    myObj.serialize(s);
  }

  template<typename SerializerT, std::enable_if_t< checkpoint::hasTrait<ShallowTrait, SerializerT>::value >* = nullptr>
  void serialize(SerializerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Removing shallow trait before passing along!\n");
    
    auto& newS = checkpoint::withoutTrait<ShallowTrait>(s);
    
    myObj.serialize(newS);
  }
}
