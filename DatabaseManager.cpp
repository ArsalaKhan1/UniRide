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
            gender TEXT
        );
    )";

    // Create rides table
    const char* createRidesTable = R"(
        CREATE TABLE IF NOT EXISTS rides (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userID TEXT NOT NULL,
            from_location TEXT NOT NULL,
            to_location TEXT NOT NULL,
            time TEXT NOT NULL,
            mode TEXT NOT NULL,
            ride_type INTEGER NOT NULL DEFAULT 1,
            current_capacity INTEGER NOT NULL DEFAULT 1,
            max_capacity INTEGER NOT NULL DEFAULT 5,
            females_only INTEGER DEFAULT 0,
            FOREIGN KEY(userID) REFERENCES users(userID)
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

    // Create OTP sessions table
    const char* createOTPTable = R"(
        CREATE TABLE IF NOT EXISTS otp_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userA_ID TEXT NOT NULL,
            userB_ID TEXT NOT NULL,
            otp_code TEXT NOT NULL,
            status TEXT DEFAULT 'pending',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(userA_ID) REFERENCES users(userID),
            FOREIGN KEY(userB_ID) REFERENCES users(userID)
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

    char* errMsg = 0;
    const char* tables[] = {createUsersTable, createRidesTable, createRequestsTable, createOTPTable, createMessagesTable};
    
    for (int i = 0; i < 5; i++) {
        rc = sqlite3_exec(db, tables[i], 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
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

bool DatabaseManager::insertRide(const Ride& ride) {
    const char* sql = "INSERT INTO rides (userID, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, ride.userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ride.from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ride.to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, ride.time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ride.mode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, static_cast<int>(ride.rideType));
    sqlite3_bind_int(stmt, 7, ride.currentCapacity);
    sqlite3_bind_int(stmt, 8, ride.maxCapacity);
    sqlite3_bind_int(stmt, 9, ride.femalesOnly ? 1 : 0);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<Ride> DatabaseManager::getAllRides() {
    std::vector<Ride> rides;
    const char* sql = "SELECT userID, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only FROM rides;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rides;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Ride ride;
        ride.userID = (char*)sqlite3_column_text(stmt, 0);
        ride.from = (char*)sqlite3_column_text(stmt, 1);
        ride.to = (char*)sqlite3_column_text(stmt, 2);
        ride.time = (char*)sqlite3_column_text(stmt, 3);
        ride.mode = (char*)sqlite3_column_text(stmt, 4);
        ride.rideType = static_cast<RideType>(sqlite3_column_int(stmt, 5));
        ride.currentCapacity = sqlite3_column_int(stmt, 6);
        ride.maxCapacity = sqlite3_column_int(stmt, 7);
        ride.femalesOnly = sqlite3_column_int(stmt, 8) == 1;
        if (!ride.userID.empty()) {
            ride.participants.push_back(ride.userID);
        }
        rides.push_back(ride);
    }

    sqlite3_finalize(stmt);
    return rides;
}

std::vector<Ride> DatabaseManager::findRideMatches(const std::string& from, const std::string& to, RideType rideType, const std::string& userID) {
    std::vector<Ride> matches;
    const char* sql = "SELECT userID, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only FROM rides WHERE from_location = ? AND to_location = ? AND ride_type = ? AND current_capacity < max_capacity;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return matches;

    sqlite3_bind_text(stmt, 1, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(rideType));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Ride ride;
        ride.userID = (char*)sqlite3_column_text(stmt, 0);
        ride.from = (char*)sqlite3_column_text(stmt, 1);
        ride.to = (char*)sqlite3_column_text(stmt, 2);
        ride.time = (char*)sqlite3_column_text(stmt, 3);
        ride.mode = (char*)sqlite3_column_text(stmt, 4);
        ride.rideType = static_cast<RideType>(sqlite3_column_int(stmt, 5));
        ride.currentCapacity = sqlite3_column_int(stmt, 6);
        ride.maxCapacity = sqlite3_column_int(stmt, 7);
        ride.femalesOnly = sqlite3_column_int(stmt, 8) == 1;
        
        // Filter out females-only rides for non-females
        if (ride.femalesOnly && !userID.empty()) {
            User user = getUserByID(userID);
            if (user.gender != "female") {
                continue; // Skip this ride
            }
        }
        
        if (!ride.userID.empty()) {
            ride.participants.push_back(ride.userID);
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

bool DatabaseManager::insertOTPSession(const std::string& userA, const std::string& userB, const std::string& otp) {
    const char* sql = "INSERT INTO otp_sessions (userA_ID, userB_ID, otp_code) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userA.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userB.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, otp.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateOTPStatus(const std::string& userA, const std::string& userB, const std::string& status) {
    const char* sql = "UPDATE otp_sessions SET status = ? WHERE userA_ID = ? AND userB_ID = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userA.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, userB.c_str(), -1, SQLITE_STATIC);

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
    const char* sql = "UPDATE rides SET current_capacity = ? WHERE userID = ? AND from_location = ? AND to_location = ?;";
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