#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "User.h"
#include "Ride.h"
#include "LocationGraph.h"

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    LocationGraph* locationGraph;

public:
    DatabaseManager(const std::string& path = "rideshare.db");
    ~DatabaseManager();
    
    void setLocationGraph(LocationGraph* graph) { locationGraph = graph; }
    
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
    // Verify that a given enrollment ID maps to the provided student email (stored in students table)
    bool doesEnrollmentMatchEmail(const std::string& enrollmentID, const std::string& email);
    std::vector<std::pair<std::string, std::string>> getPendingRequests(int rideID); // returns (userID, timestamp) pairs
    bool hasActiveRequest(const std::string& userID);
    bool isValidStudentEmail(const std::string& email);
    
    // Get accepted requests for a user (for notifications)
    std::vector<std::pair<int, std::string>> getAcceptedRequestsForUser(const std::string& userID); // returns (rideID, leadUserID) pairs
    
    // Get accepted passengers for a ride (for ride leads)
    std::vector<std::pair<std::string, std::string>> getAcceptedPassengers(int rideID); // returns (userID, userName) pairs
};

#endif // DATABASEMANAGER_H