#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "common.h"
#include "event_loop.h"

/** Type of the callback that will be called when a new
 *  attempt is made to to complete the operation. */
typedef void (*timer_retry_callback)(void *user_context);

/** Type of the callback that will be called when the operation
 *  cannot be completed within the given number of retries. */
typedef void (*timer_give_up_callback)(void *user_context);


/** Try an asynchronous operation multiple times with a timeout.
 *
 * @param loop Event loop.
 * @param retry_count Number of times the operation is retried.
 * @param timeout Timeout for each operation in milliseconds.
 * @param retry_callback Every retry this callback is going to be called.
 * @param give_up_callback All retries are exhausted and we got no response.
 * @param user_context User context that will be passed to the callbacks.
 * @return Void.
 */
void timer_async_retry(event_loop *loop,
                       int retry_count,
                       uint64_t timeout,
                       timer_retry_callback retry_callback,
                       timer_give_up_callback give_up_callback,
                       void *user_context);

#endif /* TIMER_H */
