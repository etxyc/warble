#ifndef CSCI499_XIAYICHE_COMMAND_LINE_TOOL_FUNC_CLIENT_H_
#define CSCI499_XIAYICHE_COMMAND_LINE_TOOL_FUNC_CLIENT_H_

#include <string>
#include <optional>

#include <grpcpp/grpcpp.h>
#include <glog/logging.h>
#include "func.grpc.pb.h"

using std::string;
using std::optional;
using std::function;
using grpc::Channel;
using google::protobuf::Message;
using google::protobuf::Any;
using grpc::Status;
using grpc::ClientContext;
using func::FuncService;
using func::EventRequest;
using func::EventReply;
using func::HookRequest;
using func::HookReply;
using func::UnhookRequest;
using func::UnhookReply;

namespace command_line_tool {
// Client to call Func
class FuncClient {
 public:
  // Constructor
  FuncClient(std::shared_ptr<Channel> channel) : stub_(FuncService::NewStub(channel)) {}

  // function to send an event to Func
  optional<EventReply> Event(int event_type, const Message &payload);

  // function to send a stream event to func. Given lambda will run until it returns false, or server
  // closes connection.
  void StreamEvent(int eventType, const Message &payload, std::function<bool(Any)> lambda);

  // function to hook
  bool Hook(int event_type, string event_func);

  // function to unhook
  bool Unhook(int event_type);

 private:
  // stub to call methods in Func
  std::unique_ptr<FuncService::Stub> stub_;
};
} // namespace command_line_tool

#endif //CSCI499_XIAYICHE_COMMAND_LINE_TOOL_FUNC_CLIENT_H_
