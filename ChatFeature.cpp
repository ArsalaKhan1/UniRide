#include "ChatFeature.h"
#include <iostream>
#include <chrono>

ChatFeature::ChatFeature(OTPVerification *otp)
{
    otpSystem = otp;
}
string ChatFeature::getCurrentTime()
{
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    string timeStr = ctime(&t);
    timeStr.pop_back();
    return timeStr;
}
void ChatFeature::AddMessage(const string &sender, const string &text)
{
    bool verified = false;
    if (chat.empty())
    {
        if (otpSystem->unlockChat())
        {
            verified = true;
        }
    }
    if (verified)
    {
        Message msg = {sender, text, getCurrentTime()};
        chat.push_back(msg);
    }
    else
    {
        cout << "OTP VERIFICATION FOR BOTH USERS IS NECESSARY TO CHAT!" << endl;
    }
}
void ChatFeature::DisplayChat() const
{
    if (chat.empty())
    {
        cout << "No messages yet!" << endl;
        return;
    }
    cout << "--- Chat History ---" << endl;
    for (const auto &msg : chat)
    {
        cout << "[" << msg.timestamp << "] " << msg.sender << ": " << msg.text << endl;
    }
    cout << ".............." << endl;
}
void ChatFeature::limitMessages(size_t maxSize)
{ // OPTIONAL: KEEP ONLY A FIXED NUMBER OF MESSAGES
    while (chat.size() > maxSize)
    {
        chat.pop_front();
    }
}