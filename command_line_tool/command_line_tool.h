#ifndef CSCI499_XIAYICHE_COMMAND_LINE_TOOL_COMMANDLINETOOL_H_
#define CSCI499_XIAYICHE_COMMAND_LINE_TOOL_COMMANDLINETOOL_H_

#include <string>
#include <unordered_map>

#include "../protos/warble.pb.h"
#include "func_client.h"
#include <gflags/gflags.h>

using std::string;
using std::cout;
using std::endl;
using std::unordered_map;
using warble::RegisteruserRequest;
using warble::RegisteruserReply;
using warble::WarbleRequest;
using warble::WarbleReply;
using warble::FollowRequest;
using warble::FollowReply;
using warble::ReadRequest;
using warble::ReadReply;
using warble::ProfileRequest;
using warble::ProfileReply;
using warble::StreamRequest;
using warble::StreamReply;
using func::EventRequest;
using func::EventReply;
using command_line_tool::FuncClient;

DEFINE_string(registeruser, "", "Register a user");
DEFINE_string(user, "", "Log in as user");
DEFINE_string(warble, "", "Create a warble");
DEFINE_string(reply, "", "Reply a warble");
DEFINE_string(follow, "", "Follow a user");
DEFINE_string(read, "", "Read a warble thread");
DEFINE_string(stream, "", "Stream a hashtag");
DEFINE_bool(profile, false, "Get user's profile");
DEFINE_bool(hookall, false, "Hook all functions");
DEFINE_bool(unhookall, false, "Unhook all functions");

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

namespace command_line_tool {
// Command line tool for client side
class CommandLineTool {
 public:
  // Constructor
  CommandLineTool();

  // function to resister a user
  void RegisterUser(string username);

  // function to log in
  string LogIn(string username);

  // function to post or reply a warble
  void Warble(string username, string text, string parent_id = "-1");

  // function to follow a user
  void Follow(string username, string following_username);

  // function to read a warble
  void Read(string warble_id, string username);

  // function to show a user's profile
  void Profile(string username);

  void Stream(string hashtag);

  // function to check what action to do based on flags
  int CheckAction(string registeruser,
                  string user,
                  string warble,
                  string reply,
                  string follow,
                  string read,
                  string stream,
                  bool profile,
                  bool hook,
                  bool unhook);

  // function to hook all functions
  void HookAll();

  // function to unhook all functions
  void UnhookAll();

 private:
  // FuncClient to use Func Server
  FuncClient func_client_;

  // map to store func id mapping to func name
  unordered_map<int, string> func_map_;

  // Print a warble
  void PrintWarble(warble::Warble warble);
};
} // namespace command_line_tool

#endif //CSCI499_XIAYICHE_COMMAND_LINE_TOOL_COMMANDLINETOOL_H_
