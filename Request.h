#ifndef REQUEST_H
#define REQUEST_H

#include <string>
using namespace std;

class Request {
public:
    string userID, receiverID; 
    int rideIndex; 

    Request(string u, string r, int idx);
};

#endif