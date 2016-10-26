#include "timer.h"

typedef struct {
  /** Number of retries left. */
  int retry_count;
  /** Timeout, in milliseconds. */
  uint64_t timeout;
  /** The callback that will be called when a new attempt is made to complete the operation */
  timer_retry_callback retry_callback;
  /** The callback that will be called when the redis command times out. */
  timer_give_up_callback give_up_callback;
  /** User context. */
  void *user_context;
  /** Handle to timer. */
  int64_t timer_id;
} timer_data;

/**
 * Function to handle the timeout event.
 * @param loop: event loop.
 * @param timer_id: timer identifier.
 * @param data: pointer to the timer data
 * @return timeout to reset the timer if we need to try again, or
  *        EVENT_LOOP_TIMER_DONE if retry_count == 0.
 */
int64_t timer_timeout_handler(event_loop *loop,
                              int64_t timer_id,
                              void *OWNER timer_data_arg) {
  CHECK(loop);
  CHECK(timer_data_arg);
  timer_data *data = (timer_data *) timer_data_arg;

  CHECK(data->retry_count >= 0)

  if (data->retry_count == 0) {
    /* We didn't get a response from the database after exhausting all retries;
     * give the user the possibility to cleanup the state, and remove the timer. */
    data->give_up_callback(data->user_context);
    free(data);
    return EVENT_LOOP_TIMER_DONE;
  }
  /* Decrement retry count and try again. */
  data->retry_count--;
  data->retry_callback(data->user_context);
  return data->timeout;
}

void timer_async_retry(event_loop *loop,
                       int retry_count,
                       uint64_t timeout,
                       timer_retry_callback retry_callback,
                       timer_give_up_callback give_up_callback,
                       void *user_context) {
  timer_data *data = malloc(sizeof(timer_data));
  CHECKM(data != NULL, "Memory allocation error!");
  data->retry_count = retry_count;
  data->timeout = timeout;
  data->retry_callback = retry_callback;
  data->give_up_callback = give_up_callback;
  data->user_context = user_context;
  data->timer_id = event_loop_add_timer(loop,
                                        timeout,
                                        timer_timeout_handler,
                                        data);
  data->retry_callback(user_context);
}
