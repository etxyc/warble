#ifndef CSCI499_XIAYICHE_FUNC_PROTOS_FUNC_SERVER_H_
#define CSCI499_XIAYICHE_FUNC_PROTOS_FUNC_SERVER_H_

#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <glog/logging.h>
#include "../protos/func.pb.h"
#include "../protos/func.grpc.pb.h"
#include "kvstore_client.h"
#include "warble_funcs.h"

using std::function;
using std::string;
using std::mutex;
using std::lock_guard;
using std::unordered_map;
using std::set;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using func::FuncService;
using func::HookRequest;
using func::HookReply;
using func::UnhookRequest;
using func::UnhookReply;
using func::EventRequest;
using func::EventReply;
using func::KeyValueStoreClient;
using google::protobuf::Message;

enum EventIdName {
  kRegisterUser = 1,
  kWarble = 2,
  kFollow = 3,
  kRead = 4,
  kProfile = 5,
  kHookAll = 7,
  kUnhookAll = 8,
  kReply = 9,
  kStream = 10
};

// Class to hook and unhook events and function. To execute given functions and respond replies.
class FuncServiceImpl final : public FuncService::Service {
 public:
  // Constructor
  FuncServiceImpl();

  // Function to hook an event
  Status hook(ServerContext *context, const HookRequest *request, HookReply *reply) override;

  // Function to unhook an event
  Status unhook(ServerContext *context, const UnhookRequest *request, UnhookReply *reply) override;

  // Function to execute an event
  Status event(ServerContext *context, const EventRequest *request, EventReply *reply) override;

  Status stream_event(ServerContext *context, const EventRequest *request, grpc::ServerWriter<EventReply> *writer) override;

 private:
  // Map to store functions for event
  unordered_map<string, function<bool(const EventRequest &, EventReply &, KeyValueStoreClient &)>> function_map_;
  unordered_map<string, function<bool(const EventRequest &, grpc::ServerWriter<EventReply> &, KeyValueStoreClient &)>> stream_function_map_;

  // Map to store event id, event name pairs
  unordered_map<int, string> function_id_string_;

  // key value store client
  KeyValueStoreClient key_value_store_client_;

  // mutex to support hooking functions
  mutex mutex_;

  // Function type definitions
  set<string> unaryFunctions_ = {"RegisterUser", "Warble", "Follow", "Read", "Profile"};
  set<string> streamFunctions_ = {"Stream"};
};

#endif //CSCI499_XIAYICHE_FUNC_PROTOS_FUNC_SERVER_H_
