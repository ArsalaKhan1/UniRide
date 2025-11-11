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
            err["error"] = "Invalid token or enrollment ID";
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
            res["rides"][i]["femalesOnly"] = rides[i].femalesOnly;
        }
        
        return crow::response(res);
    });

    // CREATE RIDE OFFER
    CROW_ROUTE(app, "/ride/offer").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        bool femalesOnly = data.has("femalesOnly") ? data["femalesOnly"].b() : false;
        
        if (rideType == RideType::RICKSHAW) {
            return crow::response(400, "Rickshaw rides cannot have owners");
        }
        
        Ride ride(data["userID"].s(), data["from"].s(), data["to"].s(), 
                  "now", "offer", rideType, femalesOnly);
        
        int rideID = dbManager.insertRide(ride);
        if (rideID == -1) {
            return crow::response(500, "Failed to save ride");
        }
        
        chatFeature->SetRideLead(rideID, data["userID"].s());
        
        crow::json::wvalue res;
        res["message"] = "Ride created";
        res["rideType"] = rideTypeToString(rideType);
        res["maxCapacity"] = (rideType == RideType::BIKE) ? 2 : 4;
        res["rideID"] = ride.rideID;
        
        return crow::response(res);
    });

    // CREATE REQUEST
    CROW_ROUTE(app, "/request/create").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        User user = dbManager.getUserByID(data["userID"].s());
        auto matches = dbManager.findMatchingRides(data["from"].s(), data["to"].s(), rideType, data["userID"].s(), user.gender);
        
        crow::json::wvalue res;
        res["rideType"] = rideTypeToString(rideType);
        
        if (!matches.empty()) {
            res["message"] = "Found existing matches";
            res["matches"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < matches.size(); ++i) {
                res["matches"][i]["rideID"] = matches[i].rideID;
                res["matches"][i]["leadUserID"] = matches[i].ownerID;
                res["matches"][i]["from"] = matches[i].from;
                res["matches"][i]["to"] = matches[i].to;
                res["matches"][i]["time"] = matches[i].time;
                res["matches"][i]["rideType"] = rideTypeToString(matches[i].rideType);
                res["matches"][i]["availableSlots"] = matches[i].getAvailableSlots();
            }
        } else {
            dbManager.insertRequest(data["userID"].s(), data["from"].s(), data["to"].s(), rideType);
            Ride newRide(data["userID"].s(), data["from"].s(), data["to"].s(), "now", "request", rideType, false);
            int rideID = dbManager.insertRide(newRide);
            chatFeature->SetRideLead(rideID, data["userID"].s());
            
            res["message"] = "You are now the lead";
            res["rideID"] = rideID;
            res["leadUserID"] = data["userID"].s();
            res["matches"] = crow::json::wvalue::list();
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
            auto rides = dbManager.getAllRides();
            for (const auto& ride : rides) {
                if (ride.rideID == rideID) {
                    int newCapacity = ride.currentCapacity + 1;
                    dbManager.updateRideCapacityByID(rideID, newCapacity);
                    if (newCapacity >= ride.maxCapacity) {
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

    app.port(8080).multithreaded().run();
}