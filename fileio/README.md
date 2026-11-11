# Cached File I/O

A C wrapper around POSIX file descriptors with byte and block read/write operations and a small block-aligned cache.

## Design

The [implementation](impl/student.c) tracks the logical position separately from the OS file offset. A seek can update the logical position without immediately issuing `lseek`; cache misses move the underlying descriptor as needed. Block alignment lets nearby forward and backward accesses share cached data. Dirty data is flushed when necessary, and writes beyond EOF handle the gap before the new data.

The default cache size in the current source is eight bytes. This makes cache-boundary behavior easy to exercise but is not a production tuning choice. Performance depends on workload, cache size, synchronization, and filesystem behavior. The saved `top_performance.txt` is historical output with incomplete environment context, not a current benchmark or evidence of a general speedup. `impl/student_(old).c` is an earlier implementation retained for reference; the Makefile uses `impl/student.c` by default.

## Build and test

Inside the Linux course container, from this directory:

```bash
make
make check
make check-regression
```

The project also provides `naive` and `stdio` comparison implementations. Performance testing is separate from correctness testing; use `make perf` only after building the desired configuration. See [../VALIDATION.md](../VALIDATION.md) for current verification results.
