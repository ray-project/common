#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

/* Helper functions for socket communication. */
int bind_ipc_sock(const char* socket_pathname);
int connect_ipc_sock(const char* socket_pathname);
void send_ipc_sock(int socket_fd, char* message);
char* recv_ipc_sock(int socket_fd);

#endif
