#include "kvstore_server.h"

Status KeyValueStoreImpl::put(ServerContext *context, const PutRequest *request, PutReply *reply) {
  map_.Put(request->key(), request->value());

  {
    scoped_lock<mutex> l(update_queue_mutex_);
    for (auto queue : update_queues_) {
      queue->push(make_pair(request->key(), request->value()));
    }
  }

  if (is_persistence_) {
    persist();
  }

  return Status::OK;
}

Status KeyValueStoreImpl::get(ServerContext *context, ServerReaderWriter<GetReply, GetRequest> *stream) {
  GetRequest request;

  while (stream->Read(&request)) {
    std::string key = request.key();
    GetReply reply;
    string value = map_.Get(key).value();
    reply.set_value(value);
    stream->Write(reply);
  }

  return Status::OK;
}

Status KeyValueStoreImpl::remove(ServerContext *context, const RemoveRequest *request, RemoveReply *response) {
  std::string key = request->key();
  map_.Remove(key);
  string value = map_.Get(key).value();

  if (is_persistence_) {
    persist();
  }

  return Status::OK;
}

Status KeyValueStoreImpl::stream_puts(ServerContext* context, const StreamPutsRequest* request, ServerWriter<StreamPutsReply>* writer) {
  string filter = request->filter();
  shared_ptr<queue<pair<string, string>>> queue(new std::queue<pair<string, string>>);
  // Add this thread as a queue listening for new put requests
  {
    scoped_lock<mutex> l(update_queue_mutex_);
    update_queues_.insert(queue);
  }

  bool success = true;
  while (success) {
    // Check for any updates in queue
    {
      scoped_lock<mutex> l(update_queue_mutex_);
      while (!queue->empty() && success) {
        pair<string, string> putReq = queue->front();
        queue->pop();
        if (putReq.first.find(filter) == std::string::npos) {
          continue;
        }
        StreamPutsReply reply;
        reply.set_key(putReq.first);
        reply.set_value(putReq.second);
        success = writer->Write(reply);
      }
    }
    // Sleep to allow other threads to update queue
    seconds sleepDuration(2);
    sleep_for(sleepDuration);
  }

  // Remove this thread as a queue listening for new put requests once client disconnects
  {
    scoped_lock<mutex> l(update_queue_mutex_);
    update_queues_.erase(queue);
  }

  return Status::OK;
}


void KeyValueStoreImpl::store(string file) {
  is_persistence_ = true;
  disk_file_ = file;

  ifstream inf(disk_file_);
  // if exist, load
  if (inf) {
    inf.close();
    load(disk_file_);
  } else { // if not exist, persist
    inf.close();
    persist();
  }
}

bool KeyValueStoreImpl::persist() {
  map_.Persist(disk_file_);
  return true;
}

bool KeyValueStoreImpl::load(string file) {
  map_.Load(file);
  return true;
}

void KeyValueStoreImpl::static_signal_handler(int signal) {
  if (is_persistence_) {
    map_.Persist(disk_file_);
    cout << endl << "File Saved" << endl;
  }

  exit(signal);
}

void RunServer(string file) {
  std::string server_address("0.0.0.0:50001");
  KeyValueStoreImpl service;

  signal(SIGINT, KeyValueStoreImpl::static_signal_handler);
  signal(SIGTERM, KeyValueStoreImpl::static_signal_handler);

  if (file != "") {
    cout << "Load from/save to file: " << file << endl;
    service.store(file);
  }

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

ConcurrentHashMap KeyValueStoreImpl::map_;
std::set<shared_ptr<queue<pair<string, string>>>> KeyValueStoreImpl::update_queues_;
mutex KeyValueStoreImpl::update_queue_mutex_;
bool KeyValueStoreImpl::is_persistence_ = false;
string KeyValueStoreImpl::disk_file_ = "";

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  RunServer(FLAGS_store);

  return 0;
}
