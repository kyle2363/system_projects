Project 4 - WeensyOS
====================

<!-- TODO: Fill this out. -->

## Design Overview

No notable design choices

## Collaborators

None

## Extra Credit Attempted

mostly works:
Sleep System Call
Easy. Create a system call that puts the calling process to sleep until a given amount of time (measured in ticks) elapses. Note that ticks will count timer interrupts!

Note, requires manually incrementing tick as when key is pressed tick no longer increments (set on line 46 kernel.cc to 1)


Attempted after lots of debugging:

Kill System Call
Easy. Create a system call that lets one process kill another process (or itself)!

Program will refer to a page at the memory limit? should work if not for that

Free System Call
Moderately difficult. Create sys_page_free, a system call that unallocates pages. It should behave like munmap.

opposite issue of above, refers to address 0 when it should not be able to due to no permissions

