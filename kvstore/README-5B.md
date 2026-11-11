# Distributed Store Notes

See the [project overview](README.md), [sharded client](client/shardkv_client.cpp), and [static shard controller](shardcontroller/static_shardcontroller.cpp).

The client uses controller configuration to route keys and groups batch operations by destination server. Results from batch reads are reconstructed in request order. Cross-server atomicity and fault tolerance are not claimed.
