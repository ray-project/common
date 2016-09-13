#include "common.h"
#include "db.h"

typedef void (*lookup_callback)(void *);

/* Register a new object with the directory. */
void directory_link(db_conn *db, unique_id object_id);

void directory_unlink(db_conn *db, unique_id object_id);

void directory_lookup(db_conn *db,
                      unique_id object_id,
                      lookup_callback callback);
