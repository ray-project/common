#include "sockets.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>

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
