#include "func_server.h"

FuncServiceImpl::FuncServiceImpl() : key_value_store_client_(grpc::CreateChannel("localhost:50001",
                                                                                 grpc::InsecureChannelCredentials())) {
  function_id_string_[kRegisterUser] = "RegisterUser";
  function_id_string_[kWarble] = "Warble";
  function_id_string_[kFollow] = "Follow";
  function_id_string_[kRead] = "Read";
  function_id_string_[kProfile] = "Profile";
  function_id_string_[kStream] = "Stream";
}

Status FuncServiceImpl::hook(ServerContext *context, const HookRequest *request, HookReply *reply) {
  int event_type = request->event_type();
  string event_func = request->event_function();

  lock_guard<mutex> lock(mutex_);

  if (event_func == "RegisterUser") {
    function_map_[event_func] = warble_funcs::RegisterUser;
  } else if (event_func == "Warble") {
    function_map_[event_func] = warble_funcs::Warble;
  } else if (event_func == "Follow") {
    function_map_[event_func] = warble_funcs::Follow;
  } else if (event_func == "Read") {
    function_map_[event_func] = warble_funcs::Read;
  } else if (event_func == "Profile") {
    function_map_[event_func] = warble_funcs::Profile;
  } else if (event_func == "Stream") {
    stream_function_map_[event_func] = warble_funcs::Stream;
  }

  return Status::OK;
}

Status FuncServiceImpl::unhook(ServerContext *context, const UnhookRequest *request, UnhookReply *reply) {
  int event_type = request->event_type();

  lock_guard<mutex> lock(mutex_);

  if (event_type == kRegisterUser) {
    function_map_.erase("RegisterUser");
  } else if (event_type == kWarble) {
    function_map_.erase("Warble");
  } else if (event_type == kFollow) {
    function_map_.erase("Follow");
  } else if (event_type == kRead) {
    function_map_.erase("Read");
  } else if (event_type == kProfile) {
    function_map_.erase("Profile");
  } else if (event_type == kStream) {
    stream_function_map_.erase("Stream");
  }

  return Status::OK;
}

Status FuncServiceImpl::event(ServerContext *context, const EventRequest *request, EventReply *reply) {
  LOG(INFO) << "Received Event";

  int event_type = request->event_type();

  string func_name;
  if (function_id_string_.find(event_type) != function_id_string_.end()) {
    func_name = function_id_string_[event_type];
    // Check that the function type supports non-streaming operations
    if (unaryFunctions_.find(func_name) == unaryFunctions_.end()) {
      LOG(INFO) << "Unsupported function triggered for non-stream event: " << func_name;
      return Status::CANCELLED;
    }
    LOG(INFO) << func_name;
  } else {
    LOG(WARNING) << "Cannot find Event: " << event_type;
    return Status::OK;
  }

  function<bool(const EventRequest &, EventReply &, KeyValueStoreClient &)> func;
  if (function_map_.find(func_name) != function_map_.end()) {
    LOG(INFO) << "Find Registered Function.";
    func = function_map_[func_name];
  } else {
    LOG(WARNING) << "Cannot find Function: " << func_name;
    return Status::CANCELLED;
  }

  bool result = func(*request, *reply, key_value_store_client_);
  LOG(INFO) << "Finish Func";

  if (result) {
    return Status::OK;
  } else {
    return Status::CANCELLED;
  }
}

Status FuncServiceImpl::stream_event(ServerContext *context, const EventRequest *request, ServerWriter<EventReply> *writer) {
  LOG(INFO) << "Received Stream Event";

  int event_type = request->event_type();

  string func_name;
  if (function_id_string_.find(event_type) != function_id_string_.end()) {
    func_name = function_id_string_[event_type];
    // Check that the function type supports streaming
    if (streamFunctions_.find(func_name) == streamFunctions_.end()) {
      LOG(INFO) << "Unsupported function triggered for stream event: " << func_name;
      return Status::CANCELLED;
    }
    LOG(INFO) << func_name;
  } else {
    LOG(WARNING) << "Cannot find Event: " << event_type;
    return Status::OK;
  }

  function<bool(const EventRequest &, ServerWriter<EventReply> &, KeyValueStoreClient &)> func;
  if (stream_function_map_.find(func_name) != stream_function_map_.end()) {
    LOG(INFO) << "Find Registered Function.";
    func = stream_function_map_[func_name];
  } else {
    LOG(WARNING) << "Cannot find Function: " << func_name;
    return Status::CANCELLED;
  }

  bool result = func(*request, *writer, key_value_store_client_);
  LOG(INFO) << "Finish Stream Func";

  if (result) {
    return Status::OK;
  } else {
    return Status::CANCELLED;
  }
}


void RunServer() {
  LOG(INFO) << "Running Func Server";

  std::string server_address("0.0.0.0:50000");
  FuncServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case, it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char *argv[]) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  RunServer();

  return 0;
}
