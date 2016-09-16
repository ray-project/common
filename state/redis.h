#include "db.h"
#include "object_table.h"

#include "hiredis/hiredis.h"
#include "hiredis/async.h"

struct db_conn_impl {
  /* String that identifies this client type. */
  /* Ion: say more about what the client type is */
  char *client_type;
  /* Unique ID for this client within the type. */
  int64_t client_id;
  /* Redis context for this global state store connection. */
  redisAsyncContext *context;
  /* Which events are we processing (read, write)? */
  int reading, writing;
  /* The event loop this global state store connection is part of. */
  event_loop *loop;
};

/* Ion: say what the next functions are doing and what are the arguments, i.e., r and privdata */
void object_table_fetch_addr_port(redisAsyncContext *c,
                                  void *r,
                                  void *privdata);

void object_table_lookup_callback(redisAsyncContext *c,
                                  void *r,
                                  void *privdata);
