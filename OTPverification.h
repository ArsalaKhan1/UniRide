#ifndef OTPVERIFICATION_H
#define OTPVERIFICATION_H

#include <queue>
#include <string>
using namespace std;

class OTPVerification {
private:
    string userA_ID, userB_ID, otp_code;
    bool userA_verified, userB_verified, partnerIdentityVerified;
    queue<char> generatedOTP;
    queue<char> inputQueue;

    string generateOTP(int length);

public:
    OTPVerification(string user_A, string user_B);
    void initiateVerification();
    void takeUserInput(const string &input);
    void verifyOTP(const string &userID);
    bool unlockChat();
    void confirmPartnerIdentity(bool userA_option, bool userB_option);
    bool isFullyVerified();
   
};

#endif