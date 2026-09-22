#pragma once
#include <Arduino.h>

enum UserType {
  STD,
  ADMIN
};

struct User {
  int id;
  String name;
  String chatId;
  UserType role;
};

//? Can be defined by ADMIN via bot commands
String wifi_ssid = "";
String wifi_password = "";
String bot_token = ""; //! MUST BE INSERTED MANUALLY WITH INITIAL USERS.

//! CRITICAL: Populate this array with chat IDs of authorized users.
//? Admins will be able to add/remove users via bot commands.
User users[] = 
{
  //* --- Example User ---
  //* {ID(int), "USER_NAME", "USER_CHAT_ID", ADMIN or STD}
};

int numUsers = sizeof(users) / sizeof(users[0]);