#include "User.h"

User::User() = default;

User::User(const std::string &id, const std::string &n, const std::string &e)
    : userID(id), name(n), email(e) {}
