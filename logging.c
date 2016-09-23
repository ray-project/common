#include "logging.h"

#include <hiredis/hiredis.h>
#include <utstring.h>

#include "state/redis.h"
#include "io.h"

static const char *log_levels[5] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char *log_fmt =
    "HMSET log:%s:%s:%s log_level %s event_type %s message %s timestamp %s";

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
             const char *event_type,
             const char *message) {
  if (log_level < logger->log_level) {
    return;
  }
  if (log_level < RAY_DEBUG || log_level > RAY_FATAL) {
    return;
  }
  struct timeval tv;
  UT_string *timestamp;
  utstring_new(timestamp);
  gettimeofday(&tv, NULL);
  utstring_printf(timestamp, "%ld.%ld", tv.tv_sec, tv.tv_usec);

  UT_string *origin_id;
  utstring_new(origin_id);
  if (logger->is_local) {
    db_conn *db = (db_conn *) logger->conn;
    utstring_printf(origin_id, "%ld:%s", db->client_id, "");
    redisAsyncCommand(db->context, NULL, NULL, log_fmt,
                      utstring_body(timestamp), logger->client_type,
                      utstring_body(origin_id), log_levels[log_level],
                      event_type, message, utstring_body(timestamp));
  } else {
    /* If we don't own a Redis connection, we leave our client
     * ID to be filled in by someone else. */
    utstring_printf(origin_id, "%s:%s", "%ld", "%ld");
    int *socket_fd = (int *) logger->conn;
    write_formatted_string(*socket_fd, log_fmt, utstring_body(timestamp),
                           logger->client_type, utstring_body(origin_id),
                           log_levels[log_level], event_type, message,
                           utstring_body(timestamp));
  }
  utstring_free(origin_id);
  utstring_free(timestamp);
}
