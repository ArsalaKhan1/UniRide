#include "RequestQueue.h"
#include <algorithm>

RequestQueue::RequestQueue(RideSystem *rs, DatabaseManager *db) : rideSystem(rs), dbManager(db) {}

std::vector<int> RequestQueue::createRequest(const std::string &userID, const std::string &from, const std::string &to, RideType rideType) {
    std::vector<int> createdIDs;
    auto matches = rideSystem->findMatches(from, to, rideType, userID);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (matches.empty()) {
        // Create new ride based on type
        std::string mode = (rideType == RideType::BIKE) ? "bike" : 
                          (rideType == RideType::CARPOOL) ? "car" : "rickshaw";
        
        // For rickshaw, first participant creates the group
        std::string ownerID = userID;
        
        rideSystem->addRide(ownerID, from, to, "flexible", mode, rideType);
        
        Ride newRide(ownerID, from, to, "flexible", mode, rideType);
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

std::vector<int> RequestQueue::createRideOffer(const std::string &userID, const std::string &from, const std::string &to, RideType rideType, bool femalesOnly) {
    std::vector<int> createdIDs;
    std::lock_guard<std::mutex> lock(mtx);
    
    std::string mode = (rideType == RideType::BIKE) ? "bike" : "car";
    
    // Only bike and carpool can have owners
    if (rideType != RideType::RICKSHAW) {
        rideSystem->addRide(userID, from, to, "flexible", mode, rideType, femalesOnly);
        
        Ride newRide(userID, from, to, "flexible", mode, rideType, femalesOnly);
        dbManager->insertRide(newRide);
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
