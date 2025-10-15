#include "RequestQueue.h"
#include <iostream>

RequestQueue::RequestQueue(RideSystem *rs) : rideSystem(rs) {}

// Enqueue all matching rides
void RequestQueue::createRequest(string userID, string from, string to)
{
    vector<Ride> matches = rideSystem->findMatches(from, to);
    if (!matches.empty())
    {
        for (int i = 0; i < matches.size(); i++)
        {
            Ride matchedRide = matches[i];
            enqueueRequest(userID, matchedRide.userID, i);
        }
    }
    else
    {
        cout << "No matching rides found for " << userID << " from "
             << from << " to " << to << ".\n";
    }
}

void RequestQueue::enqueueRequest(string userID, string receiverID, int rideIdx)
{
    Request newReq(userID, receiverID, rideIdx);
    reqQueue.push(newReq);
    cout << "Request from " << userID << " -> " << receiverID
         << " for ride " << rideIdx << " added to queue.\n";
}

//  choose which request to accept/reject
void RequestQueue::processRequests()
{
    while (!reqQueue.empty())
    {
        Request current = reqQueue.front(); // get the first request

        cout << "\n--- New Ride Request ---\n";
        cout << "From: " << current.userID
             << " -> To: " << current.receiverID
             << " (Ride Index: " << current.rideIndex << ")\n";
        cout << "-------------------------\n";

        char decision;
        cout << "Accept (A) or Reject (R)? ";
        cin >> decision;

        if (decision == 'A' || decision == 'a')
        {
            cout << "Request accepted: " << current.userID
                 << " is now connected with " << current.receiverID << "!\n";

            // After acceptance, we can stop (return) or keep going
            // If you want to return immediately after an acceptance:
            return;
        }
        else
        {
            cout << "Request rejected: " << current.userID
                 << " → " << current.receiverID << endl;
        }

        // remove the processed request
        reqQueue.pop();
    }

    cout << "\nAll pending requests have been processed!\n";
}

void RequestQueue::displayRequests()
{
    if (reqQueue.empty())
    {
        cout << "No pending requests.\n";
        return;
    }

    cout << "\n--- Pending Requests ---\n";
    queue<Request> temp = reqQueue;
    while (!temp.empty())
    {
        Request r = temp.front();
        cout << "From: " << r.userID
             << " → To: " << r.receiverID
             << " (Ride Index: " << r.rideIndex << ")\n";
        temp.pop();
    }
    cout << "-------------------------\n";
}

bool RequestQueue::isEmpty()
{
    return reqQueue.empty();
}