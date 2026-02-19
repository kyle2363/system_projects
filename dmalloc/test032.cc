#include "dmalloc.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Advanced error message for freeing data inside another heap block.

int main() {
    void* ptr = malloc(2001);
    *((int*)ptr) = 1000;
    *((int*)ptr + 4) = 2000;
    free((char*) ptr + 128);
    print_statistics();
}

//! MEMORY BUG: test???.cc:11: invalid free of pointer ???, not allocated
//! test???.cc:8: ??? is 128 bytes inside a 2001 byte region allocated here
//! ???
