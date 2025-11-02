#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#pragma once
#include <queue>
#include <mutex>
#include "Request.h"
#include "RideSystem.h"
#include "DatabaseManager.h"
#include "crow.h"

class RequestQueue {
private:
    mutable std::mutex mtx;
    std::queue<Request> queue;
    int nextRequestID = 1;
    RideSystem *rideSystem;
    DatabaseManager *dbManager;

public:
    RequestQueue(RideSystem *rs, DatabaseManager *db);
    std::vector<int> createRequest(const std::string &userID, const std::string &from, const std::string &to, RideType rideType = RideType::CARPOOL);
    std::vector<int> createRideOffer(const std::string &userID, const std::string &from, const std::string &to, RideType rideType, bool femalesOnly = false);
    crow::json::wvalue listPending() const;
    bool respondToRequest(int requestID, bool accept, std::string &outMessage);
};
#endif // REQUESTQUEUE_H