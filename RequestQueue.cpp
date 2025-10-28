#include "RequestQueue.h"
#include <algorithm>

RequestQueue::RequestQueue(RideSystem *rs, DatabaseManager *db) : rideSystem(rs), dbManager(db) {}

std::vector<int> RequestQueue::createRequest(const std::string &userID, const std::string &from, const std::string &to) {
    std::vector<int> createdIDs;
    auto matches = rideSystem->findMatches(from, to);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (matches.empty()) {
        rideSystem->addRide(userID, from, to, "flexible", "any");
        
        Ride newRide(userID, from, to, "flexible", "any");
        dbManager->insertRide(newRide);
        
        return createdIDs; 
    }
    
    for (size_t i = 0; i < matches.size(); ++i) {
        int reqID = nextRequestID++;
        Request r(reqID, userID, matches[i].userID, static_cast<int>(i));
        queue.push_back(r);
        createdIDs.push_back(reqID);
    }
    return createdIDs;
}

crow::json::wvalue RequestQueue::listPending() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);
    int i = 0;
    for (const auto &r : queue) {
        res["requests"][i]["requestID"] = r.requestID;
        res["requests"][i]["userID"] = r.userID;
        res["requests"][i]["receiverID"] = r.receiverID;
        res["requests"][i]["rideIndex"] = r.rideIndex;
        ++i;
    }
    return res;
}

bool RequestQueue::respondToRequest(int requestID, bool accept, std::string &outMessage) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(queue.begin(), queue.end(), [&](const Request &r) {
        return r.requestID == requestID;
    });
    if (it == queue.end()) {
        outMessage = "Request not found";
        return false;
    }
    if (accept) {
        outMessage = "Request " + std::to_string(requestID) + " accepted: " + it->userID + " connected with " + it->receiverID;
        queue.erase(it);
        return true;
    } else {
        outMessage = "Request " + std::to_string(requestID) + " rejected";
        queue.erase(it);
        return false;
    }
}
