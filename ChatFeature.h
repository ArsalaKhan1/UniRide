#ifndef CHATFEATURE_H
#define CHATFEATURE_H

#include <deque>
#include "OTPverification.h"
#include "Message.h"
using namespace std;

class ChatFeature
{
private:
    deque<Message> chat;
    OTPVerification *otpSystem;
public:
    ChatFeature(OTPVerification *otp);
    string getCurrentTime();
    void AddMessage(const string &sender, const string &text);
    void DisplayChat() const;
    void limitMessages(size_t maxSize);

};

#endif
