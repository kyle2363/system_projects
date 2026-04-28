#include "shardkv_client.hpp"


std::optional<std::string> ShardKvClient::Get(const std::string& key) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return std::nullopt;

  // find responsible server in config
  std::optional<std::string> server = config->get_server(key);
  if (!server) return std::nullopt;

  return SimpleClient{*server}.Get(key);
}

bool ShardKvClient::Put(const std::string& key, const std::string& value) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return false;

  // find responsible server in config, then make Put request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return false;
  return SimpleClient{*server}.Put(key, value);
}

bool ShardKvClient::Append(const std::string& key, const std::string& value) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return false;

  // find responsible server in config, then make Append request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return false;
  return SimpleClient{*server}.Append(key, value);
}

std::optional<std::string> ShardKvClient::Delete(const std::string& key) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return std::nullopt;

  // find responsible server in config, then make Delete request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return std::nullopt;
  return SimpleClient{*server}.Delete(key);
}

std::optional<std::vector<std::string>> ShardKvClient::MultiGet(
    const std::vector<std::string>& keys) {
  std::vector<std::string> values;
  values.reserve(keys.size());
  // TODO (Part B, Step 3): Implement!
  auto config = this->Query();
  if (!config) return std::nullopt;

  //to maintain order
  std::map<std::string, std::vector<std::pair<int, std::string>>> server_keys;

  //group keys to the server they are from
  for(int i=0; i<(int)keys.size(); i++){
    std::optional<std::string> server = config->get_server(keys[i]);
    if (!server) return std::nullopt;  
    server_keys[*server].push_back({i, keys[i]});
  }

  std::vector<std::string> final_values(keys.size());
  

  for (const auto& [server, key_pairs] : server_keys) {
    std::vector<std::string> server_keys_only;
    for (const auto& [idx, key] : key_pairs) {
      server_keys_only.push_back(key);
    }

    auto res = SimpleClient{server}.MultiGet(server_keys_only);
    if (!res) return std::nullopt;

    for (int i = 0; i < (int)key_pairs.size(); i++) {
      int original_index = key_pairs[i].first;
      final_values[original_index] = (*res)[i];
    }
  }
  return final_values;
}

bool ShardKvClient::MultiPut(const std::vector<std::string>& keys,
                             const std::vector<std::string>& values) {
  // TODO (Part B, Step 3): Implement!

  auto config = this->Query();
  if (!config) return false;

  std::map<std::string, std::vector<std::pair<int, std::string>>> server_keys;

  //group keys to the server they are from
  for(int i=0; i<(int)keys.size(); i++){
    std::optional<std::string> server = config->get_server(keys[i]);
    if (!server) return false;  
    server_keys[*server].push_back({i, keys[i]});
  }
  

  for (const auto& [server, index_key_pairs] : server_keys) {
    std::vector<std::string> sub_keys;
    std::vector<std::string> sub_values;
    for (const auto& [index, key] : index_key_pairs) {
      sub_keys.push_back(key);
      sub_values.push_back(values[index]);
    }

    auto res = SimpleClient{server}.MultiPut(sub_keys, sub_values);
    if (!res) return false;
  }
  return true;
}

// Shardcontroller functions
std::optional<ShardControllerConfig> ShardKvClient::Query() {
  QueryRequest req;
  if (!this->shardcontroller_conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = this->shardcontroller_conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* query_res = std::get_if<QueryResponse>(&*res)) {
    return query_res->config;
  }

  return std::nullopt;
}

bool ShardKvClient::Move(const std::string& dest_server,
                         const std::vector<Shard>& shards) {
  MoveRequest req{dest_server, shards};
  if (!this->shardcontroller_conn->send_request(req)) return false;

  std::optional<Response> res = this->shardcontroller_conn->recv_response();
  if (!res) return false;
  if (auto* move_res = std::get_if<MoveResponse>(&*res)) {
    return true;
  }

  return false;
}
