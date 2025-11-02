#include "Ride.h"

Ride::Ride(const std::string &id, const std::string &f, const std::string &t,
           const std::string &tm, const std::string &m, RideType rt, bool fo)
    : userID(id), from(f), to(t), time(tm), mode(m), rideType(rt), femalesOnly(fo) {
    
    // Set capacity based on ride type
    switch (rideType) {
        case RideType::BIKE:
            maxCapacity = 2;
            currentCapacity = 1; // Owner counts as 1
            break;
        case RideType::CARPOOL:
            maxCapacity = 5; // Owner + 4 passengers OR 4 participants
            currentCapacity = userID.empty() ? 0 : 1; // 1 if has owner, 0 for participant groups
            break;
        case RideType::RICKSHAW:
            maxCapacity = 3;
            currentCapacity = userID.empty() ? 0 : 1; // First participant
            break;
    }
    
    if (!userID.empty()) {
        participants.push_back(userID);
    }
}

bool Ride::canAcceptMoreParticipants() const {
    return currentCapacity < maxCapacity;
}

int Ride::getAvailableSlots() const {
    return maxCapacity - currentCapacity;
}
