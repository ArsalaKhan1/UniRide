#include "OTPVerification.h"
#include <random>
#include <chrono>

// Utility function to generate a random numeric OTP of given length
static std::string genOTP(int length) {
    std::string s;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    for (int i = 0; i < length; ++i)
        s += char('0' + dis(gen));
    return s;
}

// Constructor
OTPVerification::OTPVerification(const std::string &userA, const std::string &userB)
    : userA_ID(userA), userB_ID(userB) {
    otp_code = generateOTP(6);
    userA_verified = false;
    userB_verified = false;
    partnerIdentityVerified = false;
}

// Generate a new OTP code
std::string OTPVerification::generateOTP(int length) {
    return genOTP(length);
}

// Start a new verification session (reset state and issue OTP)
std::string OTPVerification::initiateVerification() {
    std::lock_guard<std::mutex> lock(mtx);
    otp_code = generateOTP(6);
    userA_verified = false;
    userB_verified = false;
    partnerIdentityVerified = false;

    // In production, you'd send this OTP to the users.
    // For development/testing, we return it directly.
    return otp_code;
}

// Verify the OTP entered by a given user
bool OTPVerification::verifyOTPInput(const std::string &userID, const std::string &input) {
    std::lock_guard<std::mutex> lock(mtx);
    if (input == otp_code) {
        if (userID == userA_ID)
            userA_verified = true;
        else if (userID == userB_ID)
            userB_verified = true;
        return true;
    }
    return false;
}

// Confirm whether both users acknowledged each other’s identity
void OTPVerification::confirmPartnerIdentity(bool userA_option, bool userB_option) {
    std::lock_guard<std::mutex> lock(mtx);
    partnerIdentityVerified = (userA_option && userB_option);
}

// Check if both users have verified their OTPs
bool OTPVerification::unlockChat() const {
    std::lock_guard<std::mutex> lock(mtx);
    return (userA_verified && userB_verified);
}

// Check if both OTPs and identity confirmation are complete
bool OTPVerification::isFullyVerified() const {
    std::lock_guard<std::mutex> lock(mtx);
    return unlockChat() && partnerIdentityVerified;
}

// Return current verification status as JSON
crow::json::wvalue OTPVerification::statusJson() const {
    std::lock_guard<std::mutex> lock(mtx);
    crow::json::wvalue res;
    res["userA_ID"] = userA_ID;
    res["userB_ID"] = userB_ID;
    res["userA_verified"] = userA_verified;
    res["userB_verified"] = userB_verified;
    res["partnerIdentityVerified"] = partnerIdentityVerified;

    // NOTE: otp_code intentionally not included for security
    return res;
}
