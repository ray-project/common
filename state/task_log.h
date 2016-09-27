#ifndef TASK_LOG_H
#define TASK_LOG_H

#include "db.h"
#include "task.h"

/* Callback for subscribing to the task log. */
typedef void (*task_log_callback)(scheduled_task* task, void *userdata);

/* Initially add a task to the task log. This adds the scheduled task. */
void task_log_add_task(db_handle *db, task_iid task_iid, scheduled_task* task);

/* Update task in the task log. This will append the new status to the
 * task_iid entry of the task log. */
void task_log_update_task(db_handle *db, task_iid task_iid, int32_t state, node_id node);

/* Register callback for a certain event. The node specifies the node whose
 * events we want to listen to. If you want to listen to all events for this node,
 * use state_filter = TASK_WAITING | TASK_SCHEDULED | TASK_RUNNING | TASK_DONE. */
void task_log_register_callback(db_handle *db, task_log_callback callback, node_id node, int32_t state_filter, void *userdata);

#endif /* TASK_LOG_H */
