#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole {
    RIDE_OWNER,   // Driver who creates ride offer
    PASSENGER,    // Joins existing ride offer
    PARTICIPANT   // Joins shared booking group
};

struct User {
    std::string userID;
    std::string name;
    std::string email;
    std::string gender;
    UserRole role;

    User();
    User(const std::string &id, const std::string &n, const std::string &e, const std::string &g = "", UserRole r = UserRole::PASSENGER);
};

#endif // USER_H
