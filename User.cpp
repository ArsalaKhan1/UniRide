#include "User.h"

User::User() : role(UserRole::PASSENGER), hasActiveRequest(false) {}

User::User(const std::string &id, const std::string &n, const std::string &e, const std::string &g, const std::string &ei, bool v, UserRole r)
    : userID(id), name(n), email(e), gender(g), enrollment_id(ei), verified(v), role(r), hasActiveRequest(false) {}
