/*
 * rtest105 - Check that readc can handle reading the byte 0xff (which is -1 as a signed char)
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../io300.h"
#include "unit_tests.h"

char* expected_string = "\xff\xff\xff";

int main() {
    test_init();
    struct io300_file* f = create_file_from_string(TEST_FILE, expected_string);

    assert(CACHE_SIZE == 8);

    int failed_read = -1;
    int correct_byte = 255;
    assert(io300_readc(f) != failed_read);
    assert(io300_readc(f) == correct_byte);

    io300_close(f);
}
