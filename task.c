#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "task.h"
#include "common.h"
#include "sockets.h"

typedef struct {
  /* Either ARG_BY_REF or ARG_BY_VAL. */
  int8_t type;
  union {
    unique_id id;
    struct {
      ptrdiff_t val_offset;
      int64_t val_length;
    } val;
  };
} task_arg;

struct task_spec_impl {
  function_id func_id;
  int64_t num_args;
  /* Position of the argument that is currently constructed. */
  int64_t arg_cursor;
  int64_t num_returns;
  int64_t args_value_size;
  task_arg ids[0];
};

task_spec *alloc_task_spec(function_id func_id, int64_t num_args, int64_t num_returns, int64_t args_value_size) {
  int64_t size = sizeof(task_spec) + (num_args + num_returns) * sizeof(task_arg) + args_value_size;
  task_spec *task = malloc(size);
  memset(task, 0, size);
  task->func_id = func_id;
  task->num_args = num_args;
  task->num_returns = num_returns;
  task->args_value_size = args_value_size;
  return task;
}

int64_t task_num_args(task_spec* spec) {
  return spec->num_args;
}

int64_t task_num_returns(task_spec* spec) {
  return spec->num_returns;
}

int8_t task_arg_type(task_spec* spec, int64_t arg_index) {
  return spec->ids[arg_index].type;
}

unique_id *task_arg_id(task_spec *spec, int64_t arg_index) {
  task_arg* arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_REF)
  return &arg->id;
}

uint8_t *task_arg_val(task_spec *spec, int64_t arg_index) {
  task_arg* arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_VAL);
  uint8_t *data = (uint8_t*) &spec->ids[0];
  data += (spec->num_args + spec->num_returns) * sizeof(task_arg);
  return data + arg->val.val_offset;
}

int64_t task_arg_length(task_spec *spec, int64_t arg_index) {
  task_arg* arg = &spec->ids[arg_index];
  CHECK(arg->type == ARG_BY_VAL);
  return arg->val.val_length;
}

void task_arg_set_ref(task_spec *spec, int64_t arg_index, unique_id id) {
  task_arg* arg = &spec->ids[arg_index];
  arg->type = ARG_BY_REF;
  arg->id = id;
}

int64_t task_args_add_ref(task_spec *spec, unique_id id);
int64_t task_args_add_val(task_spec *spec, uint8_t *data, int64_t length);

void task_arg_set_val(task_spec *spec, int64_t arg_index, uint8_t *data, int64_t length) {
  task_arg *arg = &spec->ids[arg_index];
  arg->type = ARG_BY_VAL;
  uint8_t *addr = task_arg_val(spec, arg_index);
  memcpy(addr, data, length);
  
}

unique_id *task_return(task_spec* spec, int64_t ret_index) {
  return &spec->ids[spec->num_args + ret_index].id;
}

void free_task_spec(task_spec *spec) {
  free(spec);
}
