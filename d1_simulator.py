
#* CREATED FOR SIMULATION PURPOSES ONLY. 
#* SINCE I DIDN'T HAVE A D1 ON HAND

import time
import requests


BOT_TOKEN = "" #! ADD BOT TOKEN HERE IF YOU WANT TO SIMULATE
BASE_URL = f"https://api.telegram.org/bot{BOT_TOKEN}"

users = {
    #*MADE UP USERS FOR SIMULATION
    #* "CHAT_ID": {"name": "YOUR_NAME", "role": "ADMIN or STD"}
}

last_update_id = 0

def send_message(chat_id, text, context="/start"):
    url = f"{BASE_URL}/sendMessage"
    
    keyboard_buttons = []
    
    if context != "/status":
        keyboard_buttons.append([{"text": "📊 Check Pump Status", "callback_data": "/status"}])
        
    if context != "/start":
        keyboard_buttons.append([{"text": "🏠 Main Menu", "callback_data": "/start"}])
        
    if is_admin(chat_id):
        admin_row = []
        if context != "/notify_help":
            admin_row.append({"text": "📢 Send Notification", "callback_data": "/notify_help"})
        if context != "/promote_help":
            admin_row.append({"text": "👑 Manage Users", "callback_data": "/promote_help"})
            
        if admin_row:
            keyboard_buttons.append(admin_row)
            
    payload = {
        "chat_id": chat_id, 
        "text": text,
        "reply_markup": {"inline_keyboard": keyboard_buttons}
    }
    
    try:
        requests.post(url, json=payload, timeout=10)
    except Exception as e:
        print(f"Failed to send message: {e}")

def is_admin(chat_id):
    chat_id = str(chat_id)
    return chat_id in users and users[chat_id]["role"] == "ADMIN"

def handle_message(chat_id, text):
    chat_id = str(chat_id)
    text = text.strip()

    if chat_id not in users:
        send_message(chat_id, f"Unauthorized user. Your ID is: {chat_id}")
        return

    sender_name = users[chat_id]["name"]
    print(f"[{sender_name}] Triggered: {text}")

    if text == "/start":
        welcome_text = (
            f"👋 Welcome to Dolly Pump Guard, {sender_name}!\n\n"
            "Use the buttons below to interact with the system, or type commands manually."
        )
        send_message(chat_id, welcome_text)

    elif text == "/status":
        send_message(chat_id, "🟢 Status OK: Pump system is normal (Simulator).")
        
    elif text.startswith("/notify "):
        if is_admin(chat_id):
            msg = text[8:]
            full_msg = f"📢 {sender_name}:\n{msg}"
            for uid in users:
                if uid != chat_id:
                    send_message(uid, full_msg)
            send_message(chat_id, "Notification sent.")
        else:
            send_message(chat_id, "Access Denied.")
            
    elif text.startswith("/promote "):
        if is_admin(chat_id):
            target_id = text[9:].strip()
            if target_id in users:
                users[target_id]["role"] = "ADMIN"
                send_message(chat_id, f"User {users[target_id]['name']} is now an ADMIN.")
            else:
                send_message(chat_id, "User ID not found in database.")
        else:
            send_message(chat_id, "Access Denied.")

    elif text == "/notify_help":
        send_message(chat_id, "To broadcast a message, type:\n`/notify Your message here`", context="/notify_help")
        
    elif text == "/promote_help":
        user_list = "\n".join([f"ID: {uid} - {data['name']} ({data['role']})" for uid, data in users.items()])
        send_message(chat_id, f"To promote a user, type `/promote [ID]`.\n\nCurrent Users:\n{user_list}", context="/promote_help")
    
    else:
        send_message(chat_id, "Command not recognized.")

print("Dolly Pump Guard Simulator started! Press Ctrl+C to stop.")

while True:
    try:
        url = f"{BASE_URL}/getUpdates?offset={last_update_id}&timeout=25"
        response = requests.get(url, timeout=30).json()

        if response.get("ok"):
            for update in response.get("result", []):
                last_update_id = update["update_id"] + 1
                
                if "message" in update and "text" in update["message"]:
                    chat_id = update["message"]["chat"]["id"]
                    text = update["message"]["text"]
                    handle_message(chat_id, text)
                    
                elif "callback_query" in update:
                    cb_query = update["callback_query"]
                    chat_id = cb_query["message"]["chat"]["id"]
                    command_data = cb_query["data"] 
                    query_id = cb_query["id"]
                    
                    requests.post(f"{BASE_URL}/answerCallbackQuery", json={"callback_query_id": query_id})
                    
                    handle_message(chat_id, command_data)
                    
    except requests.exceptions.RequestException:
        print("Network error, retrying in 5 seconds...")
        time.sleep(5)
    except KeyboardInterrupt:
        print("\nSimulator stopped.")
        break