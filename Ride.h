#ifndef RIDE_H
#define RIDE_H

#include <string>

struct Ride {
    std::string userID;  // ID of the user offering or taking the ride
    std::string from;    // Starting point
    std::string to;      // Destination
    std::string time;    // Time of the ride
    std::string mode;    // Mode of transport (car, bike, etc.)

    Ride() = default;
    Ride(const std::string &id, const std::string &f, const std::string &t,
         const std::string &tm, const std::string &m);
};

#endif // RIDE_H
