#ifndef TASK_TABLE_H
#define TASK_TABLE_H

#include "db.h"

/* Construct and modify task specifications */
task_spec *alloc_task_spec(unique_id function_id, int64_t num_args, int64_t num_returns);
int64_t task_num_args(task_spec* spec);
int64_t task_num_returns(task_spec* spec);
unique_id *task_args(task_spec* spec);
unique_id *task_returns(task_spec* spec);
free_task_spec(task_spec *spec);

/* Add task to the task table. */
void task_table_add_task(db_conn *db, unique_id task_id, task_spec *task);

/* Get specific task from the task table. */
task_spec *task_table_get_task(db_conn *db, unique_id task_id);

/*

typedef struct {
  unique_id function_id;
  int64_t num_args;
  int64_t num_returns;
  unique_id ids[1];
} task_spec;

*/

#endif
