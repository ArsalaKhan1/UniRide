#include <iostream>
#include "User.h"
#include "RideSystem.h"
#include "Request.h"
#include "RequestQueue.h"
#include "OTPverification.h"
#include "ChatFeature.h"
#include "AuthSys.h"
using namespace std;

int main()
{
    srand(time(0)); // for OTP generation randomness

    // Step 1: Registration
    AuthSystem auth;
    cout << "=== User Registration ===" << endl;
    User u1 = auth.registerUser("U101", "Ali", "ali@university.edu");
    User u2 = auth.registerUser("U102", "Sara", "sara@university.edu");
    auth.displayUsers();

    cout << "\n=== Login Phase ===" << endl;
    string loginEmail;
    cout << "Enter email to login: ";
    cin >> loginEmail;
    if (!auth.loginUser(loginEmail)) {
        cout << "Exiting system..." << endl;
        return 0;
    }

    cout << "\n=== Login Phase ===" << endl;
    cout << "Enter email to login: ";
    cin >> loginEmail;
    if (!auth.loginUser(loginEmail)) {
        cout << "Exiting system..." << endl;
        return 0;
    }

    // Step 2: Ride posting
    RideSystem rideSys;
    rideSys.addRide(u1.userID, "Campus", "Downtown", "3 PM", "Car");
    rideSys.addRide(u2.userID, "Campus", "Airport", "4 PM", "Taxi");

    cout << "\n=== All Available Rides ===" << endl;
    rideSys.viewRides();

    // Step 3: Create ride request (Sara looking for Campus → Downtown)
    RequestQueue reqQ(&rideSys);
    reqQ.createRequest(u2.userID, "Campus", "Downtown");

    // Step 4: Process request (Ali accepts or rejects Sara’s request)
    reqQ.processRequests();

    // Step 5: OTP Verification between Ali and Sara
    OTPVerification otpSys(u1.userID, u2.userID);
    otpSys.initiateVerification();

    cout << "\n=== Ali Verification ===" << endl;
    otpSys.verifyOTP(u1.userID);

    cout << "\n=== Sara Verification ===" << endl;
    otpSys.verifyOTP(u2.userID);

    // Step 6: Confirm in-person identity
    otpSys.confirmPartnerIdentity(true, true);

    if (otpSys.isFullyVerified())
    {
        cout << "\n Both users fully verified. Chat unlocked!\n";
    }
    else
    {
        cout << "\n Verification failed. No chat allowed.\n";
    }

    // Step 7: Chat Feature
    ChatFeature chat(&otpSys);

    cout << "\n=== Chat Demo ===" << endl;
    chat.AddMessage(u1.userID, "Hey Sara, let \' s meet near the main gate.");
    chat.AddMessage(u2.userID, "Sure Ali, see you at 3 PM!");
    chat.DisplayChat();

    // Optional: limit messages (e.g., keep last 5 only)
    chat.limitMessages(5);

    return 0;
}
