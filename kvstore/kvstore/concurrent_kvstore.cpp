#include "concurrent_kvstore.hpp"

#include <mutex>
#include <optional>
#include <set>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  size_t b = store.bucket(req->key);
  std::shared_lock lock(store.locks[b]); 
  std::optional<DbItem> item = store.getIfExists(b, req->key);
  if (item.has_value()) {
    res->value = item->value;
    return true;
  }
  return false;
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  size_t b = store.bucket(req->key);
  std::unique_lock lock(store.locks[b]);
  store.insertItem(b, req->key, req->value);
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  size_t b = store.bucket(req->key);
  std::unique_lock lock(store.locks[b]);
  std::optional<DbItem> item = store.getIfExists(b, req->key);
  if (item.has_value()) {
    store.insertItem(b, req->key, item->value + req->value);
    return true;
  }else{
    store.insertItem(b, req->key, req->value);
    return true;
  }  
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  size_t b = store.bucket(req->key);
  std::unique_lock lock(store.locks[b]);
  std::optional<DbItem> item = store.getIfExists(b, req->key);
  if(item.has_value()){
    res->value = item->value;
    store.removeItem(b, req->key);
    return true;
  }
  return false;
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  std::vector<std::string> keys = req->keys;
  std::vector<std::string> values; 

  std::set<size_t> unique_buckets;
  for (const auto& key : req->keys) {
      unique_buckets.insert(store.bucket(key));
  }
  std::vector<std::shared_lock<std::shared_mutex>> held_locks;
  for (size_t b : unique_buckets) {
      held_locks.emplace_back(store.locks[b]);
  }

  for(int i=0; i < (int)keys.size(); i++){
    size_t b = store.bucket(keys.at(i));
    std::optional<DbItem> item = store.getIfExists(b, keys.at(i));
    if (item.has_value()) {
      values.push_back(item->value);
    }
  }
  if(keys.size() != values.size()){
    return false;
  }
  res->values = values;
  return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  std::vector<std::string> keys = req->keys;
  std::vector<std::string> values = req->values;
  if(keys.size() != values.size()){
    return false;
  }

  std::set<size_t> unique_buckets;
  for (const auto& key : req->keys) {
      unique_buckets.insert(store.bucket(key));
  }
  std::vector<std::unique_lock<std::shared_mutex>> held_locks;
  for (size_t b : unique_buckets) {
      held_locks.emplace_back(store.locks[b]);
  }

  for(int i=0; i < (int)keys.size(); i++){
    size_t b = store.bucket(keys.at(i));
    store.insertItem(b, keys.at(i), values.at(i));
  }
  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  // TODO (Part A, Step 3 and Step 4): Implement!
  std::vector<std::string> keys;
  std::vector<std::shared_lock<std::shared_mutex>> held_locks;
  for (size_t i = 0; i < DbMap::BUCKET_COUNT; ++i) {
      held_locks.emplace_back(store.locks[i]);
  }
  for (size_t i = 0; i < DbMap::BUCKET_COUNT; ++i) {
    for (const auto& item : store.buckets[i]) {
      keys.push_back(item.key);
    }
  }
  return keys;
}
