#include "ChatFeature.h"
#include <chrono>
#include <ctime>

ChatFeature::ChatFeature() {}

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

// --- Set ride lead (hub) ---
bool ChatFeature::SetRideLead(int rideID, const std::string &leadUserID) {
    std::lock_guard<std::mutex> lock(mtx);
    rideLeads[rideID] = leadUserID;
    return true;
}

// --- Add a chat message (hub-and-spoke model) ---
bool ChatFeature::AddMessage(const std::string &sender, const std::string &recipient,
                             const std::string &text, int rideID, std::string &outErr) {
    std::lock_guard<std::mutex> lock(mtx);

    // Check if ride has a lead
    if (rideLeads.find(rideID) == rideLeads.end()) {
        outErr = "No lead assigned for this ride";
        return false;
    }

    std::string leadUserID = rideLeads[rideID];
    
    // Hub-and-spoke validation: messages must involve the lead
    if (sender != leadUserID && recipient != leadUserID) {
        outErr = "Messages must be sent to or from the ride lead";
        return false;
    }

    Message m{sender, recipient, text, getCurrentTime(), rideID};
    rideChats[rideID].push_back(m);
    return true;
}

// --- Get messages for a specific ride ---
crow::json::wvalue ChatFeature::getRideMessagesJson(int rideID) const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);

    auto it = rideChats.find(rideID);
    if (it == rideChats.end()) {
        res["messages"] = crow::json::wvalue::list();
        return res;
    }

    int i = 0;
    for (const auto &m : it->second) {
        res["messages"][i]["sender"] = m.sender;
        res["messages"][i]["recipient"] = m.recipient;
        res["messages"][i]["text"] = m.text;
        res["messages"][i]["timestamp"] = m.timestamp;
        ++i;
    }

    return res;
}

// --- Legacy support: Convert all chat messages to JSON ---
crow::json::wvalue ChatFeature::getMessagesJson() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);

    int i = 0;
    for (const auto &ridePair : rideChats) {
        for (const auto &m : ridePair.second) {
            res["messages"][i]["sender"] = m.sender;
            res["messages"][i]["recipient"] = m.recipient;
            res["messages"][i]["text"] = m.text;
            res["messages"][i]["timestamp"] = m.timestamp;
            res["messages"][i]["rideID"] = ridePair.first;
            ++i;
        }
    }

    return res;
}

// --- Limit the number of stored messages ---
void ChatFeature::limitMessages(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &ridePair : rideChats) {
        while (ridePair.second.size() > maxSize)
            ridePair.second.pop_front();
    }
}
