#ifndef REQUEST_H
#define REQUEST_H

#include <string>

struct Request {
    int requestID;           // Unique request identifier
    std::string userID;      // ID of the user sending the request
    std::string receiverID;  // ID of the user receiving the request
    int rideIndex;           // Index of the ride being requested

    Request() = default;
    Request(int reqID, const std::string &u, const std::string &r, int idx);
};

#endif // REQUEST_H
