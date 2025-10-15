#ifndef RIDESYSTEM_H
#define RIDESYSTEM_H

#include <vector>
#include "Ride.h"
using namespace std;

class RideSystem {
private:
    vector<Ride> rides;

public:
    void addRide(string id, string f, string t, string tm, string m);
    void viewRides();
    vector<Ride> findMatches(string from, string to);
};

#endif
