#ifndef CHATFEATURE_H
#define CHATFEATURE_H
#pragma once
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include "crow.h"

struct Message {
    std::string sender;
    std::string recipient;
    std::string text;
    std::string timestamp;
    int rideID;
};

class ChatFeature {
private:
    std::unordered_map<int, std::deque<Message>> rideChats; // rideID -> messages
    std::unordered_map<int, std::string> rideLeads; // rideID -> leadUserID
    mutable std::mutex mtx;
    std::string getCurrentTime() const;

public:
    ChatFeature();
    bool AddMessage(const std::string &sender, const std::string &recipient, const std::string &text, int rideID, std::string &outErr);
    bool SetRideLead(int rideID, const std::string &leadUserID);
    crow::json::wvalue getRideMessagesJson(int rideID) const;
    crow::json::wvalue getMessagesJson() const; // Legacy support
    void limitMessages(size_t maxSize);
};
#endif // CHATFEATURE_H