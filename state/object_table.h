#include "common.h"
#include "db.h"

typedef void (*lookup_callback)(void *);

/* Register a new object with the directory. */
/* Ion: don't we need to specify the ID of the node the object is stored on? */
/* Ion: we should return errors, no? */
void object_table_add(db_conn *db, unique_id object_id);

/* Remove object from the directory */
/* Ion: same as above; the node the object is stored on? errors? */
void object_table_remove(db_conn *db, unique_id object_id);

/* Look up entry from the directory */
/* Ion: what are the arguments of the callback ? */
void object_table_lookup(db_conn *db,
                         unique_id object_id,
                         lookup_callback callback);
