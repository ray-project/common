#include "db.h"
#include "object_table.h"

#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "uthash.h"

typedef struct {
  /* Unique ID for this service. */
  int service_id;
  /* IP address and port of this service. */
  const char* addr;
  /* Handle for the uthash table. */
  UT_hash_handle hh;
} service_cache_entry;

struct db_conn_impl {
  /* String that identifies this client type. */
  char *client_type;
  /* Unique ID for this client within the type. */
  int64_t client_id;
  /* Redis context for this global state store connection. */
  redisAsyncContext *context;
  /* Which events are we processing (read, write)? */
  int reading, writing;
  /* The event loop this global state store connection is part of. */
  event_loop *loop;
  /* Cache for the IP addresses of services. */
  service_cache_entry *service_cache;
  /* Redis context for synchronou connections. Should only be used very rarely. */
  redisContext *sync_context;
};

typedef struct {
  /* The callback that will be called. */
  lookup_callback callback;
  /* Object ID that is looked up. */
  object_id object_id;
} lookup_callback_data;

void object_table_get_entry(redisAsyncContext *c,
                            void *r,
                            void *privdata);

void object_table_lookup_callback(redisAsyncContext *c,
                                  void *r,
                                  void *privdata);
