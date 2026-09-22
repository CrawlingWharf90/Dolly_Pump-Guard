#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "dolly.h"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

const unsigned long BOT_MTBS = 1000;
unsigned long bot_lasttime = 0;

const int alarmPin = D1;
int lastAlarmState = HIGH;

//* --- Helper Functions ---
int get_user_index(int id) {
  for (int i = 0; i < numUsers; i++) {
    if (users[i].id == id) return i;
  }
  return -1;
}

int get_id_from_chatId(String chatId) {
  for (int i = 0; i < numUsers; i++) {
    if (users[i].chatId == chatId) return users[i].id;
  }
  return -1;
}

bool isAdmin(int requester_id) {
  int index = get_user_index(requester_id);
  return (index != -1 && users[index].role == ADMIN);
}

//* --- Dynamic Keyboard Function ---
void send_message_with_buttons(String chatId, String text, int target_user_id, String context) {
  bool admin = isAdmin(target_user_id);
  
  String kbd = "{\"inline_keyboard\":[";
  bool firstRow = true;

  if (context != "/status") {
    kbd += "[{\"text\":\"📊 Pump Status\",\"callback_data\":\"/status\"}]";
    firstRow = false;
  }
  
  if (context != "/start") {
    if (!firstRow) kbd += ",";
    kbd += "[{\"text\":\"🏠 Main Menu\",\"callback_data\":\"/start\"}]";
    firstRow = false;
  }
  
  if (admin) {
    String adminRow = "";
    if (context != "/notify_help") {
      adminRow += "{\"text\":\"📢 Send Notification\",\"callback_data\":\"/notify_help\"}";
    }
    if (context != "/promote_help") {
      if (adminRow.length() > 0) adminRow += ",";
      adminRow += "{\"text\":\"👑 Manage Users\",\"callback_data\":\"/promote_help\"}";
    }
    
    if (adminRow.length() > 0) {
      if (!firstRow) kbd += ",";
      kbd += "[" + adminRow + "]";
    }
  }
  
  kbd += "]}";

  bot.sendMessageWithInlineKeyboard(chatId, text, "", kbd);
}

//* --- Persistence ---
void save_config() {
  File file = LittleFS.open("/config.json", "w");
  if (!file) return;
  
  DynamicJsonDocument doc(1024);
  doc["wifi_ssid"] = wifi_ssid;
  doc["wifi_password"] = wifi_password;
  
  JsonArray usersArr = doc.createNestedArray("users");
  for (int i = 0; i < numUsers; i++) {
    JsonObject u = usersArr.createNestedObject();
    u["id"] = users[i].id;
    u["name"] = users[i].name;
    u["chatId"] = users[i].chatId;
    u["role"] = (int)users[i].role;
  }
  
  serializeJson(doc, file);
  file.close();
}

void load_config() {
  File file = LittleFS.open("/config.json", "r");
  if (!file) return;

  DynamicJsonDocument doc(1024);
  if (!deserializeJson(doc, file)) {
    wifi_ssid = doc["wifi_ssid"].as<String>();
    wifi_password = doc["wifi_password"].as<String>();
    
    JsonArray arr = doc["users"].as<JsonArray>();
    numUsers = arr.size();
    for (int i = 0; i < numUsers; i++) {
      users[i].id = arr[i]["id"];
      users[i].name = arr[i]["name"].as<String>();
      users[i].chatId = arr[i]["chatId"].as<String>();
      users[i].role = (UserType)arr[i]["role"].as<int>();
    }
  }
  file.close();
}

//* --- Admin & Standard Functions ---
void notify(String text, int requester_id) {
  if (!isAdmin(requester_id)) return;
  int sender_index = get_user_index(requester_id);
  
  String full_message = "📢 " + users[sender_index].name + ":\n" + text;
  for (int i = 0; i < numUsers; i++) {
    if (users[i].id != requester_id && users[i].chatId != "") {
      bot.sendMessage(users[i].chatId, full_message, "");
    }
  }
  send_message_with_buttons(users[sender_index].chatId, "Notification sent.", requester_id, "/start");
}

void get_status(int requester_id) {
  int index = get_user_index(requester_id);
  if (index == -1) return;
  
  String msg = (lastAlarmState == LOW) ? "🚨 ALARM ACTIVE: Water level is high!" : "🟢 Status OK: Pump system is normal.";
  send_message_with_buttons(users[index].chatId, msg, requester_id, "/status");
}

void promote_user(int target_id, int requester_id) {
  int req_index = get_user_index(requester_id);
  if (!isAdmin(requester_id)) {
    bot.sendMessage(users[req_index].chatId, "Access Denied.", "");
    return;
  }
  
  int index = get_user_index(target_id);
  if (index != -1) {
    users[index].role = ADMIN;
    save_config();
    send_message_with_buttons(users[req_index].chatId, users[index].name + " is now an ADMIN.", requester_id, "/start");
  }
}

void update_chat_id(int target_id, String new_chat_id, int requester_id) {
  int req_index = get_user_index(requester_id);
  if (isAdmin(requester_id)) {
    int index = get_user_index(target_id);
    if (index != -1) {
      users[index].chatId = new_chat_id;
      save_config();
      send_message_with_buttons(users[req_index].chatId, "Chat ID updated for " + users[index].name, requester_id, "/start");
    }
  }
}

//* --- Telegram Polling & Parsing ---
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chatId = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    
    // Intercept button clicks (callback queries)
    if (bot.messages[i].type == "callback_query") {
      // The button payload acts as our command text
      // Telegram handles the loading spinner stop automatically for standard HTTP polling
    }
    
    int requester_id = get_id_from_chatId(chatId);
    
    if (requester_id == -1) {
      bot.sendMessage(chatId, "Unauthorized user. Your ID is: " + chatId, "");
      continue;
    }
    
    // Command Routing
    if (text == "/start") {
      send_message_with_buttons(chatId, "👋 Welcome to Dolly Pump Guard!\nUse the buttons below.", requester_id, "/start");
    }
    else if (text == "/status") {
      get_status(requester_id);
    } 
    else if (text == "/notify_help") {
      send_message_with_buttons(chatId, "To broadcast a message, type:\n/notify [your message]", requester_id, "/notify_help");
    }
    else if (text == "/promote_help") {
      send_message_with_buttons(chatId, "To promote a user, type:\n/promote [user_id]", requester_id, "/promote_help");
    }
    else if (text.startsWith("/notify ")) {
      notify(text.substring(8), requester_id);
    }
    else if (text.startsWith("/promote ")) {
      promote_user(text.substring(9).toInt(), requester_id);
    }
    else if (text.startsWith("/update_id ")) {
      int spaceIndex = text.indexOf(' ', 11);
      if (spaceIndex > 0) {
        int target = text.substring(11, spaceIndex).toInt();
        String new_id = text.substring(spaceIndex + 1);
        update_chat_id(target, new_id, requester_id);
      }
    }
    else {
      send_message_with_buttons(chatId, "Command not recognized.", requester_id, "");
    }
  }
}

void checkPumpAlarm() {
  int currentState = digitalRead(alarmPin);
  if (currentState != lastAlarmState) {
    delay(50); // Debounce
    if (digitalRead(alarmPin) == currentState) {
      lastAlarmState = currentState;
      if (currentState == LOW) {
        for (int i = 0; i < numUsers; i++) {
          if (users[i].chatId != "") {
             send_message_with_buttons(users[i].chatId, "🚨 CRITICAL: Bio Pump Water Level High!", users[i].id, "/status");
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(alarmPin, INPUT_PULLUP);
  
  if (LittleFS.begin()) {
    load_config();
  }

  secured_client.setInsecure(); // Required for ESP8266 SSL bypass
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  
  checkPumpAlarm();
}