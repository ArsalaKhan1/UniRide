#include "RideSystem.h"
#include "DatabaseManager.h"
#include "User.h"

void RideSystem::addRide(const std::string &id, const std::string &from, const std::string &to,
                         const std::string &time, const std::string &mode, RideType rideType, bool femalesOnly) {
    std::lock_guard<std::mutex> lock(mtx);
    rides.emplace_back(id, from, to, time, mode, rideType, femalesOnly);
}

std::vector<Ride> RideSystem::findMatches(const std::string &from, const std::string &to, RideType rideType, const std::string &userID) const {
    std::vector<Ride> matches;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &r : rides) {
        if (r.from == from && r.to == to && r.rideType == rideType && r.canAcceptMoreParticipants()) {
            // Filter out females-only rides for non-females
            if (r.femalesOnly && !userID.empty() && dbManager) {
                User user = dbManager->getUserByID(userID);
                if (user.gender != "female") {
                    continue; // Skip this ride
                }
            }
            matches.push_back(r);
        }
    }
    return matches;
}

std::vector<Ride> RideSystem::getAllRides() const {
    std::lock_guard<std::mutex> lock(mtx);
    return rides;
}

crow::json::wvalue RideSystem::getAllRidesJson() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);
    for (size_t i = 0; i < rides.size(); ++i) {
        res["rides"][i]["userID"] = rides[i].userID;
        res["rides"][i]["from"] = rides[i].from;
        res["rides"][i]["to"] = rides[i].to;
        res["rides"][i]["time"] = rides[i].time;
        res["rides"][i]["mode"] = rides[i].mode;
        res["rides"][i]["rideType"] = static_cast<int>(rides[i].rideType);
        res["rides"][i]["currentCapacity"] = rides[i].currentCapacity;
        res["rides"][i]["maxCapacity"] = rides[i].maxCapacity;
        res["rides"][i]["availableSlots"] = rides[i].getAvailableSlots();
    }
    return res;
}

bool RideSystem::joinRide(const std::string &userID, const std::string &from, const std::string &to, RideType rideType) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &r : rides) {
        if (r.from == from && r.to == to && r.rideType == rideType && r.canAcceptMoreParticipants()) {
            // Check gender restriction for females-only rides
            if (r.femalesOnly && dbManager) {
                User user = dbManager->getUserByID(userID);
                if (user.gender != "female") {
                    return false; // Reject non-females from females-only rides
                }
            }
            r.participants.push_back(userID);
            r.currentCapacity++;
            return true;
        }
    }
    return false;
}
