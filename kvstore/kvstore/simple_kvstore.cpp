#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  auto it = data.find(req->key);
  if (it == data.end()) return false;
  res->value = it->second;
  return true;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  data[req->key] = req->value;
  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  data[req->key] = data[req->key].append(req->value);
  return true;
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  auto it = data.find(req->key);
  if (it == data.end()) return false; 
  
  res->value = it->second; 
  data.erase(it);
  return true;
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  std::vector<std::string> keys = req->keys;
  std::vector<std::string> values;
  for(int i=0; i < (int)keys.size(); i++){
    auto it = data.find(req->keys.at(i));
    if (it == data.end()) return false;
    values.push_back(it->second);
  }
  if(keys.size() != values.size()){
    return false;
  }
  res->values = values;
  return true;
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  std::vector<std::string> keys = req->keys;
  std::vector<std::string> values = req->values;
  if(keys.size() != values.size()){
    return false;
  }
  for(int i=0; i < (int)keys.size(); i++){
    data[req->keys.at(i)] = req->values.at(i);
  }
  return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Step 1 and Step 2): Implement!
  std::lock_guard<std::mutex> lock(mtx);
  std::vector<std::string> keys;
  for (const auto& [key, value] : data) {
    keys.push_back(key);
  }
  return keys;
}

