/*
 * rtest_stride.c - sequential write stress test every n bytes
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

#define TOTAL_SIZE 500   
#define STRIDE 10        

int main() {
    test_init();
    assert(CACHE_SIZE == 8);

    srand(time(NULL));

    struct io300_file* f = create_file_from_string(TEST_FILE, "");

    char expected[TOTAL_SIZE];
    char writeBuf[STRIDE];
    char readBuf[STRIDE];

    memset(expected, 0, TOTAL_SIZE);

    // Sequential writes every STRIDE bytes
    for (int pos = 0; pos < TOTAL_SIZE; pos += STRIDE) {
        int size = (pos + STRIDE <= TOTAL_SIZE) ? STRIDE : (TOTAL_SIZE - pos);

        for (int j = 0; j < size; j++) {
            writeBuf[j] = rand() % 256;
        }

        io300_seek(f, pos);
        ssize_t written = io300_write(f, writeBuf, size);
        assert(written == size);

        memcpy(expected + pos, writeBuf, size);
    }

    for (int pos = 0; pos < TOTAL_SIZE; pos += STRIDE) {
        int size = (pos + STRIDE <= TOTAL_SIZE) ? STRIDE : (TOTAL_SIZE - pos);

        memset(readBuf, 0, STRIDE);

        io300_seek(f, pos);
        ssize_t bytes_read = io300_read(f, readBuf, size);
        assert(bytes_read == size);

        for (int j = 0; j < size; j++) {
            if (readBuf[j] != expected[pos + j]) {
                printf("Read mismatch at pos %d offset %d: expected %d got %d\n",
                       pos, j,
                       (unsigned char)expected[pos + j],
                       (unsigned char)readBuf[j]);
                assert(0);
            }
        }
    }

    char fullRead[TOTAL_SIZE];
    memset(fullRead, 0, TOTAL_SIZE);
    io300_seek(f, 0);
    ssize_t total_read = io300_read(f, fullRead, TOTAL_SIZE);
    assert(total_read == TOTAL_SIZE);

    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (fullRead[i] != expected[i]) {
            printf("Final check mismatch at pos %d: expected %d got %d\n",
                   i, (unsigned char)expected[i], (unsigned char)fullRead[i]);
            assert(0);
        }
    }

    io300_close(f);
    return 0;
}