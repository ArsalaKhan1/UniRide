#include "crow.h"
#include "AuthSys.h"
#include "RideSystem.h"
#include "RequestQueue.h"
#include "OTPverification.h"
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

    // OTP + Chat shared state
    auto otpSystem = std::make_shared<OTPVerification>("", "");
    auto chatFeature = std::make_unique<ChatFeature>(otpSystem);

    CROW_ROUTE(app, "/")
    ([]() {
        return "UniRide API is running successfully!";
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
            res["rides"][i]["userID"] = rides[i].userID;
            res["rides"][i]["from"] = rides[i].from;
            res["rides"][i]["to"] = rides[i].to;
            res["rides"][i]["time"] = rides[i].time;
            res["rides"][i]["mode"] = rides[i].mode;
            res["rides"][i]["rideType"] = rideTypeToString(rides[i].rideType);
            res["rides"][i]["currentCapacity"] = rides[i].currentCapacity;
            res["rides"][i]["maxCapacity"] = rides[i].maxCapacity;
            res["rides"][i]["availableSlots"] = rides[i].getAvailableSlots();
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
        
        auto ids = requestQueue.createRideOffer(
            data["userID"].s(),
            data["from"].s(),
            data["to"].s(),
            rideType,
            femalesOnly
        );
        
        crow::json::wvalue res;
        res["message"] = "Ride offer created successfully";
        res["rideType"] = rideTypeToString(rideType);
        res["maxCapacity"] = (rideType == RideType::BIKE) ? 2 : 5;
        
        return crow::response(res);
    });

    // CREATE REQUEST (for passengers/participants)
    CROW_ROUTE(app, "/request/create").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        RideType rideType = stringToRideType(data["rideType"].s());
        
        // Save request to database
        if (!dbManager.insertRequest(data["userID"].s(), data["from"].s(), data["to"].s(), rideType)) {
            return crow::response(500, "Failed to save request to database");
        }

        // Find matches from database
        auto matches = dbManager.findRideMatches(data["from"].s(), data["to"].s(), rideType, data["userID"].s());
        
        auto ids = requestQueue.createRequest(
            data["userID"].s(),
            data["from"].s(),
            data["to"].s(),
            rideType
        );
        
        crow::json::wvalue res;
        res["createdIDs"] = ids;
        res["rideType"] = rideTypeToString(rideType);
        res["matches"] = crow::json::wvalue::list();
        
        for (size_t i = 0; i < matches.size(); ++i) {
            res["matches"][i]["userID"] = matches[i].userID;
            res["matches"][i]["from"] = matches[i].from;
            res["matches"][i]["to"] = matches[i].to;
            res["matches"][i]["time"] = matches[i].time;
            res["matches"][i]["mode"] = matches[i].mode;
            res["matches"][i]["rideType"] = rideTypeToString(matches[i].rideType);
            res["matches"][i]["availableSlots"] = matches[i].getAvailableSlots();
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

    // RESPOND TO REQUEST
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

    // OTP INITIATE
    CROW_ROUTE(app, "/otp/initiate").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        otpSystem = std::make_shared<OTPVerification>(
            data["userA_ID"].s(),
            data["userB_ID"].s()
        );
        chatFeature = std::make_unique<ChatFeature>(otpSystem);  // Update chat with new OTP system
        std::string otp = otpSystem->initiateVerification();
        
        // Save OTP session to database
        if (!dbManager.insertOTPSession(data["userA_ID"].s(), data["userB_ID"].s(), otp)) {
            return crow::response(500, "Failed to save OTP session to database");
        }

        crow::json::wvalue res;
        res["otp"] = otp;
        return crow::response(res);
    });

    // OTP VERIFY
    CROW_ROUTE(app, "/otp/verify").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        bool ok = otpSystem->verifyOTPInput(data["userID"].s(), data["otp"].s());
        
        // Update OTP status in database
        if (ok) {
            // Note: This is simplified - in real app you'd track which session this belongs to
            dbManager.updateOTPStatus("", "", "verified");
        }
        
        crow::json::wvalue res;
        res["status"] = ok ? "Verified" : "Invalid OTP";
        return crow::response(res);
    });

    // OTP STATUS
    CROW_ROUTE(app, "/otp/status").methods("GET"_method)
    ([&]() {
        return crow::response(otpSystem->statusJson());
    });

    // CHAT SEND
    CROW_ROUTE(app, "/chat/send").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        // Save message to database
        if (!dbManager.insertMessage(data["sender"].s(), data["text"].s())) {
            return crow::response(500, "Failed to save message to database");
        }

        std::string err;
        bool ok = chatFeature->AddMessage(data["sender"].s(), data["text"].s(), err);
        crow::json::wvalue res;
        res["success"] = ok;
        res["error"] = err;
        return crow::response(res);
    });

    // CHAT FETCH
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
