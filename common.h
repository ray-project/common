#ifndef COMMON_H
#define COMMON_H

#include <errno.h>

#ifdef NDEBUG
#define LOG_DEBUG(M, ...)
#else
#define LOG_DEBUG(M, ...) \
  fprintf(stderr, "[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define LOG_ERR(M, ...)                                                     \
  fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
          errno == 0 ? "None" : strerror(errno), ##__VA_ARGS__)

#define LOG_INFO(M, ...) \
  fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CHECK(COND)                        \
  do {                                     \
    if (!(COND)) {                         \
      LOG_ERR("Check failure: %s", #COND); \
      exit(-1);                            \
    }                                      \
  } while (0);

/* Return and error codes. */

/* If a fatal error happens, the process exits with an exit code. */

#define GIVE_UP(STATUS_CODE) \
  do { \
    LOG_ERR("Giving up with status code %d", STATUS_CODE); \
    exit(STATUS_CODE); \
  } while (0);

/* Unique IDs */

#define UNIQUE_ID_SIZE 20

typedef struct { unsigned char id[UNIQUE_ID_SIZE]; } unique_id;

/* Convert a 20 byte sha1 hash to a hexdecimal string. This function assumes
 * that buffer points to an already allocated char array of size 2 *
 * UNIQUE_ID_SIZE + 1 */
char *sha1_to_hex(const unsigned char *sha1, char *buffer);

#endif
