#include "simple_client.hpp"
#include <sstream>
#include <string>

std::optional<std::string> SimpleClient::Get(const std::string& key) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  GetRequest req{key};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* get_res = std::get_if<GetResponse>(&*res)) {
    return get_res->value;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to Get value from server: ", error_res->msg);
  }

  return std::nullopt;
}

bool SimpleClient::Put(const std::string& key, const std::string& value) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  PutRequest req{key, value};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* put_res = std::get_if<PutResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to Put value to server: ", error_res->msg);
  }

  return false;
}

bool SimpleClient::Append(const std::string& key, const std::string& value) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  AppendRequest req{key, value};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* append_res = std::get_if<AppendResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to Append value to server: ", error_res->msg);
  }

  return false;
}

std::optional<std::string> SimpleClient::Delete(const std::string& key) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  DeleteRequest req{key};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* delete_res = std::get_if<DeleteResponse>(&*res)) {
    return delete_res->value;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to Delete value on server: ", error_res->msg);
  }

  return std::nullopt;
}

std::optional<std::vector<std::string>> SimpleClient::MultiGet(
    const std::vector<std::string>& keys) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  MultiGetRequest req{keys};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* multiget_res = std::get_if<MultiGetResponse>(&*res)) {
    return multiget_res->values;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to MultiGet values on server: ", error_res->msg);
  }

  return std::nullopt;
}

bool SimpleClient::MultiPut(const std::vector<std::string>& keys,
                            const std::vector<std::string>& values) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  MultiPutRequest req{keys, values};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* multiput_res = std::get_if<MultiPutResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(YELLOW, "Failed to MultiPut values on server: ", error_res->msg);
  }

  return false;
}

std::string toLower(std::string data) {
  for (char &c : data) {
      c = std::tolower((unsigned char)c);
  }
  return data;
}

// GDPRDelete implements an anonymization-based "right to be forgotten".
// Instead of fully deleting data from the KVStore, this approach removes the
// connection between a user’s identity and their content while preserving the
// structure of the data.
//
// Specifically, this function:
// 1. Generates a unique anonymous user ID (e.g., "anon_user_00").
// 2. Retrieves the target user’s username and splits it into individual words.
// 3. Iterates through all users, their posts, and associated replies.
// 4. Performs case-insensitive substring matching to find occurrences of the
//    user’s name in post and reply content.
// 5. Replaces all matches with the generated anonymous ID.
// 6. Updates the original user_id → username mapping to instead point to the
//    anonymous ID.
// 7. Stores a reverse mapping (anon_id → original user_id) for traceability.
//
// Design choices and tradeoffs:
// - We choose anonymization over deletion to preserve other users’ content and
//   avoid breaking references between posts and replies.
// - The "all_users" key is intentionally not updated. This may leave the store
//   in a partially inconsistent state, but is allowed per assignment guidelines.
//       -it also serves to make the anon user itself harder to find unless 
//        you first call get with the user_id
// - Matching is case-insensitive but uses substring replacement, which may
//   replace partial words (e.g., "Ann" in "banana").
// - This implementation only traverses one level of replies.
// - This assumes everyone has completely different first and last names as any overlap
//   will be replaced even if it not the same person
// 
// Thread safety:
// - This function runs client-side and issues independent requests to the server.
//   Thread safety is handled by the server’s KVStore implementation.

bool SimpleClient::GDPRDelete(const std::string& user) {
  // TODO: Write your GDPR deletion code here!
  // You can invoke operations directly on the client object, like so:
  //
  // std::string key("user_1_posts");
  // std::optional<std::string> posts = Get(key);
  // ...
  //
  // Assume the `user` argument is a user ID such as "user_1".

  //new id
  std::string new_id_base = "anon_user_";
  std::string id_num = "00";
  //loops till a unused id is found
  while(Get(new_id_base + id_num).has_value()){
    int id_num_int = std::stoi(id_num)+1;
    if(id_num_int < 10){
      id_num = "0" + std::to_string(id_num_int);
    }else{
      id_num = std::to_string(id_num_int);
    }
  }
  std::string new_id = new_id_base + id_num;

  //get users name

  std::optional<std::string> name_opt = Get(user);
  std::string name;
  if(name_opt.has_value()){
    name = name_opt.value();
  }else{
    return false;
  }

  //create a array of a users name where each element is a word in their name
  std::stringstream ss(name);
  std::string word;
  std::vector<std::string> name_list;

  while (ss >> word) name_list.push_back(word);

  //make array of there name only lowercase since it only used for search purposes
  for (std::string &w : name_list) {
    w = toLower(w);
  }

  //replace all occurrences of there name with new new_id

  //get all users
  std::optional<std::string> user_opt = Get("all_users");
  std::string users;
  if(user_opt.has_value()){
    users = user_opt.value();
  }else{
    return false;
  }

  std::stringstream ss_u(users);
  std::string single_user;
  //put users in vector form
  std::vector<std::string> users_vector;

  // Use getline with ',' as the third argument (the delimiter)
  while (std::getline(ss_u, single_user, ',')) {
    users_vector.push_back(single_user);
  }

  //loop over all users
  for(int i=0;i < (int)users_vector.size();i++){
    //check if key exist for the users post
    auto posts_opt = Get(users_vector[i] + "_posts");
    if (!posts_opt) continue;

    //make vector of all the user i post
    std::string user_posts = posts_opt.value();
    std::stringstream ss_user_post(user_posts);
    std::string single_user_post;
    std::vector<std::string> users_post_vector;

    // Use getline with ',' as the third argument (the delimiter)
    while (std::getline(ss_user_post, single_user_post, ',')) {
      users_post_vector.push_back(single_user_post);
    }

    //search over all post
    for(int j=0;j < (int)users_post_vector.size();j++){
      //get content of post
      std::optional<std::string> content_opt = Get(users_post_vector[j]);
      if (!content_opt) continue;
      std::string user_post_content = content_opt.value();

      //convert to lower case for search
      std::string lower_content = toLower(user_post_content);
      
      //for each element in namelist search and replace every occurrence of it in the content
      for(int k=0;k < (int)name_list.size();k++){
        size_t pos = 0;

        while ((pos = lower_content.find(name_list[k], pos)) != std::string::npos) {
          user_post_content.replace(pos, name_list[k].length(), new_id);
          lower_content.replace(pos, name_list[k].length(), new_id); 
          pos += new_id.length();
        }
        
      }
      //replace in the original
      Put(users_post_vector[j], user_post_content);

      //check all replies
      auto replies_opt = Get(users_post_vector[j] + "_replies");
      if (replies_opt) {
        std::stringstream ss_replies(replies_opt.value());
        std::string reply_id;
        
        //loop over each reply
        while (std::getline(ss_replies, reply_id, ',')) {
          auto reply_content_opt = Get(reply_id);
          if (!reply_content_opt) continue;
    
          std::string reply_content = reply_content_opt.value();
          std::string lower_reply = toLower(reply_content);

          //loop over each name
          for (int k = 0; k < (int)name_list.size(); k++) {
            
            size_t pos = 0;
            //replace
            while ((pos = lower_reply.find(name_list[k], pos)) != std::string::npos) {
              reply_content.replace(pos, name_list[k].length(), new_id);
              lower_reply.replace(pos, name_list[k].length(), new_id);
              pos += new_id.length();
            }
          }
          //replace the reply with the new one
          Put(reply_id, reply_content);
        }
      }
    }    
  }

  //replace the original name of the user with the new one
  Put(user, new_id);
  //used to link anon users back to the user id
  Put(new_id, user);

  return true;
}
