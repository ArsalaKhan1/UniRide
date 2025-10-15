#include "Ride.h"

Ride::Ride(string id, string f, string t, string tm, string m)
    : userID(id), from(f), to(t), time(tm), mode(m) {}

void Ride::displayRide() {
    cout << "From: " << from << "   To: " << to << "   Time: " << time << "   Mode: " << mode << "   Posted by: " << userID << endl;
}
