/*
 * rtest_random.c - random read/write stress test
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

#define FILE_SIZE 128
#define OPS 500

int main() {
    test_init();
    assert(CACHE_SIZE == 8);

    srand(time(NULL));

    struct io300_file* f = create_file_from_string(TEST_FILE, "");

    char expected[FILE_SIZE];
    char writeBuf[FILE_SIZE];
    char readBuf[FILE_SIZE];

    memset(expected, 0, FILE_SIZE);

    // Random block writes
    for (int i = 0; i < OPS; i++) {
        int pos = rand() % FILE_SIZE;
        int max_size = FILE_SIZE - pos;
        int size = (rand() % max_size) + 1;  // 1..FILE_SIZE-pos

        // Fill write buffer with random bytes
        for (int j = 0; j < size; j++) {
            writeBuf[j] = rand() % 256;
        }

        io300_seek(f, pos);
        ssize_t written = io300_write(f, writeBuf, size);

        assert(written == size);

        // Mirror into expected
        memcpy(expected + pos, writeBuf, size);
    }

    // Random block reads and verify against expected
    for (int i = 0; i < OPS; i++) {
        int pos = rand() % FILE_SIZE;
        int max_size = FILE_SIZE - pos;
        int size = (rand() % max_size) + 1;

        memset(readBuf, 0, FILE_SIZE);

        io300_seek(f, pos);
        ssize_t bytes_read = io300_read(f, readBuf, size);

        assert(bytes_read == size);

        for (int j = 0; j < size; j++) {
            if (readBuf[j] != expected[pos + j]) {
                printf("Block read mismatch at pos %d offset %d: "
                       "expected %d got %d\n",
                       pos, j,
                       (unsigned char)expected[pos + j],
                       (unsigned char)readBuf[j]);
                assert(0);
            }
        }
    }

    // Full file verification using io300_read
    memset(readBuf, 0, FILE_SIZE);
    io300_seek(f, 0);
    ssize_t total = io300_read(f, readBuf, FILE_SIZE);
    assert(total == FILE_SIZE);

    for (int i = 0; i < FILE_SIZE; i++) {
        if (readBuf[i] != expected[i]) {
            printf("Final verification mismatch at pos %d: "
                   "expected %d got %d\n",
                   i,
                   (unsigned char)expected[i],
                   (unsigned char)readBuf[i]);
            assert(0);
        }
    }

    printf("Random block read/write test passed!\n");

    io300_close(f);
    return 0;
}