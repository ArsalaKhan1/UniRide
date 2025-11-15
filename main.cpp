#include "crow.h"
#include "crow/middlewares/cors.h"
#include "AuthSys.h"
#include "RideSystem.h"
#include "RequestQueue.h"
#include "ChatFeature.h"
#include "DatabaseManager.h"
#include <memory>
#include <iostream>

// Helper function to convert string to RideType
RideType stringToRideType(const std::string& typeStr) {
    if (typeStr == "bike") return RideType::BIKE;
    if (typeStr == "carpool") return RideType::CARPOOL;
    if (typeStr == "rickshaw") return RideType::RICKSHAW;
    return RideType::CARPOOL;
}

// Helper function to convert RideType to string
std::string rideTypeToString(RideType type) {
    switch (type) {
        case RideType::BIKE: return "bike";
        case RideType::CARPOOL: return "carpool";
        case RideType::RICKSHAW: return "rickshaw";
        default: return "carpool";
    }
}

int main() {
    // Setup CORS-enabled app
    crow::App<crow::CORSHandler> app;
    
    // Configure CORS BEFORE routes
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("Content-Type", "Authorization")
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .origin("http://localhost:5173")
        .allow_credentials();

    // Initialize database
    auto dbManagerPtr = std::make_shared<DatabaseManager>("rideshare.db");
    if (!dbManagerPtr->initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return -1;
    }

    DatabaseManager& dbManager = *dbManagerPtr;
    AuthSystem authSystem(dbManagerPtr);
    RideSystem rideSystem;
    rideSystem.setDatabaseManager(&dbManager);
    
    if (!rideSystem.initializeLocationGraph("areas.db")) {
        std::cerr << "Warning: Failed to initialize location graph." << std::endl;
    }
    
    dbManager.setLocationGraph(&rideSystem.getLocationGraph());
    RequestQueue requestQueue(&rideSystem, &dbManager);
    auto chatFeature = std::make_unique<ChatFeature>();

    CROW_ROUTE(app, "/")
    ([]() {
        return "UniRide API is running successfully!";
    });

    // FIXED: Google Auth endpoint with proper JSON handling
    CROW_ROUTE(app, "/auth/google/verify").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) {
            crow::json::wvalue err;
            err["success"] = false;
            err["error"] = "Invalid JSON";
            return crow::response(400, err);
        }

        if (!data.has("idToken") || !data.has("enrollmentId")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["error"] = "Missing idToken or enrollmentId";
            return crow::response(400, err);
        }

        std::string idToken = data["idToken"].s();
        std::string enrollmentId = data["enrollmentId"].s();

        User user = authSystem.handleGoogleAuth(idToken, enrollmentId);
        
        if (user.userID.empty()) {
            crow::json::wvalue err;
            err["success"] = false;
            err["error"] = "Invalid Enrollment ID";
            return crow::response(401, err);
        }

        std::string sessionToken = authSystem.storeSessionToken(user.userID);

        crow::json::wvalue res;
        res["success"] = true;
        res["user"]["id"] = user.userID;
        res["user"]["name"] = user.name;
        res["user"]["email"] = user.email;
        res["sessionToken"] = sessionToken;
        res["expiresIn"] = 86400;

        return crow::response(200, res);
    });

    // SET USER PREFERENCES
    CROW_ROUTE(app, "/user/preferences").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        std::string genderPref = data.has("genderPreference") ? data["genderPreference"].s() : std::string("any");
        int vehicleType = data["vehicleType"].i();
        
        bool success = dbManager.updateUserPreferences(userID, genderPref, vehicleType);
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? "Preferences updated" : "Failed to update";
        return crow::response(res);
    });

    // GET USER PREFERENCES
    CROW_ROUTE(app, "/user/preferences/<string>").methods("GET"_method)
    ([&](const std::string& userID) {
        std::string genderPref;
        int vehicleType;
        
        bool success = dbManager.getUserPreferences(userID, genderPref, vehicleType);
        
        crow::json::wvalue res;
        if (success) {
            res["genderPreference"] = genderPref;
            res["vehicleType"] = vehicleType;
        } else {
            res["error"] = "Not found";
        }
        return crow::response(res);
    });

    // Helper function to convert RideStatus to string
    auto rideStatusToString = [](RideStatus status) -> std::string {
        switch (status) {
            case RideStatus::OPEN: return "open";
            case RideStatus::FULL: return "full";
            case RideStatus::STARTED: return "started";
            case RideStatus::COMPLETED: return "completed";
            default: return "open";
        }
    };

    // GET ALL RIDES
    CROW_ROUTE(app, "/ride/all").methods("GET"_method)
    ([&]() {
        auto rides = dbManager.getAllRides();
        crow::json::wvalue res;
        res["rides"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < rides.size(); ++i) {
            User leadUser = dbManager.getUserByID(rides[i].ownerID);
            res["rides"][i]["rideID"] = rides[i].rideID;
            res["rides"][i]["leadUserID"] = rides[i].ownerID;
            res["rides"][i]["leadUserName"] = leadUser.name;
            res["rides"][i]["from"] = rides[i].from;
            res["rides"][i]["to"] = rides[i].to;
            res["rides"][i]["time"] = rides[i].time;
            res["rides"][i]["rideType"] = rideTypeToString(rides[i].rideType);
            res["rides"][i]["currentCapacity"] = rides[i].currentCapacity;
            res["rides"][i]["maxCapacity"] = rides[i].maxCapacity;
            res["rides"][i]["availableSlots"] = rides[i].getAvailableSlots();
            res["rides"][i]["femalesOnly"] = rides[i].femalesOnly;
            res["rides"][i]["status"] = rideStatusToString(rides[i].status);
        }
        
        return crow::response(res);
    });

    // CREATE RIDE OFFER
    CROW_ROUTE(app, "/ride/offer").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        
        // Check if user already has an active ride
        auto activeRides = dbManager.getActiveRidesForUser(userID);
        if (!activeRides.empty()) {
            crow::json::wvalue res;
            res["success"] = false;
            res["error"] = "You already have an active ride. Please end your current ride before creating a new one.";
            return crow::response(400, res);
        }

        RideType rideType = stringToRideType(data["rideType"].s());
        bool femalesOnly = data.has("femalesOnly") ? data["femalesOnly"].b() : false;
        
        if (rideType == RideType::RICKSHAW) {
            return crow::response(400, "Rickshaw rides cannot have owners");
        }
        
        Ride ride(userID, data["from"].s(), data["to"].s(), 
                  "now", "offer", rideType, femalesOnly);

        // Interpret `seats` from the frontend as passenger seats (excluding owner)
        // when an owner is present for bike/carpool. Internally the code stores
        // maxCapacity as the total number of people. To keep existing logic
        // (where currentCapacity starts at 1 for an owner), add 1 to the
        // frontend-provided passenger seats when an owner exists. If there is
        // no owner (e.g., rickshaw-like flows created without an owner), treat
        // the provided seats as the total capacity and ensure the creator is
        // counted in currentCapacity/participants when appropriate.
        if (data.has("seats")) {
            int seatsFromFrontend = data["seats"].i();

            if (!ride.ownerID.empty() && (ride.rideType == RideType::BIKE || ride.rideType == RideType::CARPOOL)) {
                // Frontend gave passenger seats excluding owner; store total people count
                ride.maxCapacity = seatsFromFrontend + 1;
                // Ensure owner is counted as current participant
                ride.currentCapacity = 1;
                if (std::find(ride.participants.begin(), ride.participants.end(), ride.ownerID) == ride.participants.end() && !ride.ownerID.empty()) {
                    ride.participants.push_back(ride.ownerID);
                }
            } else {
                // No owner or other ride types: treat seats as total capacity.
                ride.maxCapacity = seatsFromFrontend;
                // If creator exists, ensure they are counted
                if (!ride.ownerID.empty()) {
                    ride.currentCapacity = 1;
                    if (std::find(ride.participants.begin(), ride.participants.end(), ride.ownerID) == ride.participants.end()) {
                        ride.participants.push_back(ride.ownerID);
                    }
                }
            }
        }
        
        int rideID = dbManager.insertRide(ride);
        if (rideID == -1) {
            return crow::response(500, "Failed to save ride");
        }
        
        chatFeature->SetRideLead(rideID, data["userID"].s());
        
        crow::json::wvalue res;
        res["message"] = "Ride created";
        res["rideType"] = rideTypeToString(rideType);
        res["maxCapacity"] = ride.maxCapacity;
        res["rideID"] = ride.rideID;
        
        return crow::response(res);
    });

    // CREATE REQUEST
    CROW_ROUTE(app, "/request/create").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        
        // Check if user already has an active ride
        auto activeRides = dbManager.getActiveRidesForUser(userID);
        if (!activeRides.empty()) {
            crow::json::wvalue res;
            res["success"] = false;
            res["error"] = "You already have an active ride. Please end your current ride before creating a new one.";
            return crow::response(400, res);
        }

        RideType rideType = stringToRideType(data["rideType"].s());
        User user = dbManager.getUserByID(userID);
        auto matches = dbManager.findMatchingRides(data["from"].s(), data["to"].s(), rideType, userID, user.gender);
        
        crow::json::wvalue res;
        res["rideType"] = rideTypeToString(rideType);
        
        if (!matches.empty()) {
            res["message"] = "Found existing matches";
            res["matches"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < matches.size(); ++i) {
                User leadUser = dbManager.getUserByID(matches[i].ownerID);
                res["matches"][i]["rideID"] = matches[i].rideID;
                res["matches"][i]["leadUserID"] = matches[i].ownerID;
                res["matches"][i]["leadUserName"] = leadUser.name;
                // Add a display string that includes username, ride type, and available seats
                {
                    std::string display = leadUser.name + " - " + rideTypeToString(matches[i].rideType) + " - " + std::to_string(matches[i].getAvailableSlots()) + " seats";
                    res["matches"][i]["leadDisplay"] = display;
                }
                res["matches"][i]["from"] = matches[i].from;
                res["matches"][i]["to"] = matches[i].to;
                res["matches"][i]["time"] = matches[i].time;
                res["matches"][i]["rideType"] = rideTypeToString(matches[i].rideType);
                res["matches"][i]["availableSlots"] = matches[i].getAvailableSlots();
            }
        } else {
            // Always record the user's request
            dbManager.insertRequest(data["userID"].s(), data["from"].s(), data["to"].s(), rideType);

            // If this call is only a search (frontend sets searchOnly=true), do NOT auto-create a ride.
            bool isSearchOnly = data.has("searchOnly") ? (data["searchOnly"].b()) : false;

            if (!isSearchOnly) {
                Ride newRide(data["userID"].s(), data["from"].s(), data["to"].s(), "now", "request", rideType, false);
                int rideID = dbManager.insertRide(newRide);
                if (rideID != -1) {
                    chatFeature->SetRideLead(rideID, data["userID"].s());
                }

                User leadUser = dbManager.getUserByID(data["userID"].s());
                res["message"] = "You are now the lead";
                res["rideID"] = rideID;
                res["leadUserID"] = data["userID"].s();
                res["leadUserName"] = leadUser.name;
                res["matches"] = crow::json::wvalue::list();
            } else {
                // Search-only: inform caller that no matches were found and do not create rides
                res["message"] = "No matching requests found";
                res["matches"] = crow::json::wvalue::list();
            }
        }
        
        return crow::response(res);
    });

    // SEND JOIN REQUEST
    CROW_ROUTE(app, "/ride/request").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        int rideID = data["rideID"].i();
        
        // Check if ride exists and is not started or completed
        Ride ride = dbManager.getRideByID(rideID);
        if (ride.rideID == 0) {
            crow::json::wvalue res;
            res["success"] = false;
            res["message"] = "Ride not found";
            return crow::response(404, res);
        }
        
        if (ride.status == RideStatus::STARTED || ride.status == RideStatus::COMPLETED) {
            crow::json::wvalue res;
            res["success"] = false;
            res["message"] = "Cannot join a ride that has already started or been completed";
            return crow::response(400, res);
        }
        
        if (dbManager.hasActiveRequest(userID)) {
            crow::json::wvalue res;
            res["success"] = false;
            res["message"] = "You already have a pending request";
            return crow::response(res);
        }
        
        bool success = dbManager.insertJoinRequest(rideID, userID);
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? "Request sent" : "Failed";
        return crow::response(res);
    });

    // APPROVE/REJECT REQUEST
    CROW_ROUTE(app, "/ride/respond").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        int rideID = data["rideID"].i();
        std::string userID = data["userID"].s();
        bool accept = data["accept"].b();
        
        std::string status = accept ? "accepted" : "rejected";
        bool success = dbManager.updateJoinRequestStatus(rideID, userID, status);
        
        if (success && accept) {
            // Ensure chat lead is set for this ride (in case it wasn't set before)
            Ride ride = dbManager.getRideByID(rideID);
            if (!ride.ownerID.empty()) {
                chatFeature->SetRideLead(rideID, ride.ownerID);
            }
            
            auto rides = dbManager.getAllRides();
            for (const auto& rideItem : rides) {
                if (rideItem.rideID == rideID) {
                    int newCapacity = rideItem.currentCapacity + 1;
                    dbManager.updateRideCapacityByID(rideID, newCapacity);
                    if (newCapacity >= rideItem.maxCapacity) {
                        dbManager.updateRideStatus(rideID, "full");
                    }
                    break;
                }
            }
        }
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? (accept ? "Approved" : "Rejected") : "Failed";
        return crow::response(res);
    });

    // GET RIDE REQUESTS
    CROW_ROUTE(app, "/ride/<int>/requests").methods("GET"_method)
    ([&](int rideID) {
        auto requests = dbManager.getPendingRequests(rideID);
        crow::json::wvalue res;
        res["requests"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < requests.size(); ++i) {
            res["requests"][i]["userID"] = requests[i].first;
            // Include the requester's username alongside their ID
            User reqUser = dbManager.getUserByID(requests[i].first);
            res["requests"][i]["userName"] = reqUser.name;
            res["requests"][i]["timestamp"] = requests[i].second;
        }
        
        return crow::response(res);
    });

    // CHAT SEND
    CROW_ROUTE(app, "/chat/send").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string sender = data["sender"].s();
        std::string recipient = data["recipient"].s();
        std::string text = data["text"].s();
        int rideID = data["rideID"].i();

        // Check if ride is completed - disable chat
        Ride ride = dbManager.getRideByID(rideID);
        if (ride.status == RideStatus::COMPLETED) {
            crow::json::wvalue res;
            res["success"] = false;
            res["error"] = "Cannot send messages for a completed ride";
            return crow::response(400, res);
        }

        dbManager.insertMessage(sender, text);

        std::string err;
        bool ok = chatFeature->AddMessage(sender, recipient, text, rideID, err);
        crow::json::wvalue res;
        res["success"] = ok;
        res["error"] = err;
        return crow::response(res);
    });

    // CHAT GET BY RIDE
    CROW_ROUTE(app, "/chat/ride/<int>").methods("GET"_method)
    ([&](int rideID) {
        return crow::response(chatFeature->getRideMessagesJson(rideID));
    });

    // GET ACCEPTED REQUESTS FOR USER (for notifications)
    CROW_ROUTE(app, "/user/<string>/accepted-requests").methods("GET"_method)
    ([&](const std::string& userID) {
        auto accepted = dbManager.getAcceptedRequestsForUser(userID);
        crow::json::wvalue res;
        res["acceptedRequests"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < accepted.size(); ++i) {
            int rideID = accepted[i].first;
            std::string leadUserID = accepted[i].second;
            
            // Get ride details
            auto allRides = dbManager.getAllRides();
            for (const auto& ride : allRides) {
                if (ride.rideID == rideID) {
                    User leadUser = dbManager.getUserByID(leadUserID);
                    res["acceptedRequests"][i]["rideID"] = rideID;
                    res["acceptedRequests"][i]["from"] = ride.from;
                    res["acceptedRequests"][i]["to"] = ride.to;
                    res["acceptedRequests"][i]["rideType"] = rideTypeToString(ride.rideType);
                    res["acceptedRequests"][i]["leadUserID"] = leadUserID;
                    res["acceptedRequests"][i]["leadUserName"] = leadUser.name;
                    break;
                }
            }
        }
        
        return crow::response(res);
    });

    // GET ACCEPTED PASSENGERS FOR RIDE (for ride leads)
    CROW_ROUTE(app, "/ride/<int>/accepted").methods("GET"_method)
    ([&](int rideID) {
        auto passengers = dbManager.getAcceptedPassengers(rideID);
        crow::json::wvalue res;
        res["accepted"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < passengers.size(); ++i) {
            res["accepted"][i]["userID"] = passengers[i].first;
            res["accepted"][i]["userName"] = passengers[i].second;
        }
        
        return crow::response(res);
    });

    // GET RIDE PARTICIPANTS (lead + accepted passengers) for chat
    CROW_ROUTE(app, "/ride/<int>/participants").methods("GET"_method)
    ([&](int rideID) {
        Ride ride = dbManager.getRideByID(rideID);
        auto passengers = dbManager.getAcceptedPassengers(rideID);
        crow::json::wvalue res;
        res["participants"] = crow::json::wvalue::list();
        
        // Add ride lead
        if (!ride.ownerID.empty()) {
            User leadUser = dbManager.getUserByID(ride.ownerID);
            res["participants"][0]["userID"] = ride.ownerID;
            res["participants"][0]["userName"] = leadUser.name;
            res["participants"][0]["isLead"] = true;
        }
        
        // Add accepted passengers
        for (size_t i = 0; i < passengers.size(); ++i) {
            res["participants"][i + 1]["userID"] = passengers[i].first;
            res["participants"][i + 1]["userName"] = passengers[i].second;
            res["participants"][i + 1]["isLead"] = false;
        }
        
        return crow::response(res);
    });

    // START RIDE (only for lead)
    CROW_ROUTE(app, "/ride/<int>/start").methods("POST"_method)
    ([&](const crow::request &req, crow::response &res, int rideID) {
        auto data = crow::json::load(req.body);
        if (!data) {
            res.code = 400;
            res.body = "Invalid JSON";
            res.end();
            return;
        }

        std::string userID = data["userID"].s();
        Ride ride = dbManager.getRideByID(rideID);
        
        if (ride.rideID == 0) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Ride not found";
            res.code = 404;
            res.body = result.dump();
            res.end();
            return;
        }
        
        if (ride.ownerID != userID) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Only the ride lead can start the ride";
            res.code = 403;
            res.body = result.dump();
            res.end();
            return;
        }
        
        if (ride.status == RideStatus::STARTED) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Ride is already started";
            res.code = 400;
            res.body = result.dump();
            res.end();
            return;
        }
        
        if (ride.status == RideStatus::COMPLETED) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Cannot start a completed ride";
            res.code = 400;
            res.body = result.dump();
            res.end();
            return;
        }
        
        bool success = dbManager.updateRideStatus(rideID, "started");
        
        crow::json::wvalue result;
        result["success"] = success;
        result["message"] = success ? "Ride started successfully" : "Failed to start ride";
        res.body = result.dump();
        res.end();
    });

    // END RIDE (only for lead)
    CROW_ROUTE(app, "/ride/<int>/end").methods("POST"_method)
    ([&](const crow::request &req, crow::response &res, int rideID) {
        auto data = crow::json::load(req.body);
        if (!data) {
            res.code = 400;
            res.body = "Invalid JSON";
            res.end();
            return;
        }

        std::string userID = data["userID"].s();
        Ride ride = dbManager.getRideByID(rideID);
        
        if (ride.rideID == 0) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Ride not found";
            res.code = 404;
            res.body = result.dump();
            res.end();
            return;
        }
        
        if (ride.ownerID != userID) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Only the ride lead can end the ride";
            res.code = 403;
            res.body = result.dump();
            res.end();
            return;
        }
        
        if (ride.status == RideStatus::COMPLETED) {
            crow::json::wvalue result;
            result["success"] = false;
            result["error"] = "Ride is already completed";
            res.code = 400;
            res.body = result.dump();
            res.end();
            return;
        }
        
        bool success = dbManager.updateRideStatus(rideID, "completed");
        
        crow::json::wvalue result;
        result["success"] = success;
        result["message"] = success ? "Ride ended successfully" : "Failed to end ride";
        res.body = result.dump();
        res.end();
    });

    app.port(8080).multithreaded().run();
}