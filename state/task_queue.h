#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "db.h"
#include "task.h"

typedef unique_id task_id;
typedef unique_id task_iid;
typedef unique_id node_id;

/* Callback for subscribing to the task queue. The only argument this
 * callback gets is the task_id of the. */
typedef void (*task_queue_callback)(task_iid *task_iid, task_spec* task);

/* Submit task to the global scheduler. */
void task_queue_submit_task(db_conn *db, task_iid task_iid, task_spec* task);

/* Submit task to a local scheduler based on the decision made by the global scheduler. */
void task_queue_schedule_task(db_conn *db, task_iid task_iid, node_id node);

/* Subscribe to task queue. */
void task_queue_register_callback(db_conn *db, task_queue_callback callback);

#endif
