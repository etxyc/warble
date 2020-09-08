#ifndef CSCI499_XIAYICHE_KVSTORE_SERVER_H
#define CSCI499_XIAYICHE_KVSTORE_SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>
#include <memory>
#include <unordered_map>
#include <fstream>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include "../protos/kvstore.pb.h"
#include "../protos/kvstore.grpc.pb.h"
#include "utils/concurrent_hash_map.h"
#include <gflags/gflags.h>

using std::string;
using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::set;
using std::pair;
using std::queue;
using std::mutex;
using std::scoped_lock;
using std::chrono::seconds;
using std::this_thread::sleep_for;
using std::shared_ptr;
using std::make_pair;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using grpc::ServerReaderWriter;
using kvstore::KeyValueStore;
using kvstore::PutRequest;
using kvstore::PutReply;
using kvstore::GetRequest;
using kvstore::GetReply;
using kvstore::RemoveRequest;
using kvstore::RemoveReply;
using kvstore::StreamPutsRequest;
using kvstore::StreamPutsReply;
using kvstore::ConcurrentHashMap;

DEFINE_string(store, "", "Store data");

// Class to provice key value store service for use by Func
class KeyValueStoreImpl final : public KeyValueStore::Service {
 public:
  // put k-v pair into the database
  Status put(ServerContext *context, const PutRequest *request, PutReply *reply) override;

  // get value by key
  Status get(ServerContext *context, ServerReaderWriter<GetReply, GetRequest> *stream) override;

  // delete a key
  Status remove(ServerContext *context, const RemoveRequest *request, RemoveReply *response) override;

  // stream incoming put requests, optionally with a filter
  Status stream_puts(ServerContext* context, const StreamPutsRequest* request, ServerWriter<StreamPutsReply>* writer);

  // function to store data to a file, load from the file first if the file exists
  void store(string file);

  // function to handle termination signal and save file
  static void static_signal_handler(int signal);

 private:
  // database to store kv pairs, supporting thread-safety
  static ConcurrentHashMap map_;

  // Set of streaming listeners to send data to
  static set<shared_ptr<queue<pair<string, string>>>> update_queues_;
  static mutex update_queue_mutex_;

  // boolean value to store if need to store disk persistence
  static bool is_persistence_;

  // name of the disk persistence file name;
  static string disk_file_;

  // function to load the disk file to map_
  bool load(string file);

  // function to store map_ to file
  bool persist();
};

#endif //CSCI499_XIAYICHE_KVSTORE_SERVER_H
