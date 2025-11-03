#ifndef AUTHSYS_H
#define AUTHSYS_H

#pragma once
#include <string>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "User.h"
#include "DatabaseManager.h"
#include "crow.h"

class AuthSystem {
private:
    mutable std::mutex mtx;
    std::shared_ptr<DatabaseManager> dbManager;
    std::unordered_map<std::string, std::string> sessionTokens; // token -> userID
    
    std::string generateJWT(const User& user);
    bool validateJWT(const std::string& token, std::string& userID);
    std::string generateSessionToken();
    crow::json::rvalue verifyGoogleToken(const std::string& idToken);

public:
    AuthSystem(std::shared_ptr<DatabaseManager> db);
    

    bool isRegistered(const std::string &email) const;
    User handleGoogleAuth(const std::string& idToken, const std::string& enrollmentId);
    std::string storeSessionToken(const std::string& userID);
    bool validateSessionToken(const std::string& token, std::string& userID);
    crow::json::wvalue toJson() const;
};
#endif // AUTHSYS_H