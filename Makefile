CC = gcc
CFLAGS = -g -Wall --std=c99 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L
BUILD = build

CFLAGS += -Wmissing-prototypes
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-declarations

$(BUILD)/db_tests: test/db_tests.c thirdparty/greatest.h event_loop.c state/redis.c common.c
	$(CC) -o $@ test/db_tests.c event_loop.c state/redis.c common.c thirdparty/hiredis/libhiredis.a $(CFLAGS) -I. -Ithirdparty

clean:
	rm -r $(BUILD)/*

redis:
	cd thirdparty ; bash ./build-redis.sh

test: redis FORCE
	./thirdparty/redis-3.2.3/src/redis-server & sleep 1s ; ./build/db_tests

FORCE:
