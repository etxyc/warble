#include "func_client.h"

namespace command_line_tool {
optional<EventReply> FuncClient::Event(int event_type, const Message &payload) {
  EventRequest request;
  request.set_event_type(event_type);
  request.mutable_payload()->PackFrom(payload);

  ClientContext context;
  EventReply reply;
  Status status = stub_->event(&context, request, &reply);

  if (!status.ok()) {
    return {};
  }

  return reply;
}

bool FuncClient::Hook(int event_type, string event_func) {
  HookRequest hook_request;
  hook_request.set_event_type(event_type);
  hook_request.set_event_function(event_func);

  ClientContext context;
  HookReply reply;
  Status status = stub_->hook(&context, hook_request, &reply);

  if (!status.ok()) {
    return false;
  } else {
    return true;
  }
}

bool FuncClient::Unhook(int event_type) {
  UnhookRequest unhook_request;
  unhook_request.set_event_type(event_type);

  ClientContext context;
  UnhookReply reply;
  Status status = stub_->unhook(&context, unhook_request, &reply);

  if (status.ok()) {
    return true;
  } else {
    return false;
  }
}

void FuncClient::StreamEvent(int eventType, const Message &payload, std::function<bool(Any)> lambda) {
  EventRequest request;
  request.set_event_type(eventType);
  request.mutable_payload()->PackFrom(payload);

  ClientContext context;

  std::shared_ptr<grpc::ClientReader<EventReply>> stream(stub_->stream_event(&context, request));
  EventReply reply;
  while (stream->Read(&reply)) {
    bool success = lambda(reply.payload());
    if (!success) {
      break;
    }
  }
}


} // command_line_tool