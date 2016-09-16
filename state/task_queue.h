#include "db.h"

/* Callback for subscribing to the task queue. The only argument this
 * callback gets is the task_id of the. */
typedef void (*task_queue_callback)(unique_id *);

/* Add task to task queue. */
void task_queue_add_task(db_conn *db, unique_id task_id);

/* Subscribe to task queue. */
void task_queue_subscribe(db_conn *db, task_queue_callback callback);
