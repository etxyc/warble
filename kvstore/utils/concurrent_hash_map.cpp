#include "concurrent_hash_map.h"

namespace kvstore {
optional<string> ConcurrentHashMap::Get(const string &key) {
  if (!ContainsKey(key)) {
    return {};
  }

  lock_guard<mutex> lock(mutex_);
  return map_[key];
}

bool ConcurrentHashMap::Put(const string &key, const string &value) {
  lock_guard<mutex> lock(mutex_);
  map_[key] = value;
  return true;
}

bool ConcurrentHashMap::Remove(const string &key) {
  lock_guard<mutex> lock(mutex_);
  map_.erase(key);
  return true;
}

bool ConcurrentHashMap::ContainsKey(const string &key) {
  lock_guard<mutex> lock(mutex_);
  if (map_.find(key) != map_.end()) {
    return true;
  } else {
    return false;
  }
}

bool ConcurrentHashMap::Persist(string file) {
  lock_guard<mutex> lock(mutex_);
  ofstream outf(file, ios::trunc | ios::binary);

  // for each kv in map, write to file
  MapProtoBuff map_proto_buff;

  for (auto i : map_) {
    (*map_proto_buff.mutable_map())[i.first] = i.second;
  }

  string data;
  map_proto_buff.SerializeToString(&data);
  outf << data;

  outf.close();
  return true;
}

bool ConcurrentHashMap::Load(string file) {
  lock_guard<mutex> lock(mutex_);

  // clean map
  map_.clear();

  // open file, put kv
  ifstream inf(file);

  std::stringstream temp;
  temp << inf.rdbuf();
  string data = temp.str();
  inf.close();

  MapProtoBuff map_proto_buff;
  map_proto_buff.ParseFromString(data);

  unordered_map<string, string> map_copy(map_proto_buff.map().begin(), map_proto_buff.map().end());
  map_ = map_copy;

  return true;
}
} // namespace kvstore
