#include "crow.h"
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
    return RideType::CARPOOL; // default
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
    crow::SimpleApp app;

    // Initialize database
    DatabaseManager dbManager("rideshare.db");
    if (!dbManager.initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return -1;
    }

    AuthSystem authSystem;
    RideSystem rideSystem;
    rideSystem.setDatabaseManager(&dbManager);
    RequestQueue requestQueue(&rideSystem, &dbManager);

    // Chat system
    auto chatFeature = std::make_unique<ChatFeature>();

    CROW_ROUTE(app, "/")
    ([]() {
        return "UniRide API is running successfully!";
    });

    // SET USER PREFERENCES
    CROW_ROUTE(app, "/user/preferences").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        std::string genderPref = data.has("genderPreference") ? data["genderPreference"].s() : std::string("any");
        int vehicleType = data["vehicleType"].i(); // 0=BIKE, 1=RICKSHAW, 2=CAR_OWNER, 3=CAR_BOOKING
        
        bool success = dbManager.updateUserPreferences(userID, genderPref, vehicleType);
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? "Preferences updated successfully" : "Failed to update preferences";
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
            res["error"] = "User preferences not found";
        }
        return crow::response(res);
    });

    // FIND MATCHING RIDES
    CROW_ROUTE(app, "/rides/search").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string userID = data["userID"].s();
        std::string from = data["from"].s();
        std::string to = data["to"].s();
        RideType rideType = stringToRideType(data["rideType"].s());
        
        // Get user's gender for matching
        User user = dbManager.getUserByID(userID);
        std::string userGender = user.gender;
        
        auto matches = dbManager.findMatchingRides(from, to, rideType, userID, userGender);
        
        crow::json::wvalue res;
        res["matches"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < matches.size(); ++i) {
            res["matches"][i]["rideID"] = matches[i].rideID;
            res["matches"][i]["ownerID"] = matches[i].ownerID;
            res["matches"][i]["from"] = matches[i].from;
            res["matches"][i]["to"] = matches[i].to;
            res["matches"][i]["time"] = matches[i].time;
            res["matches"][i]["rideType"] = rideTypeToString(matches[i].rideType);
            res["matches"][i]["availableSlots"] = matches[i].getAvailableSlots();
            res["matches"][i]["femalesOnly"] = matches[i].femalesOnly;
        }
        
        return crow::response(res);
    });
    // USER REGISTRATION
    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        auto user = authSystem.registerUser(
            data["id"].s(),
            data["name"].s(),
            data["email"].s(),
            data["gender"].s()
        );
        
        // Save to database
        if (!dbManager.insertUser(user)) {
            return crow::response(500, "Failed to save user to database");
        }
        
        return crow::response(authSystem.toJson());
    });

    // LOGIN
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        User user = dbManager.getUserByEmail(data["email"].s());
        bool ok = !user.email.empty();
        
        crow::json::wvalue res;
        res["status"] = ok ? "Login successful" : "User not found";
        if (ok) {
            res["user"]["id"] = user.userID;
            res["user"]["name"] = user.name;
            res["user"]["email"] = user.email;
        }
        return crow::response(res);
    });



    // GET ALL RIDES
    CROW_ROUTE(app, "/ride/all").methods("GET"_method)
    ([&]() {
        auto rides = dbManager.getAllRides();
        crow::json::wvalue res;
        res["rides"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < rides.size(); ++i) {
            res["rides"][i]["rideID"] = rides[i].rideID;
            res["rides"][i]["leadUserID"] = rides[i].ownerID;
            res["rides"][i]["from"] = rides[i].from;
            res["rides"][i]["to"] = rides[i].to;
            res["rides"][i]["time"] = rides[i].time;
            res["rides"][i]["rideType"] = rideTypeToString(rides[i].rideType);
            res["rides"][i]["currentCapacity"] = rides[i].currentCapacity;
            res["rides"][i]["maxCapacity"] = rides[i].maxCapacity;
            res["rides"][i]["availableSlots"] = rides[i].getAvailableSlots();
            res["rides"][i]["peopleJoined"] = rides[i].currentCapacity;
            res["rides"][i]["spotsRemaining"] = rides[i].getAvailableSlots();
            res["rides"][i]["femalesOnly"] = rides[i].femalesOnly;
        }
        
        return crow::response(res);
    });

    // CREATE RIDE OFFER (for owners)
    CROW_ROUTE(app, "/ride/offer").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        bool femalesOnly = data.has("femalesOnly") ? data["femalesOnly"].b() : false;
        
        // Validate ride type for owners
        if (rideType == RideType::RICKSHAW) {
            return crow::response(400, "Rickshaw rides cannot have owners");
        }
        
        // Create ride object
        Ride ride(data["userID"].s(), data["from"].s(), data["to"].s(), 
                  "now", "offer", rideType, femalesOnly);
        
        // Save to database
        int rideID = dbManager.insertRide(ride);
        if (rideID == -1) {
            return crow::response(500, "Failed to save ride to database");
        }
        
        // Set ride owner as chat lead (hub)
        chatFeature->SetRideLead(rideID, data["userID"].s());
        
        crow::json::wvalue res;
        res["message"] = "Ride offer created successfully";
        res["rideType"] = rideTypeToString(rideType);
        res["maxCapacity"] = (rideType == RideType::BIKE) ? 2 : (rideType == RideType::CARPOOL) ? 4 : 3;
        res["rideID"] = ride.rideID;
        
        return crow::response(res);
    });

    // CREATE REQUEST (for passengers/participants)
    CROW_ROUTE(app, "/request/create").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        
        // Find existing matches first
        auto matches = dbManager.findRideMatches(data["from"].s(), data["to"].s(), rideType, data["userID"].s());
        
        crow::json::wvalue res;
        res["rideType"] = rideTypeToString(rideType);
        
        if (!matches.empty()) {
            // Show existing matches for user to join
            res["message"] = "Found existing travel requests matching your preferences";
            res["matches"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < matches.size(); ++i) {
                res["matches"][i]["rideID"] = matches[i].rideID;
                res["matches"][i]["leadUserID"] = matches[i].ownerID;
                res["matches"][i]["from"] = matches[i].from;
                res["matches"][i]["to"] = matches[i].to;
                res["matches"][i]["time"] = matches[i].time;
                res["matches"][i]["rideType"] = rideTypeToString(matches[i].rideType);
                res["matches"][i]["currentCapacity"] = matches[i].currentCapacity;
                res["matches"][i]["maxCapacity"] = matches[i].maxCapacity;
                res["matches"][i]["availableSlots"] = matches[i].getAvailableSlots();
                res["matches"][i]["peopleJoined"] = matches[i].currentCapacity;
                res["matches"][i]["spotsRemaining"] = matches[i].getAvailableSlots();
            }
        } else {
            // No matches found, create new request and make user the lead
            if (!dbManager.insertRequest(data["userID"].s(), data["from"].s(), data["to"].s(), rideType)) {
                return crow::response(500, "Failed to save request to database");
            }
            
            // Create new ride with user as lead
            Ride newRide(data["userID"].s(), data["from"].s(), data["to"].s(), 
                        "now", "request", rideType, false);
            
            int rideID = dbManager.insertRide(newRide);
            if (rideID == -1) {
                return crow::response(500, "Failed to create new ride");
            }
            
            // Set user as chat lead (hub) for this new request
            chatFeature->SetRideLead(rideID, data["userID"].s());
            
            res["message"] = "No matching requests found. You are now the lead for a new travel request.";
            res["rideID"] = rideID;
            res["leadUserID"] = data["userID"].s();
            res["matches"] = crow::json::wvalue::list();
        }
        
        return crow::response(res);
    });

    // VIEW REQUESTS
    CROW_ROUTE(app, "/request/pending").methods("GET"_method)
    ([&]() {
        return crow::response(requestQueue.listPending());
    });

    // JOIN RIDE
    CROW_ROUTE(app, "/ride/join").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        
        bool success = rideSystem.joinRide(
            data["userID"].s(),
            data["from"].s(),
            data["to"].s(),
            rideType
        );
        
        crow::json::wvalue res;
        if (success) {
            res["message"] = "Successfully joined ride";
            res["status"] = "success";
        } else {
            res["message"] = "No available rides found or ride is full";
            res["status"] = "failed";
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
        
        // Check if user already has active request
        if (dbManager.hasActiveRequest(userID)) {
            crow::json::wvalue res;
            res["success"] = false;
            res["message"] = "You already have a pending request. Wait for approval or rejection.";
            return crow::response(res);
        }
        
        bool success = dbManager.insertJoinRequest(rideID, userID);
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? "Join request sent successfully" : "Failed to send join request";
        return crow::response(res);
    });

    // APPROVE/REJECT JOIN REQUEST
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
            // Get current ride and update capacity
            auto rides = dbManager.getAllRides();
            for (const auto& ride : rides) {
                if (ride.rideID == rideID) {
                    int newCapacity = ride.currentCapacity + 1;
                    dbManager.updateRideCapacityByID(rideID, newCapacity);
                    
                    // Update ride status to full if capacity reached
                    if (newCapacity >= ride.maxCapacity) {
                        dbManager.updateRideStatus(rideID, "full");
                    }
                    break;
                }
            }
        }
        
        crow::json::wvalue res;
        res["success"] = success;
        res["message"] = success ? (accept ? "Request approved" : "Request rejected") : "Failed to process request";
        return crow::response(res);
    });

    // GET PENDING REQUESTS FOR RIDE OWNER
    CROW_ROUTE(app, "/ride/<int>/requests").methods("GET"_method)
    ([&](int rideID) {
        auto requests = dbManager.getPendingRequests(rideID);
        
        crow::json::wvalue res;
        res["requests"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < requests.size(); ++i) {
            res["requests"][i]["userID"] = requests[i].first;
            res["requests"][i]["timestamp"] = requests[i].second;
        }
        
        return crow::response(res);
    });

    // RESPOND TO REQUEST (Legacy)
    CROW_ROUTE(app, "/request/respond").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");
        
        std::string status = data["accept"].b() ? "accepted" : "rejected";
        dbManager.updateRequestStatus(data["requestID"].i(), status);
        
        std::string msg;
        bool success = requestQueue.respondToRequest(data["requestID"].i(), data["accept"].b(), msg);
        crow::json::wvalue res;
        res["message"] = msg;
        res["success"] = success;
        return crow::response(res);
    });



    // CHAT SEND (Hub-and-Spoke Model)
    CROW_ROUTE(app, "/chat/send").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string sender = data["sender"].s();
        std::string recipient = data["recipient"].s();
        std::string text = data["text"].s();
        int rideID = data["rideID"].i();

        // Save message to database
        if (!dbManager.insertMessage(sender, text)) {
            return crow::response(500, "Failed to save message to database");
        }

        std::string err;
        bool ok = chatFeature->AddMessage(sender, recipient, text, rideID, err);
        crow::json::wvalue res;
        res["success"] = ok;
        res["error"] = err;
        return crow::response(res);
    });

    // CHAT SEND (Legacy - broadcasts to all)
    CROW_ROUTE(app, "/chat/broadcast").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        // Save message to database
        if (!dbManager.insertMessage(data["sender"].s(), data["text"].s())) {
            return crow::response(500, "Failed to save message to database");
        }

        crow::json::wvalue res;
        res["success"] = true;
        res["message"] = "Message broadcasted";
        return crow::response(res);
    });

    // CHAT FETCH FOR SPECIFIC RIDE
    CROW_ROUTE(app, "/chat/ride/<int>").methods("GET"_method)
    ([&](int rideID) {
        return crow::response(chatFeature->getRideMessagesJson(rideID));
    });

    // CHAT FETCH ALL (Legacy)
    CROW_ROUTE(app, "/chat/all").methods("GET"_method)
    ([&]() {
        auto messages = dbManager.getAllMessages();
        crow::json::wvalue res;
        res["messages"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < messages.size(); ++i) {
            res["messages"][i]["sender"] = messages[i].first;
            res["messages"][i]["text"] = messages[i].second;
        }
        
        return crow::response(res);
    });

    app.port(8080).multithreaded().run();
}
