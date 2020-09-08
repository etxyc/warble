#ifndef CSCI499_XIAYICHE_KVSTORE_CLIENT_H
#define CSCI499_XIAYICHE_KVSTORE_CLIENT_H

#include <string>
#include <vector>
#include <optional>

#include <grpcpp/grpcpp.h>
#include "../protos/kvstore.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using kvstore::PutRequest;
using kvstore::PutReply;
using kvstore::KeyValueStore;

namespace func {
// Client class to call key value store service
class KeyValueStoreClient {
 public:
  // constructor
  KeyValueStoreClient(std::shared_ptr<Channel> channel) : stub_(KeyValueStore::NewStub(channel)) {}

  // client function to put a kv pair into database
  bool Put(const std::string &key, const std::string &value);

  // client function to get value by key
  std::optional<std::vector<std::string>> Get(const std::vector<std::string> &keys);

  // client function to remove a pair by key
  bool Remove(const std::string &key);

  // Register a function to run (typically a lambda) on every received KV pair from the stream_puts command
  // The given function should return true to continue, and false to close the connection after each received pair
  void Stream_Puts(std::string filter, std::function<bool(std::pair<std::string, std::string>)> lambda);

 private:
  // the stub to remotely call server functions
  std::unique_ptr<KeyValueStore::Stub> stub_;
};
} // namespace func

#endif //CSCI499_XIAYICHE_KVSTORE_CLIENT_H
