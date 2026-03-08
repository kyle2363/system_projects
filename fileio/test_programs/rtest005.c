
/*
 * rtest005.c - simple io300_read tests
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

#define FILE_SIZE 20

int main() {
    test_init();
    assert(CACHE_SIZE == 8);
    
    struct io300_file* f = create_file_from_string(TEST_FILE, "initial data");


    char contents[FILE_SIZE];
    for (int i = 0; i < FILE_SIZE; i++) 
        contents[i] = i;

    char readData[FILE_SIZE];

    //write data to file
    io300_seek(f, 0);
    io300_write(f, contents, FILE_SIZE);
    io300_seek(f, 0);

    //read file
    ssize_t bytesRead = io300_read(f, readData, FILE_SIZE);
    assert(bytesRead == FILE_SIZE);

    for (int i = 0; i < FILE_SIZE; i++) {
        assert(readData[i] == contents[i]);
    }

    //test read beyond EOF
    bytesRead = io300_read(f, readData, FILE_SIZE);
    
    assert(bytesRead == 0);

    //test read mid file
    io300_seek(f, 5);
    bytesRead = io300_read(f, readData, 10);
    assert(bytesRead == 10); 
    for (int i = 0; i < bytesRead; i++) {
        assert(readData[i] == contents[5 + i]);
    }

    io300_close(f);
}