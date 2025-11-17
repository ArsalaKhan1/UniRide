#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& path) : db(nullptr), dbPath(path), locationGraph(nullptr) {}

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
            females_only INTEGER DEFAULT 0,  -- ADD THIS LINE
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
            email_pattern TEXT,
            gender TEXT
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

    // Ensure students table has gender column (no-op if already present)
    sqlite3_exec(db, "ALTER TABLE students ADD COLUMN gender TEXT;", 0, 0, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
        errMsg = 0;
    }

    // Populate mock students table (now includes gender)
    const char* inserts[] = {
        "INSERT OR IGNORE INTO students VALUES('NED/0393/2024', 'khan4735002@cloud.neduet.edu.pk', 'male');",
        "INSERT OR IGNORE INTO students VALUES('NED/0887/2024', 'soomro4720844@cloud.neduet.edu.pk', 'female');",
        "INSERT OR IGNORE INTO students VALUES('NED/1915/2024', 'rafique4735048@cloud.neduet.edu.pk', 'female');",
        "INSERT OR IGNORE INTO students VALUES('NED/0636/2024', 'zaman4705230@cloud.neduet.edu.pk', 'male');",
        "INSERT OR IGNORE INTO students VALUES('NED/0556/2024', 'rashid4705806@cloud.neduet.edu.pk', 'male');",
        "INSERT OR IGNORE INTO students VALUES('NED/0770/2024', 'abrar4705198@cloud.neduet.edu.pk', 'female');"
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
    // Try to fetch gender from students table using enrollment_id if provided
    std::string genderToInsert = user.gender;
    if (!user.enrollment_id.empty()) {
        const char* q = "SELECT gender FROM students WHERE enrollment_id = ?;";
        sqlite3_stmt* qstmt = nullptr;
        int qrc = sqlite3_prepare_v2(db, q, -1, &qstmt, NULL);
        if (qrc == SQLITE_OK) {
            sqlite3_bind_text(qstmt, 1, user.enrollment_id.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(qstmt) == SQLITE_ROW) {
                const unsigned char* gptr = sqlite3_column_text(qstmt, 0);
                if (gptr) genderToInsert = reinterpret_cast<const char*>(gptr);
            }
        }
        if (qstmt) sqlite3_finalize(qstmt);
    }

    const char* sql = "INSERT OR REPLACE INTO users (userID, name, email, gender) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user.userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, genderToInsert.c_str(), -1, SQLITE_STATIC);

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

// Helper function to convert string status to RideStatus enum
static RideStatus stringToRideStatus(const std::string& statusStr) {
    if (statusStr == "open") return RideStatus::OPEN;
    if (statusStr == "full") return RideStatus::FULL;
    if (statusStr == "started") return RideStatus::STARTED;
    if (statusStr == "completed") return RideStatus::COMPLETED;
    return RideStatus::OPEN; // default
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
        const char* statusStr = (char*)sqlite3_column_text(stmt, 10);
        ride.status = stringToRideStatus(statusStr ? statusStr : "open");
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
    const char* sql = "SELECT id, owner_id, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only FROM rides WHERE ride_type = ? AND current_capacity < max_capacity AND ride_status = 'open' AND owner_id != ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return matches;

    sqlite3_bind_int(stmt, 1, static_cast<int>(rideType));
    sqlite3_bind_text(stmt, 2, userID.c_str(), -1, SQLITE_STATIC);

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
        
        // Check proximity using LocationGraph
        if (locationGraph && !locationGraph->areConnected(from, ride.from)) continue;
        if (locationGraph && !locationGraph->areConnected(to, ride.to)) continue;
        
        // Enforce females-only visibility rules:
        // If the ride is marked females-only, only show it to users whose recorded gender is 'female'.
        // Male users and unknown users should not see females-only rides.
        if (ride.femalesOnly) {
            if (!userID.empty()) {
                User user = getUserByID(userID);
                // Only allow if the user's recorded gender is female
                if (user.gender != "female") {
                    continue;
                }
            } else {
                // Unknown user — do not expose females-only rides
                continue;
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

bool DatabaseManager::insertRequest(const std::string& userID, const std::string& from, const std::string& to, RideType rideType, bool femalesOnly) {
    const char* sql = "INSERT INTO requests (userID, from_location, to_location, ride_type, females_only) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, static_cast<int>(rideType));
    sqlite3_bind_int(stmt, 5, femalesOnly ? 1 : 0);

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

std::vector<Ride> DatabaseManager::findMatchingRides(
    const std::string& from, 
    const std::string& to, 
    RideType rideType, 
    const std::string& userID, 
    const std::string& genderPref,
    bool searcherWantsFemalesOnly) {
    
    std::vector<Ride> matches;
    const char* sql = R"(
        SELECT id, owner_id, from_location, to_location, time, mode, ride_type, 
               current_capacity, max_capacity, females_only, gender_preference, ride_status
        FROM rides 
        WHERE ride_type = ? 
          AND ride_status = 'open' 
          AND current_capacity < max_capacity
          AND owner_id != ?
          AND (gender_preference = 'any' OR gender_preference = ?)
    )";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return matches;

    sqlite3_bind_int(stmt, 1, static_cast<int>(rideType));
    sqlite3_bind_text(stmt, 2, userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, genderPref.c_str(), -1, SQLITE_STATIC);

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
        ride.genderPreference = (char*)sqlite3_column_text(stmt, 10);
        const char* statusStr = (char*)sqlite3_column_text(stmt, 11);
        ride.status = stringToRideStatus(statusStr ? statusStr : "open");
        
        // Check proximity using LocationGraph
        if (locationGraph && !locationGraph->areConnected(from, ride.from)) continue;
        if (locationGraph && !locationGraph->areConnected(to, ride.to)) continue;
        
        // Get user info for gender checks
        User user = getUserByID(userID);
        std::string userGender = user.gender;
        
        // DEBUG: Log ride and user info
        std::cout << "DEBUG: Ride ID " << ride.rideID << ", femalesOnly: " << ride.femalesOnly 
                  << ", userGender: '" << userGender << "', searcherWantsFemalesOnly: " << searcherWantsFemalesOnly << std::endl;
        
        // ===== CORRECTED FILTERING LOGIC =====
        
        // RULE 1: Males can NEVER see females-only rides
        if (ride.femalesOnly && userGender == "male") {
            std::cout << "DEBUG: Skipping ride " << ride.rideID << " - RULE 1: Male user cannot see females-only ride" << std::endl;
            continue; // Skip this ride
        }
        
        // RULE 2: If female user wants ONLY females-only rides
        if (userGender == "female" && searcherWantsFemalesOnly) {
            // Only show females-only rides
            if (!ride.femalesOnly) {
                std::cout << "DEBUG: Skipping ride " << ride.rideID << " - RULE 2: Female wants females-only but ride is not females-only" << std::endl;
                continue;
            }
        }
        
        // RULE 3: If female user does NOT want females-only specifically, show ALL rides
        // RULE 4: Non-female users (except males) see only regular rides
        if (ride.femalesOnly && userGender != "female") {
            std::cout << "DEBUG: Skipping ride " << ride.rideID << " - RULE 4: Non-female user cannot see females-only ride" << std::endl;
            continue;
        }
        
        std::cout << "DEBUG: Including ride " << ride.rideID << " in matches" << std::endl;
        
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

bool DatabaseManager::doesEnrollmentMatchEmail(const std::string& enrollmentID, const std::string& email) {
    const char* sql = "SELECT email_pattern FROM students WHERE enrollment_id = ?;";
    sqlite3_stmt* stmt;
    std::string pattern;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, enrollmentID.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        if (txt) pattern = reinterpret_cast<const char*>(txt);
    }

    sqlite3_finalize(stmt);

    if (pattern.empty()) return false;

    // Simple exact match for now. If pattern contains wildcards in future, change to LIKE.
    return pattern == email;
}

std::vector<std::pair<int, std::string>> DatabaseManager::getAcceptedRequestsForUser(const std::string& userID) {
    std::vector<std::pair<int, std::string>> accepted;
    const char* sql = R"(
        SELECT jr.ride_id, 
               COALESCE(r.owner_id, 
                   (SELECT jr2.user_id FROM join_requests jr2 
                    WHERE jr2.ride_id = r.id AND jr2.status IN ('accepted', 'pending')
                    ORDER BY jr2.created_at ASC LIMIT 1)) AS lead_user_id
        FROM join_requests jr
        JOIN rides r ON jr.ride_id = r.id
        WHERE jr.user_id = ? AND jr.status = 'accepted' AND r.ride_status IN ('open', 'full', 'started')
        ORDER BY jr.ride_id ASC
    )";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return accepted;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int rideID = sqlite3_column_int(stmt, 0);
        const unsigned char* leadUserIDPtr = sqlite3_column_text(stmt, 1);
        std::string leadUserID = leadUserIDPtr ? (char*)leadUserIDPtr : "";
        if (!leadUserID.empty()) {
            accepted.push_back({rideID, leadUserID});
        }
    }

    sqlite3_finalize(stmt);
    return accepted;
}

std::vector<std::pair<std::string, std::string>> DatabaseManager::getAcceptedPassengers(int rideID) {
    std::vector<std::pair<std::string, std::string>> passengers;
    const char* sql = R"(
        SELECT jr.user_id, u.name
        FROM join_requests jr
        JOIN users u ON jr.user_id = u.userID
        WHERE jr.ride_id = ? AND jr.status = 'accepted'
        ORDER BY jr.created_at ASC
    )";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return passengers;

    sqlite3_bind_int(stmt, 1, rideID);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string userID = (char*)sqlite3_column_text(stmt, 0);
        const unsigned char* namePtr = sqlite3_column_text(stmt, 1);
        std::string userName = namePtr ? (char*)namePtr : "";
        passengers.push_back({userID, userName});
    }

    sqlite3_finalize(stmt);
    return passengers;
}

std::vector<Ride> DatabaseManager::getActiveRidesForUser(const std::string& userID) {
    std::vector<Ride> activeRides;
    const char* sql = R"(
        SELECT id, owner_id, from_location, to_location, time, mode, ride_type, 
               current_capacity, max_capacity, females_only, ride_status, gender_preference
        FROM rides 
        WHERE (owner_id = ? OR id IN (
            SELECT ride_id FROM join_requests 
            WHERE user_id = ? AND status = 'accepted'
        ))
        AND ride_status IN ('open', 'started')
    )";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return activeRides;

    sqlite3_bind_text(stmt, 1, userID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userID.c_str(), -1, SQLITE_STATIC);

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
        const char* statusStr = (char*)sqlite3_column_text(stmt, 10);
        ride.status = stringToRideStatus(statusStr ? statusStr : "open");
        ride.genderPreference = (char*)sqlite3_column_text(stmt, 11);
        if (!ride.ownerID.empty()) {
            ride.participants.push_back(ride.ownerID);
        }
        activeRides.push_back(ride);
    }

    sqlite3_finalize(stmt);
    return activeRides;
}

Ride DatabaseManager::getRideByID(int rideID) {
    Ride ride;
    const char* sql = "SELECT id, owner_id, from_location, to_location, time, mode, ride_type, current_capacity, max_capacity, females_only, ride_status, gender_preference FROM rides WHERE id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return ride;

    sqlite3_bind_int(stmt, 1, rideID);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
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
        const char* statusStr = (char*)sqlite3_column_text(stmt, 10);
        ride.status = stringToRideStatus(statusStr ? statusStr : "open");
        ride.genderPreference = (char*)sqlite3_column_text(stmt, 11);
        if (!ride.ownerID.empty()) {
            ride.participants.push_back(ride.ownerID);
        }
    }

    sqlite3_finalize(stmt);
    return ride;
}