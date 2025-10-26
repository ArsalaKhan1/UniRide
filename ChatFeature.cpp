#include "ChatFeature.h"
#include <chrono>
#include <ctime>

ChatFeature::ChatFeature(std::shared_ptr<OTPVerification> otp)
    : otpSystem(std::move(otp)) {}

// --- Get current timestamp ---
std::string ChatFeature::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::string s = std::ctime(&t);

    // Remove trailing newline if present
    if (!s.empty() && s.back() == '\n')
        s.pop_back();

    return s;
}

// --- Add a chat message ---
bool ChatFeature::AddMessage(const std::string &sender,
                             const std::string &text,
                             std::string &outErr) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!otpSystem || !otpSystem->unlockChat()) {
        outErr = "OTP verification required for both users to chat";
        return false;
    }

    Message m{sender, text, getCurrentTime()};
    chat.push_back(m);
    return true;
}

// --- Convert chat messages to JSON ---
crow::json::wvalue ChatFeature::getMessagesJson() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);

    int i = 0;
    for (const auto &m : chat) {
        res["messages"][i]["sender"] = m.sender;
        res["messages"][i]["text"] = m.text;
        res["messages"][i]["timestamp"] = m.timestamp;
        ++i;
    }

    return res;
}

// --- Limit the number of stored messages ---
void ChatFeature::limitMessages(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mtx);
    while (chat.size() > maxSize)
        chat.pop_front();
}
