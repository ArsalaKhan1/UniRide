#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& path) : db(nullptr), dbPath(path) {}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Create users table
    const char* createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            userID TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            gender TEXT,
            gender_preference TEXT DEFAULT 'any',
            vehicle_preference INTEGER DEFAULT 3,
            has_active_request INTEGER DEFAULT 0
        );
    )";

    // Create rides table
    const char* createRidesTable = R"(
        CREATE TABLE IF NOT EXISTS rides (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_id TEXT,
            from_location TEXT NOT NULL,
            to_location TEXT NOT NULL,
            time TEXT NOT NULL,
            mode TEXT NOT NULL,
            ride_type INTEGER NOT NULL DEFAULT 1,
            ride_status TEXT DEFAULT 'open',
            current_capacity INTEGER NOT NULL DEFAULT 1,
            max_capacity INTEGER NOT NULL DEFAULT 5,
            females_only INTEGER DEFAULT 0,
            gender_preference TEXT DEFAULT 'any',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(owner_id) REFERENCES users(userID)
        );
    )";

    // Create join requests table
    const char* createJoinRequestsTable = R"(
        CREATE TABLE IF NOT EXISTS join_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ride_id INTEGER NOT NULL,
            user_id TEXT NOT NULL,
            status TEXT DEFAULT 'pending',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(ride_id) REFERENCES rides(id),
            FOREIGN KEY(user_id) REFERENCES users(userID),
            UNIQUE(ride_id, user_id)
        );
    )";

    // Create requests table
    const char* createRequestsTable = R"(
        CREATE TABLE IF NOT EXISTS requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userID TEXT NOT NULL,
            from_location TEXT NOT NULL,
            to_location TEXT NOT NULL,
            ride_type INTEGER NOT NULL,
            status TEXT DEFAULT 'pending',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(userID) REFERENCES users(userID)
        );
    )";

    // Create chat messages table
    const char* createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id TEXT NOT NULL,
            message_text TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(sender_id) REFERENCES users(userID)
        );
    )";

    // Create students table
    const char* createStudentsTable = R"(
        CREATE TABLE IF NOT EXISTS students (
            enrollment_id TEXT PRIMARY KEY,
            email_pattern TEXT
        );
    )";

    char* errMsg = 0;
    const char* tables[] = {createUsersTable, createRidesTable, createJoinRequestsTable, createRequestsTable, createMessagesTable, createStudentsTable};
    
    for (int i = 0; i < 6; i++) {
        rc = sqlite3_exec(db, tables[i], 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
    }

    // Add new columns to existing users table if they don't exist
    const char* addColumns[] = {
        "ALTER TABLE users ADD COLUMN gender_preference TEXT DEFAULT 'any';",
        "ALTER TABLE users ADD COLUMN vehicle_preference INTEGER DEFAULT 3;",
        "ALTER TABLE users ADD COLUMN has_active_request INTEGER DEFAULT 0;"
    };
    
    for (int i = 0; i < 3; i++) {
        sqlite3_exec(db, addColumns[i], 0, 0, &errMsg);
        // Ignore errors for columns that already exist
        if (errMsg) {
            sqlite3_free(errMsg);
            errMsg = 0;
        }
    }

    // Add new columns to existing rides table if they don't exist
    const char* addRideColumns[] = {
        "ALTER TABLE rides ADD COLUMN owner_id TEXT;",
        "ALTER TABLE rides ADD COLUMN ride_status TEXT DEFAULT 'open';",
        "ALTER TABLE rides ADD COLUMN gender_preference TEXT DEFAULT 'any';"
    };
    
    for (int i = 0; i < 3; i++) {
        sqlite3_exec(db, addRideColumns[i], 0, 0, &errMsg);
        // Ignore errors for columns that already exist
        if (errMsg) {
            sqlite3_free(errMsg);
            errMsg = 0;
        }
    }

    // Populate mock students table
    const char* inserts[] = {
        "INSERT OR IGNORE INTO students VALUES('NED/0393/2024', 'khan4735002@cloud.neduet.edu.pk');",
        "INSERT OR IGNORE INTO students VALUES('NED/0887/2024', 'soomro4720844@cloud.neduet.edu.pk');",
        "INSERT OR IGNORE INTO students VALUES('NED/1915/2024', 'rafique4735048@cloud.neduet.edu.pk');"
    };

    for (auto sql : inserts) {
        if (sqlite3_exec(db, sql, 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Error inserting student: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
    }

    return true;
}

bool DatabaseManager::insertUser(const User& user) {
    const char* sql = "INSERT OR REPLACE INTO users (userID, name, email, gender) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user.userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user.gender.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

User DatabaseManager::getUserByEmail(const std::string& email) {
    const char* sql = "SELECT userID, name, email, gender FROM users WHERE email = ?;";
    sqlite3_stmt* stmt;
    User user;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return user;

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.userID = (char*)sqlite3_column_text(stmt, 0);
        user.name = (char*)sqlite3_column_text(stmt, 1);
        user.email = (char*)sqlite3_column_text(stmt, 2);
        user.gender = (char*)sqlite3_column_text(stmt, 3);
    }

    sqlite3_finalize(stmt);
    return user;
}

User DatabaseManager::getUserByID(const std::string& userID) {
    const char* sql = "SELECT userID, name, email, gender FROM users WHERE userID = ?;";
    sqlite3_stmt* stmt;
    User user;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return user;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.userID = (char*)sqlite3_column_text(stmt, 0);
        user.name = (char*)sqlite3_column_text(stmt, 1);
        user.email = (char*)sqlite3_column_text(stmt, 2);
        user.gender = (char*)sqlite3_column_text(stmt, 3);
    }

    sqlite3_finalize(stmt);
    return user;
}

std::vector<User> DatabaseManager::getAllUsers() {
    std::vector<User> users;
    const char* sql = "SELECT userID, name, email FROM users;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return users;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.userID = (char*)sqlite3_column_text(stmt, 0);
        user.name = (char*)sqlite3_column_text(stmt, 1);
        user.email = (char*)sqlite3_column_text(stmt, 2);
        users.push_back(user);
    }

    sqlite3_finalize(stmt);
    return users;
}

int DatabaseManager::insertRide(Ride& ride) {
    const char* sql = "INSERT INTO rides (owner_id, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only, ride_status, gender_preference) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, ride.ownerID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ride.from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ride.to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, ride.time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ride.mode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, static_cast<int>(ride.rideType));
    sqlite3_bind_int(stmt, 7, ride.currentCapacity);
    sqlite3_bind_int(stmt, 8, ride.maxCapacity);
    sqlite3_bind_int(stmt, 9, ride.femalesOnly ? 1 : 0);
    sqlite3_bind_text(stmt, 10, "open", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, ride.genderPreference.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        int rideID = sqlite3_last_insert_rowid(db);
        ride.rideID = rideID;
        return rideID;
    }
    
    return -1;
}

std::vector<Ride> DatabaseManager::getAllRides() {
    std::vector<Ride> rides;
    const char* sql = "SELECT id, owner_id, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only, ride_status, gender_preference FROM rides;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rides;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Ride ride;
        ride.rideID = sqlite3_column_int(stmt, 0);
        ride.ownerID = (char*)sqlite3_column_text(stmt, 1);
        ride.userID = ride.ownerID;
        ride.from = (char*)sqlite3_column_text(stmt, 2);
        ride.to = (char*)sqlite3_column_text(stmt, 3);
        ride.time = (char*)sqlite3_column_text(stmt, 4);
        ride.mode = (char*)sqlite3_column_text(stmt, 5);
        ride.rideType = static_cast<RideType>(sqlite3_column_int(stmt, 6));
        ride.currentCapacity = sqlite3_column_int(stmt, 7);
        ride.maxCapacity = sqlite3_column_int(stmt, 8);
        ride.femalesOnly = sqlite3_column_int(stmt, 9) == 1;
        ride.genderPreference = (char*)sqlite3_column_text(stmt, 11);
        if (!ride.ownerID.empty()) {
            ride.participants.push_back(ride.ownerID);
        }
        rides.push_back(ride);
    }

    sqlite3_finalize(stmt);
    return rides;
}

std::vector<Ride> DatabaseManager::findRideMatches(const std::string& from, const std::string& to, RideType rideType, const std::string& userID) {
    std::vector<Ride> matches;
    const char* sql = "SELECT id, owner_id, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only FROM rides WHERE from_location = ? AND to_location = ? AND ride_type = ? AND current_capacity < max_capacity AND ride_status = 'open' AND owner_id != ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return matches;

    sqlite3_bind_text(stmt, 1, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(rideType));
    sqlite3_bind_text(stmt, 4, userID.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Ride ride;
        ride.rideID = sqlite3_column_int(stmt, 0);
        ride.ownerID = (char*)sqlite3_column_text(stmt, 1);
        ride.userID = ride.ownerID; // For compatibility
        ride.from = (char*)sqlite3_column_text(stmt, 2);
        ride.to = (char*)sqlite3_column_text(stmt, 3);
        ride.time = (char*)sqlite3_column_text(stmt, 4);
        ride.mode = (char*)sqlite3_column_text(stmt, 5);
        ride.rideType = static_cast<RideType>(sqlite3_column_int(stmt, 6));
        ride.currentCapacity = sqlite3_column_int(stmt, 7);
        ride.maxCapacity = sqlite3_column_int(stmt, 8);
        ride.femalesOnly = sqlite3_column_int(stmt, 9) == 1;
        
        // Filter out females-only rides for non-females
        if (ride.femalesOnly && !userID.empty()) {
            User user = getUserByID(userID);
            if (user.gender != "female") {
                continue; // Skip this ride
            }
        }
        
        if (!ride.ownerID.empty()) {
            ride.participants.push_back(ride.ownerID);
        }
        matches.push_back(ride);
    }

    sqlite3_finalize(stmt);
    return matches;
}

bool DatabaseManager::insertRequest(const std::string& userID, const std::string& from, const std::string& to, RideType rideType) {
    const char* sql = "INSERT INTO requests (userID, from_location, to_location, ride_type) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, static_cast<int>(rideType));

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateRequestStatus(int requestID, const std::string& status) {
    const char* sql = "UPDATE requests SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, requestID);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}



bool DatabaseManager::insertMessage(const std::string& senderID, const std::string& messageText) {
    const char* sql = "INSERT INTO messages (sender_id, message_text) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, senderID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, messageText.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateRideCapacity(const std::string& userID, const std::string& from, const std::string& to, int newCapacity) {
    const char* sql = "UPDATE rides SET current_capacity = ? WHERE owner_id = ? AND from_location = ? AND to_location = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, newCapacity);
    sqlite3_bind_text(stmt, 2, userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, to.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getAllMessages() {
    std::vector<std::pair<std::string, std::string>> messages;
    const char* sql = "SELECT sender_id, message_text FROM messages ORDER BY timestamp;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return messages;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string sender = (char*)sqlite3_column_text(stmt, 0);
        std::string message = (char*)sqlite3_column_text(stmt, 1);
        messages.push_back({sender, message});
    }

    sqlite3_finalize(stmt);
    return messages;
}

bool DatabaseManager::updateUserPreferences(const std::string& userID, const std::string& genderPref, int vehicleType) {
    const char* sql = "UPDATE users SET gender_preference = ?, vehicle_preference = ? WHERE userID = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, genderPref.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, vehicleType);
    sqlite3_bind_text(stmt, 3, userID.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::getUserPreferences(const std::string& userID, std::string& genderPref, int& vehicleType) {
    const char* sql = "SELECT gender_preference, vehicle_preference FROM users WHERE userID = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        genderPref = (char*)sqlite3_column_text(stmt, 0);
        vehicleType = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

std::vector<Ride> DatabaseManager::findMatchingRides(const std::string& from, const std::string& to, 
                                                    RideType rideType, const std::string& userID, 
                                                    const std::string& genderPref) {
    std::vector<Ride> matches;
    const char* sql = R"(
        SELECT id, owner_id, from_location, to_location, time, mode, ride_type, 
               current_capacity, max_capacity, females_only, gender_preference, ride_status
        FROM rides 
        WHERE from_location = ? AND to_location = ? AND ride_type = ? 
              AND ride_status = 'open' AND current_capacity < max_capacity
              AND (gender_preference = 'any' OR gender_preference = ?)
    )";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return matches;

    sqlite3_bind_text(stmt, 1, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(rideType));
    sqlite3_bind_text(stmt, 4, genderPref.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Ride ride;
        ride.rideID = sqlite3_column_int(stmt, 0);
        ride.ownerID = (char*)sqlite3_column_text(stmt, 1);
        ride.userID = ride.ownerID; // For compatibility
        ride.from = (char*)sqlite3_column_text(stmt, 2);
        ride.to = (char*)sqlite3_column_text(stmt, 3);
        ride.time = (char*)sqlite3_column_text(stmt, 4);
        ride.mode = (char*)sqlite3_column_text(stmt, 5);
        ride.rideType = static_cast<RideType>(sqlite3_column_int(stmt, 6));
        ride.currentCapacity = sqlite3_column_int(stmt, 7);
        ride.maxCapacity = sqlite3_column_int(stmt, 8);
        ride.femalesOnly = sqlite3_column_int(stmt, 9) == 1;
        ride.genderPreference = (char*)sqlite3_column_text(stmt, 10);
        
        matches.push_back(ride);
    }

    sqlite3_finalize(stmt);
    return matches;
}

bool DatabaseManager::insertJoinRequest(int rideID, const std::string& userID) {
    const char* sql = "INSERT OR IGNORE INTO join_requests (ride_id, user_id) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, rideID);
    sqlite3_bind_text(stmt, 2, userID.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateJoinRequestStatus(int rideID, const std::string& userID, const std::string& status) {
    const char* sql = "UPDATE join_requests SET status = ? WHERE ride_id = ? AND user_id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, rideID);
    sqlite3_bind_text(stmt, 3, userID.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateRideStatus(int rideID, const std::string& status) {
    const char* sql = "UPDATE rides SET ride_status = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, rideID);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getPendingRequests(int rideID) {
    std::vector<std::pair<std::string, std::string>> requests;
    const char* sql = "SELECT user_id, created_at FROM join_requests WHERE ride_id = ? AND status = 'pending';";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return requests;

    sqlite3_bind_int(stmt, 1, rideID);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string userID = (char*)sqlite3_column_text(stmt, 0);
        std::string timestamp = (char*)sqlite3_column_text(stmt, 1);
        requests.push_back({userID, timestamp});
    }

    sqlite3_finalize(stmt);
    return requests;
}

bool DatabaseManager::hasActiveRequest(const std::string& userID) {
    const char* sql = "SELECT COUNT(*) FROM join_requests WHERE user_id = ? AND status = 'pending';";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);

    bool hasActive = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hasActive = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return hasActive;
}

bool DatabaseManager::updateRideCapacityByID(int rideID, int newCapacity) {
    const char* sql = "UPDATE rides SET current_capacity = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, newCapacity);
    sqlite3_bind_int(stmt, 2, rideID);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::isValidEnrollment(const std::string& enrollmentID) {
    const char* sql = "SELECT COUNT(*) FROM students WHERE enrollment_id = ?;";
    sqlite3_stmt* stmt;
    bool exists = false;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, enrollmentID.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}