#ifndef CACHE_H
#define CACHE_H

/* A cached version of the global state database (object table, task log, task
 * table). */
typedef db_cache_impl db_cache;

/* These are the error codes the methods can return. */
#define CACHE_NOT_FOUND -1
#define CACHE_OK 0

/* Return an array of plasma managers that have the object (the last parameter
 * is an out parameter, which is assumed to be an array we append to). */
int cached_object_table_get(db_cache *db,
                            object_id object_id,
                            UT_array *managers);

/* Register a new object with the cache. */
void cached_object_table_add(db_cache *db, object_id object_id, node_id node);

/* Initially add a task to the task log cache. */
void cached_task_log_add_task(db_cache *db,
                              task_iid task_iid,
                              task_spec *task,
                              task_status task_status);

/* Update task in the task log cache. This will append the new status to the
 * task log cache. */
void cached_task_log_update_task(db_cache *db,
                                 task_iid task_iid,
                                 task_status status);

/* Get task from the task log cache. The last two parameters are out parameters,
 * which are both assumed to be allocated. */
int cached_task_log_get_task(db_cache *db,
                             task_iid task_iid,
                             task_spec *task,
                             task_status *status);

/* Add task to the task table cache. */
void chached_task_table_add_task(db_cache *db, task_spec *task);

/* Get specific task from the task table cache. */
int cached_task_table_get_task(db_cache *db, task_id task_id, task_spec *out);

#endif /* CACHE_H */
