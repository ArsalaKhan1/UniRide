#ifndef AUTHSYS_H
#define AUTHSYS_H

#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include "User.h"
#include "crow.h"

class AuthSystem {
private:
    mutable std::mutex mtx;
    std::unordered_map<std::string, User> users; // key: email

public:
    User registerUser(const std::string &id, const std::string &name, const std::string &email);
    bool loginUser(const std::string &email);
    bool isRegistered(const std::string &email) const;
    crow::json::wvalue toJson() const;
};
#endif // AUTHSYS_H