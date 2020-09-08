#include "command_line_tool.h"

namespace command_line_tool {
CommandLineTool::CommandLineTool() : func_client_(grpc::CreateChannel("localhost:50000",
                                                                      grpc::InsecureChannelCredentials())),
                                     func_map_() {
  func_map_[kRegisterUser] = "RegisterUser";
  func_map_[kWarble] = "Warble";
  func_map_[kFollow] = "Follow";
  func_map_[kRead] = "Read";
  func_map_[kProfile] = "Profile";
  func_map_[kStream] = "Stream";
}

void CommandLineTool::RegisterUser(string username) {
  RegisteruserRequest registeruser_request;
  registeruser_request.set_username(username);
  optional<EventReply> reply = func_client_.Event(kRegisterUser, registeruser_request);

  if (reply.has_value()) {
    cout << "Register successfully." << endl;
    cout << "" << endl;
  } else {
    cout << "Register failed. Try again." << endl;
    cout << "Try Hook all functions." << endl;
    cout << "Try another username. This username might be used." << endl;
    return;
  }
}

string CommandLineTool::LogIn(string username) {
  return username;
}

void CommandLineTool::Warble(string username, string text, string parent_id) {
  if (parent_id == "") {
    parent_id = "-1";
  }

  WarbleRequest warble_request;
  warble_request.set_username(username);
  warble_request.set_text(text);
  warble_request.set_parent_id(parent_id);

  optional<EventReply> reply = func_client_.Event(kWarble, warble_request);

  if (reply.has_value()) {
    cout << "Post warble successfully." << endl;
    cout << "" << endl;
  } else {
    cout << "Post warble failed. Try again." << endl;
    cout << "Try Hook all functions." << endl;
    cout << "Try another username. This username might not exist." << endl;
    cout << "Try another parent id. Warble to reply might not exist." << endl;
    return;
  }

  WarbleReply warble_reply;
  const Any &payload = reply.value().payload();
  payload.UnpackTo(&warble_reply);

  string id = warble_reply.warble().id();
  std::cout << "warble id: " << id << std::endl;
}

void CommandLineTool::Follow(string username, string following_username) {
  FollowRequest follow_request;
  follow_request.set_username(username);
  follow_request.set_to_follow(following_username);
  optional<EventReply> reply = func_client_.Event(kFollow, follow_request);

  if (reply.has_value()) {
    cout << "Follow successfully." << endl;
    cout << "" << endl;
  } else {
    cout << "Follow failed. Try again." << endl;
    cout << "Try Hook all functions." << endl;
    cout << "username or user to follow might not exist." << endl;
    return;
  }

}

void CommandLineTool::Read(string warble_id, string username) {
  ReadRequest read_request;
  read_request.set_warble_id(warble_id);
  read_request.set_username(username);

  optional<EventReply> reply = func_client_.Event(kRead, read_request);

  if (reply.has_value()) {
    cout << "Read warble successfully." << endl;
    cout << "" << endl;
  } else {
    cout << "Read warble failed. Try again." << endl;
    cout << "Try Hook all functions." << endl;
    cout << "This username might not exist. Try another username." << endl;
    cout << "This warble id might not exist. Try another warble id." << endl;
    return;
  }

  ReadReply read_reply;
  const Any &payload = reply.value().payload();
  payload.UnpackTo(&read_reply);

  if (read_reply.warbles_size() == 0) {
    cout << "No warble can be found. Try another warble id." << endl;
    return;
  }

  for (warble::Warble warble : read_reply.warbles()) {
    PrintWarble(warble);
  }
}

void CommandLineTool::Profile(string username) {
  ProfileRequest profile_request;
  profile_request.set_username(username);

  optional<EventReply> reply = func_client_.Event(kProfile, profile_request);

  if (reply.has_value()) {
    cout << "Read profile successfully." << endl;
    cout << "" << endl;
  } else {
    cout << "Read profile failed. Try again." << endl;
    cout << "Try Hook all functions." << endl;
    cout << "Try another username. This username might not exist." << endl;
    return;
  }

  ProfileReply profile_reply;
  const Any &payload = reply.value().payload();
  payload.UnpackTo(&profile_reply);

  cout << "username: " << username << endl << endl;

  cout << "Followers: " << endl;
  if (profile_reply.followers_size() == 0) {
    cout << "No followers" << endl;
  }

  for (auto follower : profile_reply.followers()) {
    cout << follower << endl;
  }

  cout << endl;

  cout << "Following: " << endl;
  if (profile_reply.following_size() == 0) {
    cout << "No following" << endl;
  }

  for (auto following : profile_reply.following()) {
    cout << following << endl;
  }
}

void CommandLineTool::Stream(string hashtag) {

  StreamRequest request;
  request.set_hashtag(hashtag);
  func_client_.StreamEvent(kStream, request, [this](Any payload) {
    StreamReply reply;
    payload.UnpackTo(&reply);
    std::cout << "Printing Warble" << std::endl;
    PrintWarble(reply.warble());
    return true; // Listen forever, until the user quits with command-c
  });
}

int CommandLineTool::CheckAction(string registeruser,
                                 string user,
                                 string warble,
                                 string reply,
                                 string follow,
                                 string read,
                                 string stream,
                                 bool profile,
                                 bool hook,
                                 bool unhook) {
  if (hook) {
    return kHookAll;
  }

  if (unhook) {
    return kUnhookAll;
  }

  // registeruser
  if (user == "" && registeruser != "") {
    return kRegisterUser;
  }

  // warble
  if (warble != "") {
    if (reply == "") {
      return kWarble;
    } else { // reply warble
      return kReply;
    }
  }

  if (reply != "") {
    cout << "--reply must be specified along with --warble." << endl;
    return -1;
  }

  // follow
  if (follow != "") {
    return kFollow;
  }

  // read
  if (read != "") {
    return kRead;
  }

  // profile
  if (profile) {
    return kProfile;
  }

  if (stream != "") {
    // Validate the hashtag (this is also done server-side, here is just for convenience
    if (stream.empty() || stream[0] != '#'
        || 0 != std::count_if(stream.begin(), stream.end(), [](unsigned char c){ return std::isspace(c); })) {
      cout << "A valid hashtag must start with a #, followed by one or more non-whitespace characters." << endl;
      return -1;
    }
    return kStream;
  }

  return -1;
}

void CommandLineTool::HookAll() {
  cout << "Hook all functions" << endl;

  for (auto func : func_map_) {
    if (!func_client_.Hook(func.first, func.second)) {
      cout << "Hook function failed. Try again. " << endl;
      return;
    }
  }

  cout << "All functions are hooked." << endl;
}

void CommandLineTool::UnhookAll() {
  cout << "Unhook all functions" << endl;

  for (auto func : func_map_) {
    if (!func_client_.Unhook(func.first)) {
      cout << "Unhook function failed. Try again." << endl;
      return;
    }
  }

  cout << "All functions are unhooked." << endl;
}

void CommandLineTool::PrintWarble(warble::Warble warble) {
  cout << "warble id: " << warble.id() << endl;

  if (warble.parent_id() != "-1") {
    cout << "reply to: " << warble.parent_id() << endl;
  }
  cout << "username: " << warble.username() << endl;
  const long int second = warble.timestamp().seconds();
  cout << "post time: " << std::asctime(std::localtime((&second)));
  cout << "warble text: " << warble.text() << endl << endl;
}
} // namespace command_line_tool

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  command_line_tool::CommandLineTool tool;

  int action = tool.CheckAction(FLAGS_registeruser,
                                FLAGS_user,
                                FLAGS_warble,
                                FLAGS_reply,
                                FLAGS_follow,
                                FLAGS_read,
                                FLAGS_stream,
                                FLAGS_profile,
                                FLAGS_hookall,
                                FLAGS_unhookall);

  if (action == kHookAll) {
    tool.HookAll();
  }

  if (action == kUnhookAll) {
    tool.UnhookAll();
  }

  if (action == kRegisterUser) {
    tool.RegisterUser(FLAGS_registeruser);
  }

  if (action == kWarble || action == kReply) {
    tool.Warble(FLAGS_user, FLAGS_warble, FLAGS_reply);
  }

  if (action == kFollow) {
    tool.Follow(FLAGS_user, FLAGS_follow);
  }

  if (action == kRead) {
    tool.Read(FLAGS_read, FLAGS_user);
  }

  if (action == kProfile) {
    tool.Profile(FLAGS_user);
  }

  if (action == kStream) {
    tool.Stream(FLAGS_stream);
  }

  return 0;
}