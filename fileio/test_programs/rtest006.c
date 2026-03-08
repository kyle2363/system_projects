/*
 * rtest006.c - read one byte at a time for mutiple file sizes backwards
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

    for (int k = 0; k < 20; k++){

        int size = k;
        char contents[size];// = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a};

        for (int i = 0; i < size; i++) {
            contents[i] = i;
        }

        char readData[size]; 

        io300_seek(f, 0);
        io300_write(f, contents, size);
        io300_seek(f, 0);
    
        for (int i = size-1; i >= 0; i--) {
            io300_seek(f, i);
            readData[i] = io300_readc(f);
        }

        

        for (int i = 0; i < size; i++) {
            printf("%d ", contents[i]); 
        }

        printf("\n"); 

        for (int i = 0; i < size; i++) {
            printf("%d ", readData[i]); 
        }
        printf("\n");
        printf("\n");

        for (int i = 0; i < size; i++) {
            assert(readData[i] == contents[i]);
        }
    }

    io300_close(f); 
}
