#include "kvstore_client.h"
#include <thread>

using grpc::ClientContext;
using grpc::ClientReaderWriter;
using kvstore::PutRequest;
using kvstore::PutReply;
using kvstore::GetRequest;
using kvstore::GetReply;
using kvstore::RemoveRequest;
using kvstore::RemoveReply;

namespace func {
bool KeyValueStoreClient::Put(const std::string &key, const std::string &value) {
  PutRequest request;
  request.set_key(key);
  request.set_value(value);

  PutReply reply;
  ClientContext context;

  Status status = stub_->put(&context, request, &reply);

  if (status.ok()) {
    return true;
  } else {
    return false;
  }
}

std::optional<std::vector<std::string>> KeyValueStoreClient::Get(const std::vector<std::string> &keys) {
  std::vector<std::string> values;
  ClientContext context;
  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply>> stream(stub_->get(&context));

  std::thread writer([stream, keys]() {
    for (std::string key : keys) {
      GetRequest request;
      request.set_key(key);
      stream->Write(request);
    }
    stream->WritesDone();
  });

  GetReply reply;
  while (stream->Read(&reply)) {
    values.push_back(reply.value());
  }

  writer.join();

  Status status = stream->Finish();

  if (!status.ok()) {
    return {};
  }

  return values;
}

bool KeyValueStoreClient::Remove(const std::string &key) {
  RemoveRequest request;
  request.set_key(key);

  RemoveReply reply;
  ClientContext context;

  Status status = stub_->remove(&context, request, &reply);

  if (status.ok()) {
    return true;
  } else {
    return false;
  }
}

void KeyValueStoreClient::Stream_Puts(std::string filter, std::function<bool(std::pair<std::string, std::string>)> lambda) {
  ClientContext context;
  kvstore::StreamPutsRequest request;
  request.set_filter(filter);

  std::shared_ptr<grpc::ClientReader<kvstore::StreamPutsReply>> stream(stub_->stream_puts(&context, request));
  kvstore::StreamPutsReply reply;
  while (stream->Read(&reply)) {
    bool success = lambda(std::make_pair(reply.key(), reply.value()));
    if (!success) {
      break;
    }
  }
}

} // namespace func
