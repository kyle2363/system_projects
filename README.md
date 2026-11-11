# Systems Programming Portfolio

A collection of C and C++ systems projects completed for Brown University's CSCI 0300/1310, Fundamentals of Computer Systems (Spring 2026).

These projects build on course-provided frameworks and tests. My work focuses on the implementations and design decisions described below; some of the surrounding infrastructure was provided as stencil code.

## Projects

| Project | Focus | Start here |
| --- | --- | --- |
| [Debugging allocator](dmalloc/) | Allocation metadata, memory diagnostics, leak reporting | [Implementation](dmalloc/dmalloc.cc) |
| [Cached file I/O](fileio/) | Block caching, logical file positions, system call overhead | [Implementation](fileio/impl/student.c) |
| [Concurrent and sharded key-value store](kvstore/) | Reader/writer locks, atomic batch operations, network routing | [Concurrent store](kvstore/kvstore/concurrent_kvstore.cpp) |
| [WeensyOS](weensyos/) | Virtual memory, process isolation, fork and exit | [Kernel](weensyos/kernel.cc) |
| [Terminal Snake](snake/) | C pointers, linked lists, board parsing, terminal rendering | [Game logic](snake/src/game.c) |

For a quick code review, start with the concurrent store's ordered bucket locking, the I/O cache's separate logical and OS positions, and the allocator's external metadata table.

## Build and test

The supported baseline is the original Linux course container. Snake and KVStore explicitly check for that environment; this repository does not yet provide a standalone replacement container. Native Windows builds are not supported by these Makefiles.

From the repository root inside the course container:

```bash
make -C snake && make -C snake check
make -C dmalloc && make -C dmalloc check
make -C fileio && make -C fileio check
make -C kvstore/build && make -C kvstore/build check
make -C weensyos
make -C weensyos run-console
```

Tools include GCC, Clang with C++20 support, GNU Make, Python 3, Perl, ncurses development libraries, and QEMU for WeensyOS. See each project README for details and limitations. Build results are recorded in [VALIDATION.md](VALIDATION.md).

## Scope and attribution

This is educational software, with known limitations; it is not intended for production use. Course starter code, test harnesses, and third-party notices are retained. WeensyOS has its own [copyright notice](weensyos/COPYRIGHT) and [authors](weensyos/AUTHORS.md). No repository-wide license is asserted over third-party material.

The `timemachine/` directory contains a software-preservation reflection rather than a programming project and is outside the featured portfolio.

See [PUBLICATION.md](PUBLICATION.md) for remaining checks before public release.
