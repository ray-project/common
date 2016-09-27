/* Redis implementation of the global state store */

#include <assert.h>

#include <stdlib.h>
#include "hiredis/adapters/ae.h"
#include "utstring.h"

#include "common.h"
#include "db.h"
#include "object_table.h"
#include "task_log.h"
#include "event_loop.h"
#include "redis.h"
#include "io.h"

#define LOG_REDIS_ERR(context, M, ...)                                        \
  fprintf(stderr, "[ERROR] (%s:%d: message: %s) " M "\n", __FILE__, __LINE__, \
          context->errstr, ##__VA_ARGS__)

#define CHECK_REDIS_CONNECT(CONTEXT_TYPE, context, M, ...) \
  do {                                                     \
    CONTEXT_TYPE *_context = (context);                    \
    if (!_context) {                                       \
      LOG_ERR("could not allocate redis context");         \
      exit(-1);                                            \
    }                                                      \
    if (_context->err) {                                   \
      LOG_REDIS_ERR(_context, M, ##__VA_ARGS__);           \
      exit(-1);                                            \
    }                                                      \
  } while (0);

db_handle *db_connect(const char *address,
                      int port,
                      const char *client_type,
                      const char *client_addr,
                      int client_port) {
  db_handle *db = malloc(sizeof(db_handle));
  /* Sync connection for initial handshake */
  redisReply *reply;
  long long num_clients;
  redisContext *context = redisConnect(address, port);
  CHECK_REDIS_CONNECT(redisContext, context, "could not connect to redis %s:%d",
                      address, port);
  /* Add new client using optimistic locking. */
  while (1) {
    reply = redisCommand(context, "WATCH %s", client_type);
    freeReplyObject(reply);
    reply = redisCommand(context, "HLEN %s", client_type);
    num_clients = reply->integer;
    freeReplyObject(reply);
    reply = redisCommand(context, "MULTI");
    freeReplyObject(reply);
    reply = redisCommand(context, "HSET %s %lld %s:%d", client_type,
                         num_clients, client_addr, client_port);
    freeReplyObject(reply);
    reply = redisCommand(context, "EXEC");
    if (reply) {
      freeReplyObject(reply);
      break;
    }
    freeReplyObject(reply);
  }

  db->client_type = strdup(client_type);
  db->client_id = num_clients;
  db->service_cache = NULL;
  db->sync_context = context;

  /* Establish async connection */
  db->context = redisAsyncConnect(address, port);
  CHECK_REDIS_CONNECT(redisAsyncContext, db->context,
                      "could not connect to redis %s:%d", address, port);
  db->context->data = (void *) db;
  /* Establish async connection for subscription */
  db->sub_context = redisAsyncConnect(address, port);
  CHECK_REDIS_CONNECT(redisAsyncContext, db->sub_context,
                      "could not connect to redis %s:%d (sub)", address, port);
  db->sub_context->data = (void *) db;
  return db;
}

void db_disconnect(db_handle *db) {
  redisFree(db->sync_context);
  redisAsyncFree(db->context);
  redisAsyncFree(db->sub_context);
  service_cache_entry *e, *tmp;
  HASH_ITER(hh, db->service_cache, e, tmp) {
    free(e->addr);
    HASH_DEL(db->service_cache, e);
    free(e);
  }
  free(db->client_type);
  free(db);
}

void db_attach(db_handle *db, event_loop *loop) {
  redisAeAttach(loop, db->context);
  redisAeAttach(loop, db->sub_context);
}

void object_table_add(db_handle *db, unique_id object_id) {
  static char hex_object_id[2 * UNIQUE_ID_SIZE + 1];
  sha1_to_hex(&object_id.id[0], &hex_object_id[0]);
  redisAsyncCommand(db->context, NULL, NULL, "SADD obj:%s %d",
                    &hex_object_id[0], db->client_id);
  if (db->context->err) {
    LOG_REDIS_ERR(db->context, "could not add object_table entry");
  }
}

void object_table_get_entry(redisAsyncContext *c, void *r, void *privdata) {
  db_handle *db = c->data;
  lookup_callback_data *cb_data = privdata;
  redisReply *reply = r;
  if (reply == NULL)
    return;
  int *result = malloc(reply->elements * sizeof(int));
  int64_t manager_count = reply->elements;
  if (reply->type == REDIS_REPLY_ARRAY) {
    for (int j = 0; j < reply->elements; j++) {
      CHECK(reply->element[j]->type == REDIS_REPLY_STRING);
      result[j] = atoi(reply->element[j]->str);
      service_cache_entry *entry;
      HASH_FIND_INT(db->service_cache, &result[j], entry);
      if (!entry) {
        redisReply *reply = redisCommand(db->sync_context, "HGET %s %lld",
                                         db->client_type, result[j]);
        CHECK(reply->type == REDIS_REPLY_STRING);
        entry = malloc(sizeof(service_cache_entry));
        entry->service_id = result[j];
        entry->addr = strdup(reply->str);
        HASH_ADD_INT(db->service_cache, service_id, entry);
        freeReplyObject(reply);
      }
    }
  } else {
    LOG_ERR("expected integer or string, received type %d", reply->type);
    exit(-1);
  }
  const char **manager_vector = malloc(manager_count * sizeof(char *));
  for (int j = 0; j < manager_count; ++j) {
    service_cache_entry *entry;
    HASH_FIND_INT(db->service_cache, &result[j], entry);
    manager_vector[j] = entry->addr;
  }
  cb_data->callback(cb_data->object_id, manager_count, manager_vector);
  free(privdata);
  free(result);
}

void object_table_lookup(db_handle *db,
                         object_id object_id,
                         lookup_callback callback) {
  static char hex_object_id[2 * UNIQUE_ID_SIZE + 1];
  sha1_to_hex(&object_id.id[0], &hex_object_id[0]);
  lookup_callback_data *cb_data = malloc(sizeof(lookup_callback_data));
  cb_data->callback = callback;
  cb_data->object_id = object_id;
  redisAsyncCommand(db->context, object_table_get_entry, cb_data,
                    "SMEMBERS obj:%s", &hex_object_id[0]);
  if (db->context->err) {
    LOG_REDIS_ERR(db->context, "error in object_table lookup");
  }
}

void task_log_add_task(db_handle *db, task_iid task_iid, task_spec *task, task_status task_status) {
  static char hex[UNIQUE_HEX_SIZE];
  UT_string *command;
  utstring_new(command);
  sha1_to_hex(&task_iid.id[0], &hex[0]);
  utstring_printf(command, "HMSET tasklog:%s ", &hex[0]);
  print_task(task, command);
  sha1_to_hex(&task_status.node.id[0], &hex[0]);
  utstring_printf(command, " status:0:status %d", task_status.status);
  utstring_printf(command, " status:0:node %s", &hex[0]);
  redisAsyncCommand(db->context, NULL, NULL, utstring_body(command));
  if (db->context->err) {
    LOG_REDIS_ERR(db->context, "error writing task in task_log_add_task");
  }
  UT_string *pubsub;
  utstring_new(pubsub);
  print_task(task, pubsub);
  redisAsyncCommand(db->context, NULL, NULL, "PUBLISH task_log:%s %s", &hex[0], utstring_body(pubsub), task_status.status);
  if (db->context->err) {
    LOG_REDIS_ERR(db->context, "error publishing task in task_log_add_task");
  }
  utstring_free(pubsub);
  utstring_free(command);
}

void task_log_redis_callback(redisAsyncContext *c, void *reply, void *privdata) {
  redisReply *r = reply;
  if (reply == NULL) return;
  CHECK(r->type == REDIS_REPLY_ARRAY);
  /* First entry is message type, second entry is topic, third entry is payload. */
  CHECK(r->elements > 2);
  /* If this condition is true, we got the initial message that acknowledged the subscription. */
  if (r->element[2]->str == NULL) {
    return;
  }
  /* Otherwise, parse the task and call the callback. */
  CHECK(privdata);
  task_log_callback_data *callback_data = privdata;
  task_spec *task = parse_task(r->element[2]->str, r->element[2]->len);
  CHECK(r->element[1]->type == REDIS_REPLY_STRING);
  node_id node;
  static char node_hex[UNIQUE_HEX_SIZE];
  sscanf(r->element[1]->str, "task_log:%s ", &node_hex[0]);
  hex_to_sha1(&node_hex[0], &node.id[0]);
  /* TODO(pcm): Change placeholder by the actual status. */
  task_status status = {.status = 333, .node = node};
  callback_data->callback(NIL_ID, task, status, callback_data->userdata);
  free_task_spec(task);
}

void task_log_register_callback(db_handle *db, task_log_callback callback, task_status status_filter, void *userdata) {
  static char hex[UNIQUE_HEX_SIZE];
  sha1_to_hex(&status_filter.node.id[0], &hex[0]);
  task_log_callback_data *callback_data = malloc(sizeof(task_log_callback_data));
  callback_data->callback = callback;
  callback_data->userdata = userdata;
  redisAsyncCommand(db->sub_context, task_log_redis_callback, callback_data, "SUBSCRIBE task_log:%s", &hex[0]);
  if (db->sub_context->err) {
    LOG_REDIS_ERR(db->sub_context, "error in task_log_register_callback");
  }
}
