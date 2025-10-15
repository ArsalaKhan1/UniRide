#ifndef REQUEUEQUEUE_H
#define REQUESTQUEUE_H

#include <queue>
#include "Request.h"
#include "RideSystem.h"
using namespace std;

class RequestQueue
{
private:
    queue<Request> reqQueue;
    RideSystem *rideSystem; 

public:
    RequestQueue(RideSystem *rs);
    void createRequest(string userID, string from, string to);
    void enqueueRequest(string userID, string receiverID, int rideIdx);
    void processRequests();
    void displayRequests();
    bool isEmpty();
};

#endif
