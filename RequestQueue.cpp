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
        queue.push(r);
        createdIDs.push_back(reqID);
    }
    return createdIDs;
}

crow::json::wvalue RequestQueue::listPending() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);
    
    std::queue<Request> tempQueue = queue;
    int i = 0;
    while (!tempQueue.empty()) {
        const Request &r = tempQueue.front();
        res["requests"][i]["requestID"] = r.requestID;
        res["requests"][i]["userID"] = r.userID;
        res["requests"][i]["receiverID"] = r.receiverID;
        res["requests"][i]["rideIndex"] = r.rideIndex;
        tempQueue.pop();
        ++i;
    }
    return res;
}

bool RequestQueue::respondToRequest(int requestID, bool accept, std::string &outMessage) {
    std::lock_guard<std::mutex> lock(mtx);
    
    std::queue<Request> tempQueue;
    bool found = false;
    Request targetRequest;
    
    while (!queue.empty()) {
        Request r = queue.front();
        queue.pop();
        
        if (r.requestID == requestID) {
            found = true;
            targetRequest = r;
            if (accept) {
                outMessage = "Request " + std::to_string(requestID) + " accepted: " + r.userID + " connected with " + r.receiverID;
            } else {
                outMessage = "Request " + std::to_string(requestID) + " rejected";
            }
        } else {
            tempQueue.push(r);
        }
    }
    
    queue = tempQueue;
    
    if (!found) {
        outMessage = "Request not found";
        return false;
    }
    
    return accept;
}
