# Concurrent Store Notes

See the [project overview](README.md) and [concurrent implementation](kvstore/concurrent_kvstore.cpp).

The store uses per-bucket reader/writer locks and acquires locks in bucket order for multi-key operations. This allows independent buckets to proceed concurrently while maintaining a consistent lock order.

The associated deletion exercise uses case-insensitive substring replacement and retains a reverse identity mapping. It can over-replace text, miss nested content, and leave structural metadata unchanged. It is an exploration of tradeoffs, not an irreversible anonymization mechanism.
