#ifndef RIDE_H
#define RIDE_H

#include <string>
#include <iostream>
using namespace std;

class Ride {
public:
    string userID, from, to, time, mode;

    Ride(string id, string f, string t, string tm, string m);
    void displayRide();
};

#endif
