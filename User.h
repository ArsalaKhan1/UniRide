#ifndef USER_H
#define USER_H

#include <string>

struct User {
    std::string userID;
    std::string name;
    std::string email;

    User();
    User(const std::string &id, const std::string &n, const std::string &e);
};

#endif // USER_H
