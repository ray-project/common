#include "greatest.h"

#include <unistd.h>
#include <sys/socket.h>

#include "event_loop.h"
#include "timer.h"

SUITE(timer_tests);

event_loop *loop;
int simple_gave_up = 0;
int simple_num_tries = 0;

void simple_retry_callback(void *user_data) {
  simple_num_tries += 1;
	CHECK(user_data == (void*) 42);
}

void simple_give_up_callback(void *user_data) {
  simple_gave_up = 1;
	CHECK(user_data == (void*) 42);
	event_loop_stop(loop);
}

TEST simple_timer_test(void) {
  loop = event_loop_create();
  void *context = (void *) 42;
  timer_async_retry(loop, 5, 100, simple_retry_callback, simple_give_up_callback, context);
  event_loop_run(loop);
  event_loop_destroy(loop);
  CHECK(simple_gave_up);
  CHECK(simple_num_tries == 6);
  PASS();
}

struct user_data {
  /* Is the file descriptor registered in the event loop? */
  int is_registered;
  /* The file descriptor we are listening to. */
  int fd;
};

int gave_up = 0;
int num_retries = 0;

/* In the test below, this will be called after 150 ms */
void read_data(event_loop *loop, int fd, void *context, int events) {
  CHECK(num_retries == 2);
  event_loop_stop(loop);
}

void retry_callback(void *user_data) {
  struct user_data *data = user_data;
  num_retries += 1;
  if (data->is_registered) {
    event_loop_remove_file(loop, data->fd);
  }
  event_loop_add_file(loop, data->fd, EVENT_LOOP_READ, read_data, NULL);
}

/* This shall not be called. */
void give_up_callback(void *user_data) {
  gave_up = 1;
	event_loop_stop(loop);
}

TEST timer_test(void) {
  struct user_data context;
  context.is_registered = 0;
  loop = event_loop_create();
  /* Two processes, the child writes a message to the parent. */
  int fd[2];
  static const int parentsocket = 0;
  static const int childsocket = 1;
  int payload = 69;
  socketpair(PF_LOCAL, SOCK_STREAM, 0, fd);
  pid_t pid = fork();
  if (pid == 0) {
    close(fd[parentsocket]);
    /* Sleep for 150 ms. */
    usleep(150000);
    write(fd[childsocket], &payload, sizeof(payload));
    exit(0);
  } else {
    close(fd[childsocket]);
    context.fd = fd[parentsocket];
    /* Retry receiving the message. */
    timer_async_retry(loop, 5, 100, retry_callback, give_up_callback, &context);
    event_loop_run(loop);
  }
  CHECK(gave_up == 0);
  CHECK(num_retries == 2);
  PASS();
}

SUITE(timer_tests) {
  RUN_TEST(simple_timer_test);
  RUN_TEST(timer_test);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(timer_tests);
  GREATEST_MAIN_END();
}
