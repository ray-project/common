#include "db.h"
#include "object_table.h"

#include "hiredis/hiredis.h"
#include "hiredis/async.h"

struct db_conn_impl {
  long long manager_id;
  /* Redis context for this global state store connection. */
  redisAsyncContext *context;
  /* Which events are we processing (read, write)? */
  int reading, writing;
  /* The event loop this global state store connection is part of. */
  event_loop *loop;
};

void object_table_fetch_addr_port(redisAsyncContext *c,
                                  void *r,
                                  void *privdata);

void object_table_lookup_callback(redisAsyncContext *c,
                                  void *r,
                                  void *privdata);
