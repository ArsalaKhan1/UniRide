#include "RideSystem.h"

void RideSystem::addRide(const std::string &id, const std::string &from, const std::string &to,
                         const std::string &time, const std::string &mode) {
    std::lock_guard<std::mutex> lock(mtx);
    rides.emplace_back(id, from, to, time, mode);
}

std::vector<Ride> RideSystem::findMatches(const std::string &from, const std::string &to) const {
    std::vector<Ride> matches;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &r : rides) {
        if (r.from == from && r.to == to)
            matches.push_back(r);
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
    }
    return res;
}
