# Debugging Allocator

A C++ allocation wrapper that tracks allocation statistics and reports common memory errors. Built on the course's base allocator and diagnostic test suite.

## Implementation

The [allocator](dmalloc.cc) stores pointer-to-allocation metadata in an `unordered_map`, including allocation size and source location. Keeping metadata outside user allocations separates bookkeeping from the user-writable region. A set tracks freed pointers, and an eight-byte trailing sentinel helps detect writes past allocation boundaries.

The implementation includes invalid-free and double-free diagnostics, allocation statistics, leak reporting, and reallocation support. These are debugging mechanisms rather than comprehensive memory safety guarantees; a trailing sentinel does not detect every out-of-bounds access.

## Build and test

Inside the Linux course container, from this directory:

```bash
make
make check
make check-1
```

The existing suite contains 40 test sources. `make SAN=1` enables the sanitizer configuration supplied by the project; individual test rules deliberately exclude some sanitizers. Actual verification results are in [../VALIDATION.md](../VALIDATION.md).
