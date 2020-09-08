#include "warble_funcs.h"

// functions to be used by Func
namespace warble_funcs {
bool RegisterUser(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client) {
  RegisteruserRequest registeruser_request;
  const Any &payload = request.payload();
  payload.UnpackTo(&registeruser_request);

  string username = registeruser_request.username();

  // check if username already exists
  if (UserExists(username, key_value_store_client)) {
    LOG(WARNING) << "Register with existing username";
    return false;
  }

  entity::Profile profile;

  string data;
  profile.SerializeToString(&data);
  key_value_store_client.Put(username, data);

  RegisteruserReply registeruser_reply;
  reply.mutable_payload()->PackFrom(registeruser_reply);
  return true;
}

bool UserExists(string username, KeyValueStoreClient &key_value_store_client) {
  // check if username already exists
  vector<string> v_name = {username};
  optional<vector<string>> check = key_value_store_client.Get(v_name);
  if (check.has_value() && check.value().size() > 0) {
    return true;
  } else {
    return false;
  }
}

bool Warble(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client) {
  WarbleRequest warble_request;
  const Any &payload = request.payload();
  payload.UnpackTo(&warble_request);

  // check user
  if (!UserExists(warble_request.username(), key_value_store_client)) {
    return false;
  }

  // check parent id
  warble::Warble warble_check;
  if (warble_request.parent_id() != "-1"
      && !GetWarble(warble_request.parent_id(), warble_check, key_value_store_client)) {
    return false;
  }

  // Generate timestamps
  long int seconds = std::time(nullptr);
  unsigned long long int
      useconds = std::chrono::duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  std::stringstream ss;
  ss << seconds;
  string seconds_s = ss.str();

  // Generate id
  string wid = warble_request.username() + seconds_s;
  wid = "wid_" + wid;

  // Generate warble
  warble::Warble warble;
  warble.set_username(warble_request.username());
  warble.set_parent_id(warble_request.parent_id());
  warble.set_text(warble_request.text());
  warble.set_id(wid);
  warble.mutable_timestamp()->set_seconds(seconds);
  warble.mutable_timestamp()->set_useconds(useconds);

  // store in kvstore
  string data;
  warble.SerializeToString(&data);
  key_value_store_client.Put(wid, data);

  WarbleReply warble_reply;
  warble_reply.set_allocated_warble(&warble);

  reply.mutable_payload()->PackFrom(warble_reply);
  warble_reply.release_warble();
  return true;
}

bool Follow(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client) {
  FollowRequest follow_request;
  const Any &payload = request.payload();
  payload.UnpackTo(&follow_request);

  string username = follow_request.username();
  string to_follow = follow_request.to_follow();

  // check usernames
  entity::Profile user_profile;
  if (!GetProfile(username, user_profile, key_value_store_client)) {
    return false;
  }

  entity::Profile to_follow_profile;
  if (!GetProfile(to_follow, to_follow_profile, key_value_store_client)) {
    return false;
  }

  user_profile.add_following(to_follow);
  to_follow_profile.add_followers(username);

  PutProfile(username, user_profile, key_value_store_client);
  PutProfile(to_follow, to_follow_profile, key_value_store_client);

  FollowReply follow_reply;
  reply.mutable_payload()->PackFrom(follow_reply);
}

bool GetProfile(string username, entity::Profile &profile, KeyValueStoreClient &key_value_store_client) {
  vector<string> v_username = {username};

  optional<vector<string>> v_data = key_value_store_client.Get(v_username);
  string data;
  if (v_data.has_value()) {
    data = v_data.value()[0];
  } else {
    return false;
  }

  profile.ParseFromString(data);
  return true;
}

bool PutProfile(string username, entity::Profile profile, KeyValueStoreClient &key_value_store_client) {
  string data;
  profile.SerializeToString(&data);
  key_value_store_client.Put(username, data);

  return true;
}

bool Read(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client) {
  ReadRequest read_request;
  const Any &payload = request.payload();
  payload.UnpackTo(&read_request);
  string wid = read_request.warble_id();
  ReadReply read_reply;

  // check username
  if (!UserExists(read_request.username(), key_value_store_client)) {
    return false;
  }

  // check warble id
  warble::Warble warble;
  if (GetWarble(wid, warble, key_value_store_client)) {
    warble::Warble *new_warble = read_reply.add_warbles();
    new_warble->set_username(warble.username());
    new_warble->set_text(warble.text());
    new_warble->set_id(warble.id());
    new_warble->set_parent_id(warble.parent_id());
    new_warble->mutable_timestamp()->set_seconds(warble.timestamp().seconds());
    new_warble->mutable_timestamp()->set_useconds(warble.timestamp().useconds());
  } else {
    reply.mutable_payload()->PackFrom(read_reply);
    return false;
  }

  while (warble.parent_id() != "-1") {
    GetWarble(warble.parent_id(), warble, key_value_store_client);
    warble::Warble *new_warble = read_reply.add_warbles();
    new_warble->set_username(warble.username());
    new_warble->set_text(warble.text());
    new_warble->set_id(warble.id());
    new_warble->set_parent_id(warble.parent_id());
    new_warble->mutable_timestamp()->set_seconds(warble.timestamp().seconds());
    new_warble->mutable_timestamp()->set_useconds(warble.timestamp().useconds());
  }

  reply.mutable_payload()->PackFrom(read_reply);
  return true;
}

// Function to stream incoming hashtags
bool Stream(const EventRequest &request, ServerWriter<EventReply>& writer, KeyValueStoreClient &key_value_store_client) {
  StreamRequest streamRequest;
  const Any &payload = request.payload();
  payload.UnpackTo(&streamRequest);
  string hashtag = streamRequest.hashtag();

  // Test hashtag to make sure it is valid
  if (hashtag.empty() || hashtag[0] != '#'
  || 0 != std::count_if(hashtag.begin(), hashtag.end(), [](unsigned char c){ return std::isspace(c); })) {
    return false;
  }

  // Stream all new put requests coming to the KV store
  key_value_store_client.Stream_Puts("wid", [&writer, &hashtag](std::pair<std::string, std::string> kvpair){
    // Construct Protobufs to hold warble data
    warble::Warble* warble = new warble::Warble();
    warble->ParseFromString(kvpair.second);
    // Check the warble's contents for the hashtag

    std::stringstream ss(warble->text());
    string token;
    bool foundMatch = false;
    while (ss >> token) {
      if (token == hashtag) {
        foundMatch = true;
        break;
      }
    }
    // Even if this warble doesn't contain the hashtag, keep listening
    if (!foundMatch) {
      return true;
    }


    // If found, send the warble to the client
    StreamReply streamReply;
    streamReply.set_allocated_warble(warble);
    EventReply reply;
    reply.mutable_payload()->PackFrom(streamReply);

    // Send warble to command line tool
    // If the write fails (connection to command line tool breaks), then disconnect from the KV Store
    return writer.Write(reply);
  });

  return true;
}


bool GetWarble(string wid, warble::Warble &warble, KeyValueStoreClient &key_value_store_client) {
  vector<string> v_wid = {wid};
  optional<vector<string>> v_data = key_value_store_client.Get(v_wid);

  string data;
  if (v_data.has_value() && v_data.value().size() > 0) {
    data = v_data.value()[0];
  } else {
    return false;
  }

  warble.ParseFromString(data);
  return true;
}

bool Profile(const EventRequest &request, EventReply &reply, KeyValueStoreClient &key_value_store_client) {
  ProfileRequest profile_request;
  const Any &payload = request.payload();
  payload.UnpackTo(&profile_request);

  string username = profile_request.username();

  // check if user exists
  vector<string> v_username = {username};
  optional<vector<string>> v_data = key_value_store_client.Get(v_username);
  string data;
  if (v_data.has_value() && v_data.value().size() > 0) {
    data = v_data.value()[0];
  } else {
    data = "";
    return false;
  }

  entity::Profile profile;
  profile.ParseFromString(data);

  ProfileReply profile_reply;

  for (string f : profile.following()) {
    profile_reply.add_following(f);
  }

  for (string f : profile.followers()) {
    profile_reply.add_followers(f);
  }

  reply.mutable_payload()->PackFrom(profile_reply);
  return true;
}
} // namespace warble_funcs
