#include "Ride.h"
#include <algorithm>

Ride::Ride(const std::string &id, const std::string &f, const std::string &t,
           const std::string &tm, const std::string &m, RideType rt, bool fo)
    : rideID(0), ownerID(id), userID(id), from(f), to(t), time(tm), mode(m), rideType(rt), 
      femalesOnly(fo), status(RideStatus::OPEN), genderPreference(fo ? "female" : "any") {
    
    // Set capacity based on ride type
    switch (rideType) {
        case RideType::BIKE:
            maxCapacity = 2;
            currentCapacity = 1; // Owner counts as 1
            break;
        case RideType::CARPOOL:
            maxCapacity = 4; // 4 passengers (driver not counted)
            currentCapacity = 0; // Start with 0, only count passengers
            break;
        case RideType::RICKSHAW:
            maxCapacity = 3;
            currentCapacity = ownerID.empty() ? 0 : 1;
            break;
    }
    
    if (!ownerID.empty()) {
        participants.push_back(ownerID);
    }
}

bool Ride::canAcceptMoreParticipants() const {
    return status == RideStatus::OPEN && currentCapacity < maxCapacity;
}

int Ride::getAvailableSlots() const {
    return maxCapacity - currentCapacity;
}

bool Ride::hasPendingRequest(const std::string& userID) const {
    for (const auto& req : pendingRequests) {
        if (req.userID == userID && req.status == RequestStatus::PENDING) {
            return true;
        }
    }
    return false;
}

void Ride::addJoinRequest(const std::string& userID) {
    if (!hasPendingRequest(userID) && canAcceptMoreParticipants()) {
        pendingRequests.emplace_back(userID);
    }
}

bool Ride::approveRequest(const std::string& userID) {
    for (auto& req : pendingRequests) {
        if (req.userID == userID && req.status == RequestStatus::PENDING) {
            if (canAcceptMoreParticipants()) {
                req.status = RequestStatus::ACCEPTED;
                participants.push_back(userID);
                currentCapacity++;
                updateStatus();
                return true;
            }
        }
    }
    return false;
}

bool Ride::rejectRequest(const std::string& userID) {
    for (auto& req : pendingRequests) {
        if (req.userID == userID && req.status == RequestStatus::PENDING) {
            req.status = RequestStatus::REJECTED;
            return true;
        }
    }
    return false;
}

void Ride::updateStatus() {
    if (currentCapacity >= maxCapacity) {
        status = RideStatus::FULL;
    } else if (status == RideStatus::FULL && currentCapacity < maxCapacity) {
        status = RideStatus::OPEN;
    }
}
