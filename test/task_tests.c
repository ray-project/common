#include "greatest.h"

#include "task.h"

SUITE(task_tests);

TEST task_test(void) {
  function_id func_id = {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  task_spec *task = alloc_task_spec(func_id, 3, 2, 10);
  ASSERT(task_num_args(task) == 3);
	ASSERT(task_num_returns(task) == 2);
  
  free_task_spec(task);
  PASS();
}

SUITE(task_tests) {
  RUN_TEST(task_test);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(task_tests);
  GREATEST_MAIN_END();
}
