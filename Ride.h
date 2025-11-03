#ifndef RIDE_H
#define RIDE_H

#include <string>
#include <vector>

enum class RideType {
    BIKE,      // 2 people max (owner + 1 passenger)
    CARPOOL,   // 5 people max (owner + 4 passengers) or 4 participants
    RICKSHAW   // 3 participants max (no owner)
};

enum class RideStatus {
    OPEN,      // Accepting requests
    FULL,      // Capacity reached
    STARTED,   // Ride in progress
    COMPLETED  // Ride finished
};

enum class RequestStatus {
    PENDING,
    ACCEPTED,
    REJECTED
};

struct JoinRequest {
    std::string userID;
    RequestStatus status;
    std::string timestamp;
    
    JoinRequest(const std::string& uid) : userID(uid), status(RequestStatus::PENDING) {}
};

struct Ride {
    int rideID;             // Unique ride identifier
    std::string ownerID;    // ID of ride owner (empty for rickshaw)
    std::string from;       // Starting point
    std::string to;         // Destination
    std::string time;       // Time of the ride
    std::string mode;       // Mode of transport
    RideType rideType;      // Type of ride
    RideStatus status;      // Current ride status
    int currentCapacity;    // Current number of people
    int maxCapacity;        // Maximum capacity
    std::vector<std::string> participants; // Confirmed participants
    std::vector<JoinRequest> pendingRequests; // Pending join requests
    bool femalesOnly;       // Gender preference
    std::string genderPreference; // "male", "female", "any"

    // Legacy field for compatibility
    std::string userID;     // Same as ownerID

    Ride() = default;
    Ride(const std::string &id, const std::string &f, const std::string &t,
         const std::string &tm, const std::string &m, RideType rt = RideType::CARPOOL, bool fo = false);
    
    bool canAcceptMoreParticipants() const;
    int getAvailableSlots() const;
    bool hasPendingRequest(const std::string& userID) const;
    void addJoinRequest(const std::string& userID);
    bool approveRequest(const std::string& userID);
    bool rejectRequest(const std::string& userID);
    void updateStatus();
};

#endif // RIDE_H
