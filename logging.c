#include "logging.h"

#include <hiredis/hiredis.h>

#include "state/redis.h"
#include "io.h"

static const char *log_levels[5] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static char *log_fmt =
    "HMSET log:%s:%s:%s log_level %s entity_type %s "
    "entity_id %s related_entity_ids %s event_type %s message %s timestamp %s";

struct ray_logger_impl {
  /* String that identifies this client type. */
  char *client_type;
  /* Suppress all log messages below this level. */
  int log_level;
  /* Whether or not we have a direct connection to Redis. */
  int is_local;
  /* Either a db_conn or a socket to a process with a db_conn,
   * depending on the is_local flag. */
  void *conn;
};

ray_logger *init_ray_logger(const char *client_type,
                            int log_level,
                            int is_local,
                            void *conn) {
  ray_logger *logger = malloc(sizeof(ray_logger));
  logger->client_type = client_type;
  logger->log_level = log_level;
  logger->is_local = is_local;
  logger->conn = conn;
  return logger;
}

void ray_log(ray_logger *logger,
             int log_level,
             const char *entity_type,
             const char *entity_id,
             const char *event_type,
             const char *message,
             const char *related_entity_ids) {
  if (log_level < logger->log_level) {
    return;
  }
  if (log_level < RAY_DEBUG || log_level > RAY_FATAL) {
    return;
  }
  struct timeval tv;
  char timestamp[32];
  gettimeofday(&tv, NULL);
  snprintf(timestamp, sizeof(timestamp), "%ld.%ld", tv.tv_sec, tv.tv_usec);
  char origin_id[32];
  if (logger->is_local) {
    db_conn *db = (db_conn *) logger->conn;
    sprintf(origin_id, "%ld:%s", db->client_id, "");
    redisAsyncCommand(db->context, NULL, NULL, log_fmt, logger->client_type,
                      origin_id, timestamp, log_levels[log_level], entity_type,
                      entity_id, related_entity_ids, event_type, message,
                      timestamp);
  } else {
    /* If we don't own a Redis connection, we leave our client
     * ID to be filled in by someone else. */
    sprintf(origin_id, "%s:%s", "%ld", "%ld");
    int *socket_fd = (int *) logger->conn;
    write_formatted_string(*socket_fd, log_fmt, logger->client_type, origin_id,
                           timestamp, log_levels[log_level], entity_type,
                           entity_id, related_entity_ids, event_type, message,
                           timestamp);
  }
}
