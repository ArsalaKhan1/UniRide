#include "Ride.h"

Ride::Ride(const std::string &id, const std::string &f, const std::string &t,
           const std::string &tm, const std::string &m)
    : userID(id), from(f), to(t), time(tm), mode(m) {}
