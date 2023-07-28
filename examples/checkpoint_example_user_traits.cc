#include "checkpoint/checkpoint.h"

#include "checkpoint_example_user_traits.hpp"

int main(int argc, char *argv[]){
  test::TestObj obj;

  auto s_info = checkpoint::serialize(obj);
  s_info = checkpoint::serialize<checkpoint_trait>(obj);
  s_info = checkpoint::serialize<checkpoint_trait, checkpoint_trait>(obj);
  s_info = checkpoint::serialize<test::random_trait, checkpoint_trait>(obj);
  s_info = checkpoint::serialize<checkpoint_trait, test::random_trait>(obj);
  s_info = checkpoint::serialize<test::random_trait, test::random_trait>(obj);
  s_info = checkpoint::serialize<shallow_trait>(obj);
  s_info = checkpoint::serialize<misc::namespace_trait>(obj);
  s_info = checkpoint::serialize<misc::hook_all_trait>(obj);
}
