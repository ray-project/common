#include "common.h"
#include "db.h"

/* The callback that is called when the result of a lookup
 * in the object table comes back. The callback should not free
 * the ip addresses of the plasma managers. */
typedef void (*lookup_callback)(object_id obj_id,
                                struct sockaddr **managers,
                                int64_t num_nodes);

/* Register a new object with the directory. */
/* TODO(pcm): Retry, print for each attempt. */
void object_table_add(db_conn *db, object_id obj_id);

/* Remove object from the directory. */
void object_table_remove(db_conn *db,
                         object_id obj_id,
                         struct sockaddr *manager);

/* Look up entry from the directory */
void object_table_lookup(db_conn *db,
                         object_id obj_id,
                         lookup_callback callback);
