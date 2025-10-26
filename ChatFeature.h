#ifndef CHATFEATURE_H
#define CHATFEATURE_H
#pragma once
#include <deque>
#include <mutex>
#include <string>
#include "crow.h"
#include "OTPVerification.h"

struct Message {
    std::string sender;
    std::string text;
    std::string timestamp;
};

class ChatFeature {
private:
private:
    std::shared_ptr<OTPVerification> otpSystem;
    std::deque<Message> chat;
    mutable std::mutex mtx;
    std::string getCurrentTime() const;

public:
    ChatFeature(std::shared_ptr<OTPVerification> otp);
    bool AddMessage(const std::string &sender, const std::string &text, std::string &outErr);
    crow::json::wvalue getMessagesJson() const;
    void limitMessages(size_t maxSize);
};
#endif // CHATFEATURE_H