#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <cassert>
#include <cstring>

#include <unordered_set>
#include <unordered_map>
#include <limits.h>


struct allocation_info {
    size_t size;
    const char* file;
    long line;
};

static std::unordered_map<void*, allocation_info> allocation_sizes;

static std::unordered_set<void*> freed_ptrs;

static dmalloc_stats global_stats = {
    0,  // nactive
    0,  // active_size
    0,  // ntotal
    0,  // total_size
    0,  // nfail
    0,  // fail_size
    0,  // heap_min
    0   // heap_max
};

const uint64_t SECRET = 0xAAAABBBBCCCCDDDD;





/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and 
 *      return a pointer to it 
 * 
 * @arg size_t sz : the amount of memory requested 
 * @arg const char *file : a string containing the filename from which dmalloc was called 
 * @arg long line : the line number from which dmalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.

    if (sz > SIZE_MAX - 8) {
        global_stats.nfail += 1;
        global_stats.fail_size += sz;
        return nullptr;
    }

    void* ptr = base_malloc(sz + 8);

    if(!ptr){
        global_stats.nfail+=1;
        global_stats.fail_size += sz;
        return nullptr;
    }
    global_stats.ntotal += 1;
    global_stats.total_size += sz;

    global_stats.nactive += 1;
    global_stats.active_size += sz;

    uintptr_t addr = (uintptr_t) ptr;

    if (global_stats.heap_min == 0 || addr < global_stats.heap_min) {
        global_stats.heap_min = addr;
    }

    if (addr + sz + 8 > global_stats.heap_max) {  
        global_stats.heap_max = addr + sz + 8;
    }

    allocation_sizes[ptr] = { sz, file, line };

    if(freed_ptrs.find(ptr) != freed_ptrs.end()){
        freed_ptrs.erase(ptr);
    }

    char* secret_location = (char*)ptr + sz;
    for (int i = 0; i < 8; i++) {
        secret_location[i] = ((char*)&SECRET)[i];
    }

    return ptr;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    (void) file, (void) line;
    
    if (!ptr) {
        return;
    }

    if (freed_ptrs.find(ptr) != freed_ptrs.end()) {
        fprintf(stderr,
                "MEMORY BUG: %s:%ld: invalid free of pointer %p, double free\n",
                file, line, ptr);
        abort();
    }

    auto it_size = allocation_sizes.find(ptr);
    if (it_size == allocation_sizes.end()) {
        uintptr_t addr = (uintptr_t) ptr;
        
        if (addr < global_stats.heap_min || addr >= global_stats.heap_max) {
            fprintf(stderr,
                    "MEMORY BUG: %s:%ld: invalid free of pointer %p, not in heap\n",
                    file, line, ptr);
        } else {
            fprintf(stderr,
                    "MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated\n",
                    file, line, ptr);
        }

        std::unordered_map<void*, allocation_info>::const_iterator it_check;
        for (it_check = allocation_sizes.begin(); it_check != allocation_sizes.end(); ++it_check) {
            void* ptr_item = it_check->first;
            const allocation_info& info = it_check->second;
        
            uintptr_t base = (uintptr_t) ptr_item;
        
            if (addr >= base && addr < base + info.size) {
                long bytes_inside = (long)(addr - base);
                fprintf(stderr,
                    "%s:%ld: %p is %ld bytes inside a %zu byte region allocated here\n",
                    info.file, info.line, ptr, bytes_inside, info.size);
            }
        }
        abort();
    }

    size_t sz = it_size->second.size;

    char* secret_location = (char*)ptr + sz;
    bool match = true;
    for (int i = 0; i < 8; i++) {
        if (secret_location[i] != ((char*)&SECRET)[i]) {
            match = false;
            break;
        }
    }
    if (!match) {
        fprintf(stderr, "MEMORY BUG???: detected wild write during free of pointer %p", ptr);
        abort();
    }

    // Update statistics
    global_stats.nactive -= 1;
    global_stats.active_size -= sz;
    
    // Mark as freed
    freed_ptrs.insert(ptr);
    allocation_sizes.erase(it_size);

    // Actually free the memory
    base_free(ptr);
}

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb` 
 *      number of elements with wach element being `sz` bytes. The memory should be initialized 
 *      to zero  
 * 
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called 
 * @arg long line : the line number from which dcalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    if (nmemb != 0 && sz > SIZE_MAX / nmemb) {
        global_stats.nfail += 1;
        global_stats.fail_size += nmemb * sz;   
        return nullptr;
    }
    void* ptr = dmalloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_stats));
    // Your code here.
    *stats = global_stats;
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
    std::unordered_map<void*, allocation_info>::const_iterator it;
    for (it = allocation_sizes.begin(); it != allocation_sizes.end(); it++) {
        void* ptr = it->first;
        const allocation_info& info = it->second;

        printf("LEAK CHECK: %s:%ld: allocated object %p with size %zu\n",
                info.file,
                info.line,
                ptr,
                info.size);
    }
}
