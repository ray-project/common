#ifndef TASK_TABLE_H
#define TASK_TABLE_H

#include "db.h"
#include "task.h"

/* Add task to the task table. */
void task_table_add_task(db_conn *db, unique_id task_id, task_spec *task);

/* Get specific task from the task table. */
task_spec *task_table_get_task(db_conn *db, unique_id task_id);


#endif
