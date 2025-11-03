#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "User.h"
#include "Ride.h"

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;

public:
    DatabaseManager(const std::string& path = "rideshare.db");
    ~DatabaseManager();
    
    bool initialize();
    
    // User operations
    bool insertUser(const User& user);
    User getUserByEmail(const std::string& email);
    User getUserByID(const std::string& userID);
    std::vector<User> getAllUsers();
    
    // Ride operations
    int insertRide(Ride& ride);
    std::vector<Ride> getAllRides();
    std::vector<Ride> findRideMatches(const std::string& from, const std::string& to, RideType rideType, const std::string& userID = "");
    bool updateRideCapacity(const std::string& userID, const std::string& from, const std::string& to, int newCapacity);
    
    // Request operations
    bool insertRequest(const std::string& userID, const std::string& from, const std::string& to, RideType rideType);
    bool updateRequestStatus(int requestID, const std::string& status);
    
    // Message operations
    bool insertMessage(const std::string& senderID, const std::string& messageText);
    std::vector<std::pair<std::string, std::string>> getAllMessages(); // returns (sender, message) pairs
    
    // User preferences operations
    bool updateUserPreferences(const std::string& userID, const std::string& genderPref, int vehicleType);
    bool getUserPreferences(const std::string& userID, std::string& genderPref, int& vehicleType);
    
    // Enhanced ride operations
    std::vector<Ride> findMatchingRides(const std::string& from, const std::string& to, 
                                       RideType rideType, const std::string& userID, 
                                       const std::string& genderPref = "any");
    bool insertJoinRequest(int rideID, const std::string& userID);
    bool updateJoinRequestStatus(int rideID, const std::string& userID, const std::string& status);
    bool updateRideStatus(int rideID, const std::string& status);
    bool updateRideCapacityByID(int rideID, int newCapacity);
    bool isValidEnrollment(const std::string& enrollmentID);
    std::vector<std::pair<std::string, std::string>> getPendingRequests(int rideID); // returns (userID, timestamp) pairs
    bool hasActiveRequest(const std::string& userID);
    bool isValidStudentEmail(const std::string& email);
};

#endif // DATABASEMANAGER_H