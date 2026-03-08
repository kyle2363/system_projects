Project 3: Caching I/O
======================

<!-- TODO: Fill this out. -->

## Design Overview

The design of my fileIO has the following notable details
-rather then caching just forward, the cache will align with block of size CACHE_SIZE
    -ie for the call readc at index 26, the cached data will be from index 24-31
    -reasoning in extra credit section
-0 filling when writing beyond EOF is also done with blocks of size Cache size
-separate OS and wrapper positioning to reduce lseek calls
-

## Collaborators 

none

## Extra Credit attempted

Help us improve performance!(2pt)
    Form submitted.

Changes that led to better performance
    - Blocking to allow cache to have data both ways most of the time
        -improve sequential speed in both directions
    -separating OS and wrapper pointer
        -less lseeks 
    -Always trying to write/read at cache size when possible for longer write/read calls
        -lower number of read/write calls needed
    -Always filling up a cache entirely for every read call
        -increases the chance that a cache hit can occur again
    -

Adaptive Caching (added but removed in favor of blocking)
    Blocking was favored over directional caching to solve the issue with what to do about the 
    start and end of files, allow access to data to the left and right of current position, being simpler to implement, and still maintaining the speed of forward/backward caching especially in sequential situations. 
    Note on last point, while directional caching may initially have a slight boost over blocks when starting in the middle of a block, once the boundary is met after a few sequential calls the speed is the same as the number of cache hits/misses are the same.

Diabolical byte_cat 
    The diabolical_byte_cat program performs frequent small seeks while reading bytes.
    The stdio implementation performs a fseek() for every seek operation, which results in many system calls and frequent cache invalidations.

    Our implementation separates the logical file position from the OS file position. io300_seek only updates the logical position and does not call lseek. The OS file pointer is moved only when a cache miss occurs and the program actually needs to fetch a new block.

    Because many seeks remain within the same cached block, our implementation avoids unnecessary lseek system calls and continues serving data directly from the cache. This significantly reduces system call overhead and results in better performance than stdio for the diabolical_byte_cat workload.

## Post-project form

Don't forget to fill out the post-project form!  See the handout for
the link.
