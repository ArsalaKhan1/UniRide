#include "crow.h"
#include "AuthSys.h"
#include "RideSystem.h"
#include "RequestQueue.h"
#include "OTPVerification.h"
#include "ChatFeature.h"
#include <memory>

int main() {
    crow::SimpleApp app;

    AuthSystem authSystem;
    RideSystem rideSystem;
    RequestQueue requestQueue(&rideSystem);

    // OTP + Chat shared state
    auto otpSystem = std::make_shared<OTPVerification>("", "");
    ChatFeature chatFeature(otpSystem);

    CROW_ROUTE(app, "/")
    ([]() {
        return "🚗 UniRide API is running successfully!";
    });
    // ✅ USER REGISTRATION
    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        auto user = authSystem.registerUser(
            data["id"].s(),
            data["name"].s(),
            data["email"].s()
        );
        return crow::response(authSystem.toJson());
    });

    // ✅ LOGIN
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        bool ok = authSystem.loginUser(data["email"].s());
        crow::json::wvalue res;
        res["status"] = ok ? "Login successful" : "User not found";
        return crow::response(res);
    });

    // ✅ ADD RIDE
    CROW_ROUTE(app, "/ride/add").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        rideSystem.addRide(
            data["userID"].s(),
            data["from"].s(),
            data["to"].s(),
            data["time"].s(),
            data["mode"].s()
        );

        return crow::response(rideSystem.getAllRidesJson());
    });

    // ✅ GET ALL RIDES
    CROW_ROUTE(app, "/ride/all").methods("GET"_method)
    ([&]() {
        return crow::response(rideSystem.getAllRidesJson());
    });

    // ✅ CREATE REQUEST
    CROW_ROUTE(app, "/request/create").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        auto ids = requestQueue.createRequest(
            data["userID"].s(),
            data["from"].s(),
            data["to"].s()
        );
        crow::json::wvalue res;
        res["createdIDs"] = ids;
        return crow::response(res);
    });

    // ✅ VIEW REQUESTS
    CROW_ROUTE(app, "/request/pending").methods("GET"_method)
    ([&]() {
        return crow::response(requestQueue.listPending());
    });

    // ✅ RESPOND TO REQUEST
    CROW_ROUTE(app, "/request/respond").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");
        std::string msg;
        bool success = requestQueue.respondToRequest(data["requestID"].i(), data["accept"].b(), msg);
        crow::json::wvalue res;
        res["message"] = msg;
        res["success"] = success;
        return crow::response(res);
    });

    // ✅ OTP INITIATE
    CROW_ROUTE(app, "/otp/initiate").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        otpSystem = std::make_shared<OTPVerification>(
            data["userA_ID"].s(),
            data["userB_ID"].s()
        );
        std::string otp = otpSystem->initiateVerification();

        crow::json::wvalue res;
        res["otp"] = otp;
        return crow::response(res);
    });

    // ✅ OTP VERIFY
    CROW_ROUTE(app, "/otp/verify").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        bool ok = otpSystem->verifyOTPInput(data["userID"].s(), data["otp"].s());
        crow::json::wvalue res;
        res["status"] = ok ? "Verified" : "Invalid OTP";
        return crow::response(res);
    });

    // ✅ OTP STATUS
    CROW_ROUTE(app, "/otp/status").methods("GET"_method)
    ([&]() {
        return crow::response(otpSystem->statusJson());
    });

    // ✅ CHAT SEND
    CROW_ROUTE(app, "/chat/send").methods("POST"_method)
    ([&](const crow::request &req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string err;
        bool ok = chatFeature.AddMessage(data["sender"].s(), data["text"].s(), err);
        crow::json::wvalue res;
        res["success"] = ok;
        res["error"] = err;
        return crow::response(res);
    });

    // ✅ CHAT FETCH
    CROW_ROUTE(app, "/chat/all").methods("GET"_method)
    ([&]() {
        return crow::response(chatFeature.getMessagesJson());
    });

    app.port(8080).multithreaded().run();
}
