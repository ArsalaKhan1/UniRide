#include "Request.h"

Request::Request(int reqID, const std::string &u, const std::string &r, int idx)
    : requestID(reqID), userID(u), receiverID(r), rideIndex(idx) {}
