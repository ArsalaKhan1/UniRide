#include "AuthSys.h"

User AuthSystem::registerUser(const std::string &id, const std::string &name, const std::string &email) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = users.find(email);
    if (it != users.end()) {
        return it->second;
    }
    users[email] = User(id, name, email);
    return users[email];
}

bool AuthSystem::loginUser(const std::string &email) {
    std::lock_guard<std::mutex> lock(mtx);
    return users.find(email) != users.end();
}

bool AuthSystem::isRegistered(const std::string &email) const {
    std::lock_guard<std::mutex> lock(mtx);
    return users.find(email) != users.end();
}

crow::json::wvalue AuthSystem::toJson() const {
    crow::json::wvalue res;
    int i = 0;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &p : users) {
        res["users"][i]["userID"] = p.second.userID;
        res["users"][i]["name"] = p.second.name;
        res["users"][i]["email"] = p.second.email;
        ++i;
    }
    return res;
}
