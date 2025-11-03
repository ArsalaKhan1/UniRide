#include "User.h"

User::User() : role(UserRole::PASSENGER), hasActiveRequest(false) {}

User::User(const std::string &id, const std::string &n, const std::string &e, const std::string &g, UserRole r)
    : userID(id), name(n), email(e), gender(g), role(r), hasActiveRequest(false) {}
