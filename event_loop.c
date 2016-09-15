#include "event_loop.h"

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>

UT_icd item_icd = {sizeof(event_loop_item), NULL, NULL, NULL};
UT_icd poll_icd = {sizeof(struct pollfd), NULL, NULL, NULL};

/* Initializes the event loop.
 * This function needs to be called before any other event loop function. */
void event_loop_init(event_loop *loop) {
  utarray_new(loop->items, &item_icd);
  utarray_new(loop->waiting, &poll_icd);
}

/* Add a new file descriptor fd to the event loop.
 * This function sets a user defined type and id for the file descriptor
 * which can be queried using event_loop_type and event_loop_id. The parameter
 * events is the same as in http://linux.die.net/man/2/poll.
 * Returns the index of the item in the event loop. */
int64_t event_loop_attach(event_loop *loop,
                          int type,
                          void *data,
                          int fd,
                          int events) {
  assert(utarray_len(loop->items) == utarray_len(loop->waiting));
  int64_t index = utarray_len(loop->items);
  event_loop_item item = {.type = type, .data = data};
  utarray_push_back(loop->items, &item);
  struct pollfd waiting = {.fd = fd, .events = events};
  utarray_push_back(loop->waiting, &waiting);
  return index;
}

/* Detach a file descriptor from the event loop.
 * This invalidates all other indices into the event loop items, but leaves
 * the ids of the event loop items valid. */
void event_loop_detach(event_loop *loop, int64_t index, int shall_close) {
  struct pollfd *waiting_item =
      (struct pollfd *) utarray_eltptr(loop->waiting, index);
  struct pollfd *waiting_back = (struct pollfd *) utarray_back(loop->waiting);
  if (shall_close) {
    close(waiting_item->fd);
  }
  *waiting_item = *waiting_back;
  utarray_pop_back(loop->waiting);

  event_loop_item *items_item =
      (event_loop_item *) utarray_eltptr(loop->items, index);
  event_loop_item *items_back = (event_loop_item *) utarray_back(loop->items);
  *items_item = *items_back;
  utarray_pop_back(loop->items);
}

/* Poll the file descriptors associated to this event loop.
 * See http://linux.die.net/man/2/poll */
int event_loop_poll(event_loop *loop) {
  return poll((struct pollfd *) utarray_front(loop->waiting),
              utarray_len(loop->waiting), -1);
}

/* Get the total number of file descriptors participating in the event loop. */
int64_t event_loop_size(event_loop *loop) {
  return utarray_len(loop->waiting);
}

/* Get the pollfd structure associated to a file descriptor participating in the
 * event loop. */
struct pollfd *event_loop_get(event_loop *loop, int64_t index) {
  return (struct pollfd *) utarray_eltptr(loop->waiting, index);
}

/* Set the data connection information for participant in the event loop. */
void event_loop_set_data(event_loop *loop, int64_t index, void *data) {
  event_loop_item *item =
      (event_loop_item *) utarray_eltptr(loop->items, index);
  item->data = data;
}

/* Get the data connection information for participant in the event loop. */
void *event_loop_get_data(event_loop *loop, int64_t index) {
  event_loop_item *item =
      (event_loop_item *) utarray_eltptr(loop->items, index);
  return item->data;
}

/* Free the space associated to the event loop.
 * Does not free the event_loop datastructure itself. */
void event_loop_free(event_loop *loop) {
  utarray_free(loop->items);
  utarray_free(loop->waiting);
}

/* Binds to a Unix domain datagram socket at the given
 * pathname. Removes any existing file at the pathname. Returns
 * a file descriptor for the socket, or -1 if an error
 * occurred. */
int bind_ipc_sock(const char* socket_pathname) {
  struct sockaddr_un socket_address;
  int socket_fd;

  socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (socket_fd < 0) {
    fprintf(stderr, "socket() failed\n");
    return -1;
  }

  unlink(socket_pathname);
  memset(&socket_address, 0, sizeof(struct sockaddr_un));
  socket_address.sun_family = AF_UNIX;
  strncpy(socket_address.sun_path, socket_pathname, strlen(socket_pathname) + 1);

  if (bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(struct sockaddr_un)) != 0) {
    fprintf(stderr, "Bind failed\n");
    return -1;
  }

  return socket_fd;
}


/* Connects to a Unix domain datagram socket at the given
 * pathname. Returns a file descriptor for the socket, or -1 if
 * an error occurred. */
int connect_ipc_sock(const char* socket_pathname) {
  struct sockaddr_un socket_address;
  int socket_fd;

  socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (socket_fd < 0) {
    fprintf(stderr, "socket() failed\n");
    return -1;
  }

  memset(&socket_address, 0, sizeof(struct sockaddr_un));
  socket_address.sun_family = AF_UNIX;
  strncpy(socket_address.sun_path, socket_pathname, strlen(socket_pathname) + 1);

  if (connect(socket_fd, (struct sockaddr *) &socket_address, sizeof(struct sockaddr_un)) != 0) {
    fprintf(stderr, "Connection to socket failed\n");
    return -1;
  }

  return socket_fd;
}

/* Sends a message on the given socket file descriptor. */
void send_ipc_sock(int socket_fd, char* message) {
  int length = strlen(message);
  int nbytes;
  nbytes = send(socket_fd, (char *) &length, sizeof(length), 0);
  if (nbytes == -1) {
    fprintf(stderr, "Error sending to socket.\n");
    return;
  }
  nbytes = send(socket_fd, (char *) message, length * sizeof(char), 0);
  if (nbytes == -1) {
    fprintf(stderr, "Error sending to socket.\n");
    return;
  }
  close(socket_fd);
}

/* Receives a message on the given socket file descriptor. */
char* recv_ipc_sock(int socket_fd) {
  int length;
  int nbytes;
  nbytes = recv(socket_fd, &length, sizeof(length), 0);
  if (nbytes == -1) {
    fprintf(stderr, "Error receiving from socket.\n");
    return NULL;
  }
  char *message = malloc((length + 1) * sizeof(char));
  nbytes = recv(socket_fd, message, length * sizeof(char), 0);
  if (nbytes == -1) {
    fprintf(stderr, "Error receiving from socket.\n");
    return NULL;
  }
  message[length] = '\0';
  return message;
}
