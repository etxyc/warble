#ifndef CSCI499_XIAYICHE_KVSTORE_UTILS_CONCURRENT_HASH_MAP_H_
#define CSCI499_XIAYICHE_KVSTORE_UTILS_CONCURRENT_HASH_MAP_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <fstream>
#include <sstream>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include "../../protos/entity.pb.h"
#include "../../protos/entity.grpc.pb.h"

using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::unordered_map;
using std::string;
using std::mutex;
using std::lock_guard;
using std::optional;
using std::vector;
using google::protobuf::Map;
using entity::MapProtoBuff;

namespace kvstore {
// Class of key-value store supports safe, concurrent access by multiple callers.
class ConcurrentHashMap {
 public:
  // Function to Get value by key. Return empty vector if key cannot be found
  optional<string> Get(const string &key);

  // Function to Put the key value pair. Return true if operation successes.
  bool Put(const string &key, const string &value);

  // Function to Remove the key value pair. Return true if operation successes.
  bool Remove(const string &key);

  // Function to check if the key exists. Return true if key exist, otherwise return false
  bool ContainsKey(const string &key);

  // Fucntion to store the map to file
  bool Persist(string file);

  // Function to load a file to map
  bool Load(string file);

 private:
  // map to store the key value pairs
  unordered_map<string, string> map_;
  // mutex to ensure multi-thread safety
  mutex mutex_;
};
} // namespace kvstore
#endif //CSCI499_XIAYICHE_KVSTORE_UTILS_CONCURRENT_HASH_MAP_H_
