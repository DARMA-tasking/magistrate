#include "checkpoint/checkpoint.h"

#include "checkpoint_example_user_traits.hpp"

int main(int argc, char *argv[]){
  test::TestObj obj;

  auto s_info = checkpoint::serialize(obj);
  s_info = checkpoint::serialize<CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<CheckpointingTrait, CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<test::RandomTrait, CheckpointingTrait>(obj);
  s_info = checkpoint::serialize<CheckpointingTrait, test::RandomTrait>(obj);
  s_info = checkpoint::serialize<test::RandomTrait, test::RandomTrait>(obj);
  s_info = checkpoint::serialize<ShallowTrait>(obj);
  s_info = checkpoint::serialize<misc::NamespaceTrait>(obj);
  s_info = checkpoint::serialize<misc::HookAllTrait>(obj);
}
