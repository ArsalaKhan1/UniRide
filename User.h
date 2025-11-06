#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole {
    RIDE_OWNER,   // Driver who creates ride offer
    PASSENGER,    // Joins existing ride offer
    PARTICIPANT   // Joins shared booking group
};

enum class VehiclePreference {
    BIKE,
    RICKSHAW,
    CAR_OWNER,      // User owns car and offers carpool
    CAR_BOOKING     // User wants to book car via external app
};

struct UserPreferences {
    std::string genderPreference;  // "male", "female", "any"
    VehiclePreference vehicleType;
    
    UserPreferences() : genderPreference("any"), vehicleType(VehiclePreference::CAR_BOOKING) {}
};

struct User {
    std::string userID;
    std::string name;
    std::string email;
    std::string gender;
    std::string enrollment_id;
    bool verified;
    UserRole role;
    UserPreferences preferences;
    bool hasActiveRequest;

    User();
    User(const std::string &id, const std::string &n, const std::string &e, const std::string &g = "", const std::string &ei = "", bool v = false, UserRole r = UserRole::PASSENGER);
};

#endif // USER_H
