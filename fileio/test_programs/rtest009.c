/*
 * rtest009.c - mini custom byte cat
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
    
    int size = 9;
    char contents[size];
    char contents2[size];
    char readData[size]; 
    for (int i = 0; i < size; i++) {
        contents[i] = i % 255;
        contents2[i] = 0;
    }

    struct io300_file* f = create_file_from_string(TEST_FILE, contents);
    io300_seek(f, 0);
    io300_write(f, contents, size);
    io300_seek(f, 0);
    struct io300_file* f2 = create_file_from_string(TEST_FILE_2, contents2);
    io300_seek(f2, 0);

    for (int k = 0; k < size; k++){ 
        int ch = io300_readc(f);
        io300_writec(f2, ch);
        readData[k] = ch;
    }
    
    for (int i = 0; i < size; i++) {
        assert(readData[i] == contents[i]);
    }
    io300_close(f); 
}
