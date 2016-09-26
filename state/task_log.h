#ifndef TASK_LOG_H
#define TASK_LOG_H

#include "db.h"
#include "task.h"

/* The task ID is a deterministic hash of the function ID that
 * the task executes and the argument IDs or argument values */
typedef unique_id task_id;

/* The task instance ID is a globally unique ID generated which
 * identifies this particular execution of the task */
typedef unique_id task_iid;

/* The node id is an identifier for the node the task is
 * scheduled on */
typedef unique_id node_id;

enum scheduling_status {
  TASK_WAITING,
  TASK_SCHEDULED,
  TASK_RUNNING,
  TASK_DONE
}

typedef struct {
  /* Of type scheduling_status. */
  int status;
  /* Node the task has been scheduled on or is running on. */
  node_id node;
} task_status;

/* Callback for subscribing to the task log. */
typedef void (*task_log_callback)(task_iid task_iid, task_spec *task, task_status status);

/* Initially add a task to the task log. */
void task_log_add_task(task_iid task_iid, task_spec *task, task_status task_status);

/* Submit task to the global scheduler. */
void task_log_update_task(db_handle *db, task_iid task_iid, task_status status);

/* Register callback for a certain event. */
void task_log_register_callback(db_handle *db, task_log_callback callback, task_status status_filter);

#endif /* TASK_LOG_H */
