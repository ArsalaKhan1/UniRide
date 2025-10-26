#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#pragma once
#include <vector>
#include <mutex>
#include "Request.h"
#include "RideSystem.h"
#include "crow.h"

class RequestQueue {
private:
    mutable std::mutex mtx;
    std::vector<Request> queue;
    int nextRequestID = 1;
    RideSystem *rideSystem;

public:
    RequestQueue(RideSystem *rs);
    std::vector<int> createRequest(const std::string &userID, const std::string &from, const std::string &to);
    crow::json::wvalue listPending() const;
    bool respondToRequest(int requestID, bool accept, std::string &outMessage);
};
#endif // REQUESTQUEUE_H