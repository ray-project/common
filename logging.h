#include <hiredis/hiredis.h>

#include "state/redis.h"

#define RAY_VERBOSE -1
#define RAY_DEBUG 0
#define RAY_INFO 1
#define RAY_WARNING 2
#define RAY_ERROR 3
#define RAY_FATAL 4

// Entity types.
#define RAY_FUNCTION "FUNCTION"
#define RAY_OBJECT "OBJECT"
#define RAY_TASK "TASK"

static const char *log_levels[5] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static inline void redis_log(redis_conn *redis,
                             int log_level,
                             const char *entity_type,
                             const char *entity_id,
                             const char *event_type,
                             const char *message,
                             const char *related_entity_ids) {
  if (log_level < RAY_DEBUG || log_level > RAY_FATAL) {
    return;
  }
  struct timeval tv;
  char timestamp[32];
  gettimeofday(&tv, NULL);
  snprintf(timestamp, sizeof(timestamp), "%ld.%ld", tv.tv_sec, tv.tv_usec);
  char origin_id[32];
  if (redis->is_local) {
    db_conn *db = (db_conn *) redis->conn;
    sprintf(origin_id, "%ld:%s", db->client_id, "");
  } else {
    /* If we don't own a Redis connection, we leave our client
     * ID to be filled in by someone else. */
    sprintf(origin_id, "%s:%s", "%ld", "%ld");
  }
  send_redis_command(redis,
                     "HMSET log:%s:%s:%s log_level %s entity_type %s entity_id "
                     "%s related_entity_ids %s event_type %s message %s "
                     "timestamp %s",
                     redis->client_type, origin_id, timestamp,
                     log_levels[log_level], entity_type, entity_id,
                     related_entity_ids, event_type, message, timestamp);
}
