#ifndef TASK_H
#define TASK_H

/* This API specifies the task data structures. It is in C so we can
 * easily construct tasks from other languages like Python. The datastructures
 * are also defined in such a way that memory is contiguous and all pointers
 * are relative, so that we can memcpy the datastructure and ship it over the
 * network without serialization and deserialization. */

#include <stddef.h>
#include <stdint.h>
#include "common.h"
#include "utstring.h"

typedef unique_id function_id;
typedef unique_id object_id;

/* The task ID is a deterministic hash of the function ID that
 * the task executes and the argument IDs or argument values */
typedef unique_id task_id;

/* The task instance ID is a globally unique ID generated which
 * identifies this particular execution of the task */
typedef unique_id task_iid;

/* The node id is an identifier for the node the task is
 * scheduled on */
typedef unique_id node_id;

/*
 * TASK SPECIFICATIONS: Contain all the information neccessary
 * to execute the task (function id, arguments, return object ids).
 * 
 */

typedef struct task_spec_impl task_spec;

/* If argument is passed by value or reference. */
enum arg_type { ARG_BY_REF, ARG_BY_VAL };

/* Construct and modify task specifications. */

/* Allocating and initializing a task. */
task_spec *alloc_task_spec(function_id function_id,
                           int64_t num_args,
                           int64_t num_returns,
                           int64_t args_value_size);

/* Size of the task in bytes. */
int64_t task_size(task_spec *spec);

/* Return the function ID of the task. */
unique_id *task_function(task_spec *spec);

/* Getting the number of arguments and returns. */
int64_t task_num_args(task_spec *spec);
int64_t task_num_returns(task_spec *spec);

/* Getting task arguments. */
int8_t task_arg_type(task_spec *spec, int64_t arg_index);
unique_id *task_arg_id(task_spec *spec, int64_t arg_index);
uint8_t *task_arg_val(task_spec *spec, int64_t arg_index);
int64_t task_arg_length(task_spec *spec, int64_t arg_index);

/* Setting task arguments. Note that this API only allows you to set the
 * arguments in their order of appearance. */
int64_t task_args_add_ref(task_spec *spec, object_id obj_id);
int64_t task_args_add_val(task_spec *spec, uint8_t *data, int64_t length);

/* Getting and setting return arguments. Tasks return by reference for now. */
unique_id *task_return(task_spec *spec, int64_t ret_index);

/* Freeing the task datastructure. */
void free_task_spec(task_spec *spec);

/* Write the task specification to a file or socket. */
void write_task(int fd, task_spec *spec);

/* Read the task specification from a file or socket. It is the user's
 * responsibility to free the task after it has been used. */
task_spec *read_task(int fd);

/* Print task as a humanly readable string. */
void print_task(task_spec *spec, UT_string *output);

/* Parse task as printed by print_task. */
task_spec *parse_task(char *task_string, int64_t task_length);

/*
 * SCHEDULED TASK: Contains information about a scheduled task:
 * the task iid, the task specification and the task status
 * (WAITING, SCHEDULED, RUNNING, DONE) and which node the
 * task is scheduled on.
 *
 */

/* The scheduling_state can be used as a flag when we are listening for an event,
 * for example TASK_WAITING | TASK_SCHEDULED. */
enum scheduling_state {
  TASK_WAITING = 0,
  TASK_SCHEDULED = 1,
  TASK_RUNNING = 2,
  TASK_DONE = 4
};

typedef struct scheduled_task_impl scheduled_task;

/* Allocate and initialize a new scheduled task. Must be freed with
 * scheduled_task_free after use. */
scheduled_task *make_scheduled_task(task_iid task_iid, task_spec *task, int32_t state, node_id node);

/* Size of task log entry in bytes. */
int64_t scheduled_task_size(scheduled_task *task);

/* Task instance id id. */
task_iid *scheduled_task_iid(scheduled_task *task);

/* Node this task has been scheduled on. */
node_id *scheduled_task_node(scheduled_task *task);

/* Task specification of this scheduled task. */
task_spec *scheduled_task_spec(scheduled_task *task);

/* Free this scheduled task. */
void scheduled_task_free(scheduled_task *task);

#endif
