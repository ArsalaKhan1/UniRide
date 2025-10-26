#ifndef OTPVERIFICATION_H
#define OTPVERIFICATION_H

#pragma once
#include <string>
#include <mutex>
#include "crow.h"

class OTPVerification {
private:
    std::string userA_ID;
    std::string userB_ID;
    std::string otp_code;
    bool userA_verified;
    bool userB_verified;
    bool partnerIdentityVerified;
    mutable std::mutex mtx;

    std::string generateOTP(int length);

public:
    // ✅ Constructor
    OTPVerification(const std::string &userA, const std::string &userB);

    // ✅ Make non-copyable (fixes your error)
    OTPVerification(const OTPVerification&) = delete;
    OTPVerification& operator=(const OTPVerification&) = delete;

    // ✅ Allow move semantics (useful for std::unique_ptr or containers)
    OTPVerification(OTPVerification&&) = default;
    OTPVerification& operator=(OTPVerification&&) = default;

    // ✅ Core methods
    std::string initiateVerification();
    bool verifyOTPInput(const std::string &userID, const std::string &input);
    void confirmPartnerIdentity(bool userA_option, bool userB_option);
    bool unlockChat() const;
    bool isFullyVerified() const;
    crow::json::wvalue statusJson() const;
};

#endif // OTPVERIFICATION_H