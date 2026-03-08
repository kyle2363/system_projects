/*
 * rtest004.c - simple io300_readc test
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

int main() {
    test_init();
    assert(CACHE_SIZE == 8);

    struct io300_file* f = create_file_from_string(TEST_FILE, "hello world");

    char contents[11] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a};
    char readData[11]; 

    // Write data to file
    io300_seek(f, 0);
    io300_write(f, contents, 11);
    io300_seek(f, 0);

    // Read bytes one at a time
    for (int i = 0; i < 11; i++) {
        readData[i] = io300_readc(f);
    }

    
    printf("Data: \n");
    for (int i = 0; i < 11; i++) {
        printf("%d ", contents[i]); 
    }

    printf("\n"); 

    for (int i = 0; i < 11; i++) {
        printf("%d ", readData[i]); 
    }
    printf("\n");

    for (int i = 0; i < 11; i++) {
        assert(readData[i] == contents[i]);
    }

    io300_close(f); 
}
