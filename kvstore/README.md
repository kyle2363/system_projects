# Concurrent and Sharded Key-Value Store

A C++20 project exploring synchronization within a process and routing across networked stores. It builds on course-provided networking, serialization, command interfaces, and tests.

## Design

The [concurrent store](kvstore/concurrent_kvstore.cpp) uses 60 hash buckets with a `shared_mutex` per bucket. Reads hold shared locks and writes hold exclusive locks. Multi-key operations collect distinct bucket indices in sorted order before acquiring locks, giving operations a consistent lock order and protecting the batch while it executes.

The [sharded client](client/shardkv_client.cpp) queries the shard controller, groups keys by server, and maps batch-read results back into the caller's key order. The [static controller](shardcontroller/static_shardcontroller.cpp) manages shard assignments, and the [server](server/server.cpp) handles requests and configuration changes.

These are educational implementations. Do not assume replication, crash recovery, or atomic transactions across servers. Passing in-process batch tests would not establish distributed transaction guarantees. The sharded `MultiPut` implementation also assumes equal-length key and value vectors; callers must validate this precondition to avoid out-of-range access.

## Build and test

Inside the Linux course container, from this directory:

```bash
make -C build
make -C build check
make -C build check concurrent_store
make -C build check distributed_store
```

Clang with C++20 and pthread support is required. The build provides client, server, and shard-controller executables. See [../VALIDATION.md](../VALIDATION.md) for observed results.

## Additional exercise

The deletion command explores replacing usernames in a sample social dataset. It has known coverage, substring-matching, and reversibility limitations and must not be treated as secure anonymization or a compliance feature. The sample database and output are course exercise fixtures.
