#ifndef RIDESYSTEM_H
#define RIDESYSTEM_H

#pragma once
#include <vector>
#include <string>
#include <mutex>
#include "Ride.h"
#include "crow.h"

class DatabaseManager;
class User;

class RideSystem {
private:
    mutable std::mutex mtx;
    std::vector<Ride> rides;
    DatabaseManager* dbManager = nullptr;

public:
    void addRide(const std::string &id, const std::string &from, const std::string &to,
                 const std::string &time, const std::string &mode, RideType rideType = RideType::CARPOOL, bool femalesOnly = false);
    std::vector<Ride> findMatches(const std::string &from, const std::string &to, RideType rideType, const std::string &userID = "") const;
    std::vector<Ride> getAllRides() const;
    crow::json::wvalue getAllRidesJson() const;
    bool joinRide(const std::string &userID, const std::string &from, const std::string &to, RideType rideType);
    void setDatabaseManager(DatabaseManager* db) { dbManager = db; }
};
#endif // RIDESYSTEM_H