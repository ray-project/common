#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "task.h"
#include "common.h"
#include "sockets.h"

/* Tasks are stored in a consecutive chunk of memory, the first
 * sizeof(task_spec) bytes are arranged acording to the struct
 * task_spec. Then there is an array of task_args of length
 * (num_args + num_returns), and then follows the data of
 * pass-by-value arguments of size args_value_size. The offsets in the
 * task_arg.val are with respect to the end of the struct, i.e.
 * with respect to the address &task_spec.ids[0]. */

typedef struct {
  /* Either ARG_BY_REF or ARG_BY_VAL. */
  int8_t type;
  union {
    unique_id arg_id;
    struct {
      /* Offset where the data associated to this arg is located relative
       * to &task_spec.ids[0]. */
      ptrdiff_t offset;
      int64_t length;
    } val;
  };
} task_arg;

struct task_spec_impl {
  function_id func_id;
  /* Total number of arguments. */
  int64_t num_args;
  /* Position of the argument that is currently constructed. */
  int64_t arg_cursor;
  /* Number of return values. */
  int64_t num_returns;
  /* Number of bytes the pass-by-value arguments are occupying. */
  int64_t args_value_size;
  /* Offset in the argument data. */
  int64_t args_value_cursor;
  /* Argument and return IDs. */
  task_arg ids[0];
};

task_spec *alloc_task_spec(function_id func_id,
                           int64_t num_args,
                           int64_t num_returns,
                           int64_t args_value_size) {
  int64_t size = sizeof(task_spec) +
                 (num_args + num_returns) * sizeof(task_arg) + args_value_size;
  task_spec *task = malloc(size);
  memset(task, 0, size);
  task->func_id = func_id;
  task->num_args = num_args;
  task->arg_cursor = 0;
  task->num_returns = num_returns;
  task->args_value_size = args_value_size;
  return task;
}

int64_t task_num_args(task_spec *spec) {
  return spec->num_args;
}

int64_t task_num_returns(task_spec *spec) {
  return spec->num_returns;
}

int8_t task_arg_type(task_spec *spec, int64_t arg_index) {
  return spec->ids[arg_index].type;
}

unique_id *task_arg_id(task_spec *spec, int64_t arg_index) {
  task_arg *arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_REF)
  return &arg->arg_id;
}

uint8_t *task_arg_val(task_spec *spec, int64_t arg_index) {
  task_arg *arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_VAL);
  uint8_t *data = (uint8_t *) &spec->ids[0];
  data += (spec->num_args + spec->num_returns) * sizeof(task_arg);
  return data + arg->val.offset;
}

int64_t task_arg_length(task_spec *spec, int64_t arg_index) {
  task_arg *arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_VAL);
  return arg->val.length;
}

int64_t task_args_add_ref(task_spec *spec, unique_id arg_id) {
  task_arg *arg = &spec->ids[spec->arg_cursor];
  arg->type = ARG_BY_REF;
  arg->arg_id = arg_id;
  return spec->arg_cursor++;
}

int64_t task_args_add_val(task_spec *spec, uint8_t *data, int64_t length) {
  task_arg *arg = &spec->ids[spec->arg_cursor];
  arg->type = ARG_BY_VAL;
  arg->val.offset = spec->args_value_cursor;
  arg->val.length = length;
  uint8_t *addr = task_arg_val(spec, spec->arg_cursor);
  CHECK(spec->args_value_cursor + length <= spec->args_value_size);
  memcpy(addr, data, length);
  spec->args_value_cursor += length;
  return spec->arg_cursor++;
}

unique_id *task_return(task_spec *spec, int64_t ret_index) {
  return &spec->ids[spec->num_args + ret_index].arg_id;
}

void free_task_spec(task_spec *spec) {
  free(spec);
}
