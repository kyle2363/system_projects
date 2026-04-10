#include "u-lib.hh"

void process_main() {
    void* x = (void*) 0x100000;
    
    
    sys_page_alloc(x);
    while(1){}
    sys_page_free((uintptr_t) x, PAGESIZE);

    // Should not crash if reused
    sys_page_alloc(x);
}