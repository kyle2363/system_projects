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
    char readData[FILE_SIZE];
    char readData2[FILE_SIZE];

    memset(expected, 0, FILE_SIZE);

    // Random writes
    for (int i = 0; i < OPS; i++) {
        int pos = rand() % FILE_SIZE;
        char value = rand() % 256;

        io300_seek(f, pos);
        io300_writec(f, value);

        expected[pos] = value;
    }
    

    printf("readData2\n");

    for (int j = 0; j < 128; j++) {
        printf("%d ", readData2[j]); 
    }

    printf("\n expected\n");

    for (int j = 0; j < 128; j++) {
        printf("%d ", expected[j]); 
    }

    // Random reads
    for (int i = 0; i < OPS; i++) {
        int pos = rand() % FILE_SIZE;

        io300_seek(f, pos);
        char v = io300_readc(f);

        if (v != expected[pos]) {
            printf("\n");
            for (int j = pos-12; j < pos+12; j++) {
                //io300_seek(f, j);
                //char v2 = io300_readc(f);
                printf("in ex: %d   in file: %d   \n", expected[j], 0); 
            }
            printf("\n");
            printf("Mismatch at pos %d: expected %d got %d\n",
                   pos, expected[pos], v);
        }

        assert(v == expected[pos]);
    }

    // Full file verification
    io300_seek(f, 0);
    io300_read(f, readData, FILE_SIZE);

    for (int i = 0; i < FILE_SIZE; i++) {
        assert(readData[i] == expected[i]);
    }

    printf("Random test passed!\n");

    io300_close(f);
}