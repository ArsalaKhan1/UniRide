#include "RideSystem.h"
#include <iostream>

void RideSystem::addRide(string id, string f, string t, string tm, string m) {
    rides.push_back(Ride(id, f, t, tm, m));
}

void RideSystem::viewRides() {
    for (int i = 0; i < rides.size(); i++)
        rides[i].displayRide();
}

vector<Ride> RideSystem::findMatches(string from, string to) {
    vector<Ride> matches;
    for (auto &r : rides) {
        if (r.from == from && r.to == to)
            matches.push_back(r);
    }
    return matches;
}
