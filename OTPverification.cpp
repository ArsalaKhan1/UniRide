#include "OTPverification.h"
#include <iostream>


    string OTPVerification::generateOTP(int length = 6)
    {
        srand(time(0));
        string otp = "";
        while (!generatedOTP.empty()) // clear old OTP
            generatedOTP.pop();
        for (int i = 0; i < length; i++)
        {
            char digit = '0' + rand() % 10;
            otp += digit;
            generatedOTP.push(digit); // pushes(enqueues) the digits onto the queue
        }
        return otp;
    }

    OTPVerification::OTPVerification(string user_A, string user_B)
    {
        userA_ID = user_A;
        userB_ID = user_B;
        otp_code = generateOTP();
        userA_verified = false;
        userB_verified = false;
        partnerIdentityVerified = false;
    }
    void OTPVerification::initiateVerification()
    {
        cout << "Generated OTP (shared securely with both users): " << otp_code << endl;
    }
    void OTPVerification::takeUserInput(const string &input)
    {
        for (char c : input)
        {
            inputQueue.push(c);
        }
    }
    void OTPVerification::verifyOTP(const string &userID)
    {
        int tries = 0;
        while (tries < 3)
        {
            string input;
            cout << "Enter OTP: " << endl;
            cin >> input;
            takeUserInput(input);
            if (inputQueue == generatedOTP) // if input matches
            {
                if (userID == userA_ID)
                {
                    userA_verified = true;
                    cout << userA_ID << " verified successfully!" << endl;
                }
                else
                {
                    userB_verified = true;
                    cout << userB_ID << " verified successfully!" << endl;
                }
                while (!inputQueue.empty())
                    inputQueue.pop();
                break; // breaks when the OTP matches
            }
            else
            {
                cout << "OTP entered by: " << userID << " is incorrect!" << endl;
            }
            while (!inputQueue.empty())
                inputQueue.pop();
            tries++;
        }
    }
    bool OTPVerification::unlockChat()
    {
        return (userA_verified && userB_verified);
    }
    void OTPVerification::confirmPartnerIdentity(bool userA_option, bool userB_option)
    {
        if (userA_option && userB_option)
        {
            partnerIdentityVerified = true;
            cout << "Partner identity verified in person!" << endl;
            return;
        }
        cout << "Partner identity verification in person failed!" << endl;
    }
    bool OTPVerification::isFullyVerified()
    {
        return unlockChat() && partnerIdentityVerified;
    }