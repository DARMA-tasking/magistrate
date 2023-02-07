#include "checkpoint/checkpoint.h"

#include "checkpoint_example_user_traits.hpp"

int main(int argc, char *argv[]){
  test::TestObj obj;

  auto s_info = checkpoint::serialize(obj);
  s_info = checkpoint::serialize<test::TestObj, CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<test::TestObj, CheckpointingTrait, CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<test::TestObj, RandomTrait, CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<test::TestObj, CheckpointingTrait, RandomTrait>(obj);
  s_info = checkpoint::serialize<test::TestObj, RandomTrait, RandomTrait>(obj);
  s_info = checkpoint::serialize<test::TestObj, ShallowTrait>(obj);
}
