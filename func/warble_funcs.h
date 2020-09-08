#ifndef CSCI499_XIAYICHE_FUNC_FUNCS_CPP_H_
#define CSCI499_XIAYICHE_FUNC_FUNCS_CPP_H_

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <ctime>
#include <sstream>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <glog/logging.h>
#include "kvstore_client.h"
#include "../protos/warble.pb.h"
#include "../protos/entity.pb.h"
#include "func.pb.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::optional;
using std::hash;
using std::time_t;
using std::chrono::microseconds;
using std::chrono::system_clock;
using func::KeyValueStoreClient;
using warble::Warble;
using warble::RegisteruserRequest;
using warble::RegisteruserReply;
using warble::ProfileRequest;
using warble::ProfileReply;
using warble::FollowRequest;
using warble::FollowReply;
using warble::WarbleRequest;
using warble::WarbleReply;
using warble::ReadRequest;
using warble::ReadReply;
using warble::StreamRequest;
using warble::StreamReply;
using google::protobuf::Message;
using google::protobuf::Any;
using grpc::ServerWriter;
using func::EventRequest;
using func::EventReply;


namespace warble_funcs {
// Set of warble functions. Used by Func Service. Ephemeral.

// function to register a user
bool RegisterUser(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client);

// function to post or reply a warble
bool Warble(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client);

// function to follow a user
bool Follow(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client);

// function to read a warble
bool Read(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client);

// function to get a user's profile
bool Profile(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client);

// Function to stream incoming hashtags
bool Stream(const EventRequest &request, ServerWriter<EventReply>& writer, KeyValueStoreClient &key_value_store_client);

// function to Get a user's profile, store in profile
bool GetProfile(string username, entity::Profile &profile, KeyValueStoreClient &key_value_store_client);

// function to Put a user' profile to kvstore
bool PutProfile(string username, entity::Profile profile, KeyValueStoreClient &key_value_store_client);

// function to Get a Warble from kvstore
bool GetWarble(string wid, warble::Warble &warble, KeyValueStoreClient &key_value_store_client);

// function to check if a user exists
bool UserExists(string username, KeyValueStoreClient &key_value_store_client);
} // namespace warble_funcs

#endif //CSCI499_XIAYICHE_FUNC_FUNCS_CPP_H_
