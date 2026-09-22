# Dolly Pump Guard Setup Guide

<h3>Index</h3>

- [1. Create the Telegram Bot](#1-create-the-telegram-bot)
- [2. Get User Chat IDs](#2-get-user-chat-ids)
- [3. Configure dolly.h](#3-configure-dollyh)
- [4. Set Up the Bot Menu Commands](#4-set-up-the-bot-menu-commands)
- [5. Flash and Test](#5-flash-and-test)


## 1. Create the Telegram Bot

1. Open Telegram and search for [@BotFather](https://web.telegram.org/a/#93372553).
2. Tap Start and send the command `/newbot`.
3. Give the bot a display name: `Dolly Pump Guard`.
4. Give the bot a unique username ending in "bot" (e.g., **dolly_pump_guard_bot**).
5. BotFather will reply with an HTTP API Token (a long string like 1234567890:AAHdqT...). **Copy this token**; you will need it for the code.

## 2. Get User Chat IDs

Every person who needs to receive alerts or use the bot must find their personal Telegram Chat ID.<br>

1. Open Telegram and search for @userinfobot or @myidbot.
2. ap Start.
3. The bot will instantly reply with a numeric ID (e.g., 333333333).
4. Save the IDs for each member.

## 3. Configure dolly.h

You only need to edit the dolly.h file to configure your credentials. 

>Do not modify main.cpp for this step.   

Open dolly.h and update the highlighted variables with your Wi-Fi credentials, Bot Token, and Chat IDs:  

```cpp
// 1. Enter the Wi-Fi credentials for the network the pump is connected to
String wifi_ssid = "YOUR_WIFI_NAME";
String wifi_password = "YOUR_WIFI_PASSWORD";

// 2. Paste the HTTP API Token from BotFather
String bot_token = "YOUR_BOT_TOKEN_HERE";

// 3. Update the users array with the exact Chat IDs from IDBot
// Ensure ADMIN and STD roles are assigned correctly
User users[] = {
  {1, "User 1", "PASTE_USER_1_CHAT_ID_HERE", ADMIN},
  {2, "User 2", "PASTE_USER_2_CHAT_ID_HERE", STD},
  {3, "User 3", "PASTE_USER_3_CHAT_ID_HERE", ADMIN}
};
```

## 4. Set Up the Bot Menu Commands

To give users a clickable menu next to the chat bar for commands that do not use the inline buttons:

1. Go back to your chat with @BotFather.
2. Send the command `/setcommands`.
3. Select your bot from the list.
4. Copy and paste the exact block of text below and send it as a single message:

```txt
status - Check the current bio pump water level
notify - (Admin) Broadcast a message to all users
promote - (Admin) Make a user an admin
update_id - (Admin) Update a user's Chat ID
```

## 5. Flash and Test

1. Connect your Wemos D1 board to your computer.
2. Compile and upload the code via the Arduino IDE.
3. Once the board connects to Wi-Fi, open Telegram, search for your bot's username, and send `/start`. The inline buttons and main menu will appear, and the bot will actively monitor the pump's water level.