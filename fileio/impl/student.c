#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../io300.h"

/*
 *  student.c - Your implementation
 */

#ifndef CACHE_SIZE
#define CACHE_SIZE 8
#endif

#if (CACHE_SIZE < 4)
#error "internal cache size should not be below 4."
#error "if you changed this during testing, that is fine."
#error "when handing in, make sure it is reset to the provided value"
#error "if this is not done, the autograder will not run"
#endif

// Prototypes for helper functions defined later
int fetch(struct io300_file* const f);
int flush(struct io300_file* const f);

/*
 * This macro enables/disables the dbg() function. Use it to silence your
 * debugging info.
 * Use the dbg() function instead of printf debugging if you don't want to
 * hunt down 30 printfs when you want to hand in
 */
#define DEBUG_PRINT 0
#define DEBUG_STATISTICS 1

struct io300_file {
    /* read,write,seek all take a file descriptor as a parameter */
    int fd;
    /* this will serve as our cache */
    char* cache;

    // TODO: Your properties go here!

    long current_pos;

    long os_pos;

    long cached_file_index;

    long cache_offset;

    long cached_data_size;

    long cache_modified;
    
    long file_size;



    /* Used for debugging, keep track of which io300_file is which */
    char* description;
    /* To tell if we are getting the performance we are expecting */
    struct io300_statistics {
        int read_calls;
        int write_calls;
        int seeks;
    } stats;
};

/*
 *  Assert the properties that you would like your file to have at all times.
 *  Call this function frequently (like at the beginning of each function) to
 *   catch logical errors early on in development.
 */
static void check_invariants(struct io300_file* f) {
    assert(f != NULL);
    assert(f->cache != NULL);
    assert(f->fd >= 0);

    // TODO: Add more invariants
}

/*
 *  Wrapper around printf that provides information about the
 *  given file. You can turn off messages printed by this function
 *  by changing the DEBUG_PRINT macro above.
 */
static void dbg(struct io300_file* f, char* fmt, ...) {
    (void)f;
    (void)fmt;
#if (DEBUG_PRINT == 1)
    static char buff[300];
    size_t const size = sizeof(buff);
    int n = snprintf(buff, size,
                     // TODO: Add the fields you want to print when debugging
                     "{desc:%s, } -- ", f->description);
    int const bytes_left = size - n;
    va_list args;
    va_start(args, fmt);
    vsnprintf(&buff[n], bytes_left, fmt, args);
    va_end(args);
    printf("%s", buff);
#endif
}

struct io300_file* io300_open(const char* const path, int mode,
                              char* description) {
    if (path == NULL) {
        fprintf(stderr, "error: null file path\n");
        return NULL;
    }

    // Set flags for how file is created [DO NOT CHANGE THIS]
    int flags = O_CREAT | O_SYNC;
    switch (mode) {
        case MODE_READ:
            flags |= O_RDONLY;
            break;
        case MODE_WRITE:
            flags |= O_RDWR | O_TRUNC;
            break;
        case (MODE_READ | MODE_WRITE):
            flags |= O_RDWR;
            break;
        default:
            fprintf(stderr, "error: invalid file mode %02x\n", mode);
            return NULL;
    }

    int const fd =
        open(path, flags, S_IRUSR | S_IWUSR);  // DO NOT CHANGE THIS LINE
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: `%s`: %s\n", path,
                strerror(errno));
        return NULL;
    }

    struct io300_file* const ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        fprintf(stderr, "error: could not allocate io300_file\n");
        close(fd);
        return NULL;
    }

    ret->fd = fd;
    ret->cache = malloc(CACHE_SIZE);
    if (ret->cache == NULL) {
        fprintf(stderr, "error: could not allocate file cache\n");
        close(ret->fd);
        free(ret);
        return NULL;
    }
    // Initialize debugging info
    ret->description = description;
    ret->stats.read_calls = 0;
    ret->stats.write_calls = 0;
    ret->stats.seeks = 0;

    // TODO: Initialize your metadata!

    ret->cache_offset = -1; // start of data in cache

    ret->cache_modified = 0;

    ret->cached_data_size = 0;

    ret->cached_file_index = 0; // index in ORIGINAL file

    ret->current_pos = 0;

    ret->os_pos = 0;

    ret-> file_size = io300_filesize(ret);



    check_invariants(ret);
    dbg(ret, "Just finished initializing file from path: %s\n", path);
    return ret;
}

int io300_seek(struct io300_file* const f, off_t const pos) {
    check_invariants(f);
    f->stats.seeks++;
    f->current_pos = pos;
    // TODO: Implement!
    if (f->os_pos != pos) {
        off_t r = lseek(f->fd, pos, SEEK_SET);
        if (r >= 0)
            f->os_pos = r;
        return r;
    }
    //printf("%ld and %ld \n",f->os_pos, pos);
    return pos;//lseek(f->fd, pos, SEEK_SET);//pos;
}

int io300_close(struct io300_file* const f) {
    check_invariants(f);

#if (DEBUG_STATISTICS == 1)
    printf("stats: {desc: %s, read_calls: %d, write_calls: %d, seeks: %d}\n",
           f->description, f->stats.read_calls, f->stats.write_calls,
           f->stats.seeks);
#endif

    // TODO: Implement!
    flush(f);
    close(f->fd);
    free(f->cache);
    free(f);
    return 0;
}

/*
 * io300_filesize: Get the size of a file This function is part of the
 * io300 library: some tests will use it to get the file's current
 * size.  You may also use it as a helper for your implementation, or
 * modify it to better fit your design, but this is not required.
 *
 * WARNING:  this function makes a system call (fstat)!
 */
off_t io300_filesize(struct io300_file* const f) {
    check_invariants(f);
    struct stat s;
    int const r = fstat(f->fd, &s);  // system call!
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}

int io300_readc(struct io300_file* const f) {
    check_invariants(f);
    //printf("1  pos = %ld   cache_pos= %ld      ", f->current_pos, f->cached_file_index);

    if (f->current_pos >= f->file_size)
        return -1;

    if(f->cached_file_index <= f->current_pos && 
        f->cached_file_index + f->cached_data_size > f->current_pos){
        f->current_pos++;
        printf("CACHE HIT (READC)\n");
        //printf("Char: %d  ",f->cache[(f->current_pos-1) - f->cached_file_index]);
        return (unsigned char)f->cache[(f->current_pos-1) - f->cached_file_index];
    }
    if(f->cache_modified == 1){
        flush(f);
        f->cache_modified = 0;
    }

    // TODO: Implement!
    //unsigned char c;
    /*if(f->os_pos != f->current_pos){
        io300_seek(f, f->current_pos);
    }
    

    if(f->current_pos < f->cached_file_index && f->current_pos > CACHE_SIZE+1){
        io300_seek(f, f->current_pos - CACHE_SIZE + 1);
    }*/
    long block_start = ((f->current_pos / CACHE_SIZE)) * CACHE_SIZE;
    f->cached_file_index = block_start;
    long old_cur_pos = f->current_pos;

    if (f->os_pos != block_start) {
        io300_seek(f, block_start);
    }


    io300_seek(f,block_start);
    f->stats.read_calls++;
    printf("CACHE MISS (READC)\n");
    long b_read = read(f->fd, f->cache, CACHE_SIZE);

    if (b_read >= 1) {
        f->cache_offset = 0;
        f->cache_modified = 0;
        f->cached_data_size = b_read; //ASSUMED might need to change

        
        f->os_pos = block_start + b_read;
        //f-> os_pos = f->current_pos + b_read;
        f->current_pos =old_cur_pos + 1;
        return (unsigned char) f->cache[f->current_pos - f->cached_file_index - 1];
    } else {
        return -1;
    }
}

int io300_writec(struct io300_file* f, int ch) {
    check_invariants(f);
    
    unsigned char const c = (unsigned char)ch;
    const int BLOCK_SIZE = CACHE_SIZE;
    if (f->current_pos > f->file_size) {

        if (f->cache_modified == 1) {
            flush(f);
            f->cache_modified = 0;
        }
        long old_pos = f->current_pos;
        long gap = f->current_pos - f->file_size;
        io300_seek(f, f->file_size);
        f->os_pos = f->file_size;
        
        
        char zeros[BLOCK_SIZE];
        memset(zeros, 0, BLOCK_SIZE);
        
        while (gap > 0) {
            size_t chunk = (gap > BLOCK_SIZE) ? BLOCK_SIZE : gap;
            ssize_t written = write(f->fd, zeros, chunk);
            if (written > 0) {
                f->file_size += written;
                f->os_pos += written;
            }
            gap -= written;
        }
        
        long block_start = (f->current_pos / BLOCK_SIZE) * BLOCK_SIZE;
        
        if (f->os_pos != block_start) {
            io300_seek(f, block_start);
            f->os_pos = block_start;
        }
        f->stats.read_calls++;
        printf("CACHE MISS (BEYOND)(WRITEC)\n");
        ssize_t bytes_read = read(f->fd, f->cache, BLOCK_SIZE);
        
        f->cached_file_index = block_start;
        f->cached_data_size = (bytes_read > 0) ? bytes_read : 0;
        f->cache_modified = 0;
        f->os_pos = block_start + f->cached_data_size;
        io300_seek(f, old_pos);
    }

    if (f->cached_data_size > 0 && f->cached_file_index <= f->current_pos &&
        f->current_pos < f->cached_file_index + BLOCK_SIZE) {
        
        printf("CACHE HIT 1 (WRITE)\n"); 
        
        int offset = f->current_pos - f->cached_file_index;
        f->cache[offset] = c;
        f->cache_modified = 1;

        if (offset >= f->cached_data_size) {
            f->cached_data_size = offset + 1;
        }

        f->current_pos++;
        
        if (f->current_pos > f->file_size) {
            f->file_size = f->current_pos;
        }

        return c;
    }

    //long old_cache_pos = f->cached_file_index;

    if (f->cache_modified == 1) {
        flush(f);
        f->cache_modified = 0;
    }
    long old_pos = f->current_pos;

    long block_start = (f->current_pos / CACHE_SIZE) * CACHE_SIZE;
    f->cached_file_index = block_start;
    f->cached_data_size = 0;

    if (block_start < f->file_size) {
        if (f->os_pos != block_start) {
            io300_seek(f, block_start);
            f->os_pos = block_start;
        }
        f->stats.read_calls++;
        printf("CACHE MISS 1 (WRITE)\n"); 
        ssize_t b_read = read(f->fd, f->cache, CACHE_SIZE);
        if (b_read > 0) {
            f->cached_data_size = b_read;
            f->os_pos = block_start + b_read;
        }
    }
    
    
    int offset = old_pos - f->cached_file_index;
    f->cache[offset] = c;
    f->cache_modified = 1;

    if (offset + 1 > f->cached_data_size) {
        f->cached_data_size = offset + 1;
    }

    f->current_pos = old_pos + 1;
    if (old_pos > f->file_size) {
        f->file_size = f->current_pos;
    }
    return c;
}

ssize_t io300_read(struct io300_file* const f, char* const buff,
    size_t const sz) {
        check_invariants(f);

    size_t total_read = 0;

    /*for(int i=0;i<6;i++){
        printf("%d ", f->cache[i]);
    }*/
    while (total_read < sz) {

        // If current position is not inside cache → fetch new block
        if (!(f->cached_file_index <= f->current_pos &&
        f->current_pos < f->cached_file_index + f->cached_data_size)) {

            if (f->cache_modified == 1) {
                flush(f);
                f->cache_modified = 0;
            }

            

            if (f->os_pos != f->current_pos) {
                io300_seek(f, f->current_pos);
                f->os_pos = f->current_pos;
            }

            // Read new block into cache
            f->stats.read_calls++;
            printf("READ (READ)\n");
            ssize_t bytes = read(f->fd, f->cache, CACHE_SIZE);
            if (bytes <= 0) {
                break;  
            }

            f->cached_file_index = f->current_pos;
            f->cached_data_size = bytes;
            f->os_pos += bytes;
        }

        size_t cache_offset = f->current_pos - f->cached_file_index;
        size_t bytes_available = f->cached_data_size - cache_offset;
        size_t bytes_to_copy = sz - total_read;

        if (bytes_to_copy > bytes_available) {
            bytes_to_copy = bytes_available;
        }

        memcpy(buff + total_read, f->cache + cache_offset, bytes_to_copy);

        f->current_pos += bytes_to_copy;
        total_read += bytes_to_copy;
    }
    return total_read;
}

ssize_t io300_write(struct io300_file* const f, const char* buff,
                    size_t const sz) {
    check_invariants(f);
    //f->stats.write_calls++; //might need

    size_t total_written = 0;

    if (f->current_pos > f->file_size) {

        if (f->cache_modified == 1) {
            flush(f);
            f->cache_modified = 0;
        }
        long old_pos = f->current_pos;
        long gap = f->current_pos - f->file_size;
        io300_seek(f, f->file_size);
        f->os_pos = f->file_size;
        
        char zeros[CACHE_SIZE];
        memset(zeros, 0, CACHE_SIZE);
        
        while (gap > 0) {
            size_t chunk = (gap > CACHE_SIZE) ? CACHE_SIZE : gap;
            ssize_t written = write(f->fd, zeros, chunk);
            if (written > 0) {
                f->file_size += written;
                f->os_pos += written;
            }
            gap -= written;
        }
        
        long block_start = (f->current_pos / CACHE_SIZE) * CACHE_SIZE;
        
        if (f->os_pos != block_start) {
            io300_seek(f, block_start);
            f->os_pos = block_start;
        }
        f->stats.read_calls++;
        printf("CACHE MISS (BEYOND)(WRITEC)\n");
        ssize_t bytes_read = read(f->fd, f->cache, CACHE_SIZE);
        
        f->cached_file_index = block_start;
        f->cached_data_size = (bytes_read > 0) ? bytes_read : 0;
        f->cache_modified = 0;
        f->os_pos = block_start + f->cached_data_size;
        io300_seek(f, old_pos);
    }

    while (total_written < sz) {
        // Check if current_pos is within the current cache block.
        int in_cache = (f->cached_data_size > 0) &&
                        (f->cached_file_index <= f->current_pos) &&
                        (f->current_pos < f->cached_file_index + CACHE_SIZE);

        long old_pos = f->current_pos;

        if (!in_cache) {
            if (f->cache_modified) {
                flush(f);
                f->cache_modified = 0;
            }

            //long block_start = f->current_pos;
            long block_start = (f->current_pos / CACHE_SIZE) * CACHE_SIZE;

            if (f->os_pos != block_start) {
                io300_seek(f, block_start);
                f->os_pos = block_start;
            }
            f->stats.read_calls++;
            printf("READ (WRITE)\n");
            ssize_t bytes = read(f->fd, f->cache, CACHE_SIZE);
            f->cached_file_index = block_start;
            f->cached_data_size  = (bytes > 0) ? bytes : 0;
            f->os_pos            = block_start + f->cached_data_size;
            f->cache_modified    = 0;
        }
        f->current_pos = old_pos;

        long offset         = f->current_pos - f->cached_file_index;
        size_t space_left   = (size_t)(CACHE_SIZE - offset);
        size_t bytes_to_write = sz - total_written;
        if (bytes_to_write > space_left) {
            bytes_to_write = space_left;
        }

        memcpy(f->cache + offset, buff + total_written, bytes_to_write);
        f->cache_modified = 1;

        if (offset + (long)bytes_to_write > f->cached_data_size) {
            f->cached_data_size = offset + (long)bytes_to_write;
        }

        f->current_pos += (long)bytes_to_write;
        total_written  += bytes_to_write;

        if (f->current_pos > f->file_size) {
            f->file_size = f->current_pos;
        }
    }

    return (ssize_t)total_written;
}

int fetch(struct io300_file* const f) {
    check_invariants(f);
    /*
    * This (optional) helper should contain the logic for fetching
    * data from the file into the cache.  Think about how you can use
    * this helper to factor out some of the common logic in your read,
    * write, and seek functions!
    *
    * Feel free to add arguments if needed, but if you do, be sure to
    * modify the function prototype (at the top of the file) to match!
    */

    // TODO: Implement this

    return 0;
}

int flush(struct io300_file* const f) {
    check_invariants(f);
    /*
     * This helper should flush data in the cache to the file, if any
     * changes have been made.Think about how you can use this helper
     * to factor out some of the common logic in your read, write, and
     * seek functions!
     *
     * Feel free to add arguments if needed, but if you do, be sure to
     * modify the function prototype (at the top of the file) to match!
     */

    // TODO: Implement this

    if (f->cache_modified == 0 || f->cached_data_size == 0) {
        return 0; 
    }
    //printf("FLUSH\n"); 
    
    long saved_pos = f->current_pos;
    
    io300_seek(f, f->cached_file_index);
    f->current_pos = f->cached_file_index;  
    f->stats.write_calls++;
    ssize_t written = write(f->fd, f->cache, f->cached_data_size);
    
    if (written > 0) {
        f->os_pos = f->cached_file_index + written;  
        if (f->os_pos > f->file_size) {
            f->file_size = f->os_pos; 
        }
    }
    
    f->cache_modified = 0;
    f->cached_file_index = -1;
    f->cached_data_size = 0;
    
    f->current_pos = saved_pos;  
    return 0;
}
