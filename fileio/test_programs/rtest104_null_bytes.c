/*
 * rtest104 - Read some bytes from a file containing exclusively null bytes
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

char* expected_string = "\x00\x00\x00";

int main() {
    test_init();
    char buf[9];
    struct io300_file* f =
        create_file_from_bytes(TEST_FILE, expected_string, 3);

    assert(CACHE_SIZE == 8);

    assert(io300_readc(f) == '\x00');
    assert(io300_read(f, buf, 2) == 2);

    io300_close(f);
}
