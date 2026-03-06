#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

#define FILE_SIZE 20

int main() {
    printf("hi");
    test_init();
    assert(CACHE_SIZE == 8);
    printf("hi");
    // Create file with initial content
    struct io300_file* f = create_file_from_string(TEST_FILE, "initial data");

    // Fill test data
    char contents[FILE_SIZE];
    printf("hi");
    for (int i = 0; i < FILE_SIZE; i++) contents[i] = i;

    char readData[FILE_SIZE];

    // Write test data from start
    printf("hi");
    io300_seek(f, 0);
    printf("hi");
    io300_write(f, contents, FILE_SIZE);
    io300_seek(f, 0);

    // Read everything in one bulk read
    printf("hi");
    ssize_t bytesRead = io300_read(f, readData, FILE_SIZE);
    printf("hi");
    assert(bytesRead == FILE_SIZE);

    // Verify contents match
    for (int i = 0; i < FILE_SIZE; i++) {
        //printf("%d   %d \n",readData[i], contents[i]);
        assert(readData[i] == contents[i]);
    }

    // Test reading past EOF returns 0
    bytesRead = io300_read(f, readData, FILE_SIZE);
    
    assert(bytesRead == 0);

    // Test reading part of the file
    io300_seek(f, 5);
    bytesRead = io300_read(f, readData, 10);
    assert(bytesRead == 10); 
    for (int i = 0; i < bytesRead; i++) {
        assert(readData[i] == contents[5 + i]);
    }

    io300_close(f);
    printf("All io300_read tests passed!\n");
}