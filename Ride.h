#ifndef RIDE_H
#define RIDE_H

#include <string>
#include <vector>

enum class RideType {
    BIKE,      // 2 people max (owner + 1 passenger)
    CARPOOL,   // 5 people max (owner + 4 passengers) or 4 participants
    RICKSHAW   // 3 participants max (no owner)
};

struct Ride {
    std::string userID;     // ID of the user offering or taking the ride
    std::string from;       // Starting point
    std::string to;         // Destination
    std::string time;       // Time of the ride
    std::string mode;       // Mode of transport (car, bike, etc.)
    RideType rideType;      // Type of ride
    int currentCapacity;    // Current number of people
    int maxCapacity;        // Maximum capacity based on ride type
    std::vector<std::string> participants; // List of participant IDs
    bool femalesOnly;       // Optional preference for females only

    Ride() = default;
    Ride(const std::string &id, const std::string &f, const std::string &t,
         const std::string &tm, const std::string &m, RideType rt = RideType::CARPOOL, bool fo = false);
    
    bool canAcceptMoreParticipants() const;
    int getAvailableSlots() const;
};

#endif // RIDE_H
